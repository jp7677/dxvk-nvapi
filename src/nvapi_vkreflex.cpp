#include "nvapi_private.h"
#include "util/util_statuscode.h"

#include <libloaderapi.h>

using namespace dxvk;

struct VkReflexContextData {
#define PFN_MEMBER(proc) PFN_##proc p##proc
    PFN_MEMBER(vkSetLatencySleepModeNV);
    PFN_MEMBER(vkLatencySleepNV);
    PFN_MEMBER(vkGetLatencyTimingsNV);
    PFN_MEMBER(vkSetLatencyMarkerNV);
    PFN_MEMBER(vkQueueNotifyOutOfBandNV);
#undef PFN_MEMBER
    VkSemaphore semaphore;
    bool lowLatencyMode;
};

static std::unordered_map<VkDevice, VkReflexContextData> contexts = {};
static std::once_flag initialized = {};
static std::mutex mutex = {};
static PFN_vkGetDeviceProcAddr pvkGetDeviceProcAddr = nullptr;

static inline VkReflexContextData* GetContext(VkDevice device) {
    auto it = contexts.find(device);
    return it == contexts.end() ? nullptr : &it->second;
}

static bool LoadVkGetDeviceProcAddrFromModule(const char* name) {
    if (auto vulkan = ::GetModuleHandleA(name)) {
        if (auto vkgdpa = ::GetProcAddress(vulkan, "vkGetDeviceProcAddr"))
            return (pvkGetDeviceProcAddr = reinterpret_cast<PFN_vkGetDeviceProcAddr>(reinterpret_cast<void*>(vkgdpa)));
        else
            log::info(str::format("vkGetDeviceProcAddr not found in ", name));
    } else {
        log::info(str::format(name, " not loaded in current process"));
    }

    return false;
}

static bool LoadVkGetDeviceProcAddr() {
    return pvkGetDeviceProcAddr || LoadVkGetDeviceProcAddrFromModule("vulkan-1.dll") || LoadVkGetDeviceProcAddrFromModule("winevulkan.dll");
}

#define VK_GET_DEVICE_PROC_ADDR(proc) auto p##proc = reinterpret_cast<PFN_##proc>(pvkGetDeviceProcAddr(device, #proc))

static inline VkSwapchainKHR GetSwapchain(VkDevice device) {
    // winevulkan expects valid Vulkan usage so it never checks if swapchains passed to VK_NV_low_latency2 functions
    // are null or not, it just unconditionally dereferences them and this hack makes winevulkan happy enough
    return reinterpret_cast<VkSwapchainKHR>(device);
}

static VkLatencyMarkerNV ToVkLatencyMarkerNV(NV_VULKAN_LATENCY_MARKER_TYPE marker) {
    switch (marker) {
        case VULKAN_SIMULATION_START:
            return VK_LATENCY_MARKER_SIMULATION_START_NV;
        case VULKAN_SIMULATION_END:
            return VK_LATENCY_MARKER_SIMULATION_END_NV;
        case VULKAN_RENDERSUBMIT_START:
            return VK_LATENCY_MARKER_RENDERSUBMIT_START_NV;
        case VULKAN_RENDERSUBMIT_END:
            return VK_LATENCY_MARKER_RENDERSUBMIT_END_NV;
        case VULKAN_PRESENT_START:
            return VK_LATENCY_MARKER_PRESENT_START_NV;
        case VULKAN_PRESENT_END:
            return VK_LATENCY_MARKER_PRESENT_END_NV;
        case VULKAN_INPUT_SAMPLE:
            return VK_LATENCY_MARKER_INPUT_SAMPLE_NV;
        case VULKAN_TRIGGER_FLASH:
            return VK_LATENCY_MARKER_TRIGGER_FLASH_NV;
        case VULKAN_PC_LATENCY_PING:
            break; // unsupported
        case VULKAN_OUT_OF_BAND_RENDERSUBMIT_START:
            return VK_LATENCY_MARKER_OUT_OF_BAND_RENDERSUBMIT_START_NV;
        case VULKAN_OUT_OF_BAND_RENDERSUBMIT_END:
            return VK_LATENCY_MARKER_OUT_OF_BAND_RENDERSUBMIT_END_NV;
        case VULKAN_OUT_OF_BAND_PRESENT_START:
            return VK_LATENCY_MARKER_OUT_OF_BAND_PRESENT_START_NV;
        case VULKAN_OUT_OF_BAND_PRESENT_END:
            return VK_LATENCY_MARKER_OUT_OF_BAND_PRESENT_END_NV;
    }

    return VK_LATENCY_MARKER_MAX_ENUM_NV;
}

extern "C" {
    NvAPI_Status __cdecl NvAPI_Vulkan_InitLowLatencyDevice(HANDLE vkDevice, HANDLE* signalSemaphoreHandle) {
        constexpr auto n = __func__;

        auto device = reinterpret_cast<VkDevice>(vkDevice);
        auto semaphore = reinterpret_cast<VkSemaphore*>(signalSemaphoreHandle);

        if (log::tracing())
            log::trace(n, log::fmt::ptr(vkDevice), log::fmt::ptr(signalSemaphoreHandle));

        std::scoped_lock lock{mutex};

        std::call_once(initialized, LoadVkGetDeviceProcAddr);

        if (!pvkGetDeviceProcAddr)
            return NotSupported(n);

        if (!device)
            return InvalidArgument(n);

        if (!semaphore)
            return InvalidPointer(n);

        auto context = ::GetContext(device);

        if (context) {
            *semaphore = context->semaphore;
            return Ok(n);
        }

        VK_GET_DEVICE_PROC_ADDR(vkCreateSemaphore);
        VK_GET_DEVICE_PROC_ADDR(vkSetLatencySleepModeNV);
        VK_GET_DEVICE_PROC_ADDR(vkLatencySleepNV);
        VK_GET_DEVICE_PROC_ADDR(vkGetLatencyTimingsNV);
        VK_GET_DEVICE_PROC_ADDR(vkSetLatencyMarkerNV);
        VK_GET_DEVICE_PROC_ADDR(vkQueueNotifyOutOfBandNV);

        if (!pvkCreateSemaphore || !pvkSetLatencySleepModeNV || !pvkLatencySleepNV || !pvkGetLatencyTimingsNV || !pvkSetLatencyMarkerNV || !pvkQueueNotifyOutOfBandNV)
            return NoImplementation(n);

        auto semaphoreTypeCreateInfo = VkSemaphoreTypeCreateInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
            .pNext = nullptr,
            .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
            .initialValue = 0,
        };

        auto semaphoreCreateInfo = VkSemaphoreCreateInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = &semaphoreTypeCreateInfo,
            .flags = 0,
        };

        auto vr = pvkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, semaphore);

        if (vr != VK_SUCCESS)
            return Error(n, vr);

        contexts.emplace(
            std::make_pair(
                device,
                VkReflexContextData{
                    .pvkSetLatencySleepModeNV = pvkSetLatencySleepModeNV,
                    .pvkLatencySleepNV = pvkLatencySleepNV,
                    .pvkGetLatencyTimingsNV = pvkGetLatencyTimingsNV,
                    .pvkSetLatencyMarkerNV = pvkSetLatencyMarkerNV,
                    .pvkQueueNotifyOutOfBandNV = pvkQueueNotifyOutOfBandNV,
                    .semaphore = *semaphore,
                    .lowLatencyMode = false,
                }));

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_Vulkan_DestroyLowLatencyDevice(HANDLE vkDevice) {
        constexpr auto n = __func__;

        auto device = reinterpret_cast<VkDevice>(vkDevice);

        if (log::tracing())
            log::trace(n, log::fmt::ptr(vkDevice));

        std::scoped_lock lock{mutex};

        if (!pvkGetDeviceProcAddr)
            return NotSupported(n);

        if (!device)
            return InvalidArgument(n);

        VK_GET_DEVICE_PROC_ADDR(vkDestroySemaphore);

        if (!pvkDestroySemaphore)
            return NoImplementation(n);

        auto node = contexts.extract(device);

        if (node.empty())
            return HandleInvalidated(n);

        auto& context = node.mapped();

        pvkDestroySemaphore(device, context.semaphore, nullptr);

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_Vulkan_GetSleepStatus(HANDLE vkDevice, NV_VULKAN_GET_SLEEP_STATUS_PARAMS* pGetSleepStatusParams) {
        constexpr auto n = __func__;
        thread_local bool alreadyLoggedOk = false;

        auto device = reinterpret_cast<VkDevice>(vkDevice);

        if (log::tracing())
            log::trace(n, log::fmt::ptr(vkDevice), log::fmt::nv_vk_get_sleep_status_params(pGetSleepStatusParams));

        if (!device)
            return InvalidArgument(n);

        if (!pGetSleepStatusParams)
            return InvalidPointer(n);

        if (pGetSleepStatusParams->version != NV_VULKAN_GET_SLEEP_STATUS_PARAMS_VER1)
            return IncompatibleStructVersion(n, pGetSleepStatusParams->version);

        auto context = ::GetContext(device);

        pGetSleepStatusParams->bLowLatencyMode = context && context->lowLatencyMode;

        return Ok(n, alreadyLoggedOk);
    }

    NvAPI_Status __cdecl NvAPI_Vulkan_SetSleepMode(HANDLE vkDevice, NV_VULKAN_SET_SLEEP_MODE_PARAMS* pSetSleepModeParams) {
        constexpr auto n = __func__;
        thread_local bool alreadyLoggedOk = false;

        auto device = reinterpret_cast<VkDevice>(vkDevice);

        if (log::tracing())
            log::trace(n, log::fmt::ptr(vkDevice), log::fmt::nv_vk_set_sleep_status_params(pSetSleepModeParams));

        if (!device)
            return InvalidArgument(n);

        if (pSetSleepModeParams && pSetSleepModeParams->version != NV_VULKAN_SET_SLEEP_MODE_PARAMS_VER1)
            return IncompatibleStructVersion(n, pSetSleepModeParams->version);

        auto context = ::GetContext(device);

        if (!context)
            return HandleInvalidated(n);

        VkLatencySleepModeInfoNV info, *pInfo;

        if (pSetSleepModeParams) {
            info = VkLatencySleepModeInfoNV{
                .sType = VK_STRUCTURE_TYPE_LATENCY_SLEEP_MODE_INFO_NV,
                .pNext = nullptr,
                .lowLatencyMode = pSetSleepModeParams->bLowLatencyMode,
                .lowLatencyBoost = pSetSleepModeParams->bLowLatencyBoost,
                .minimumIntervalUs = pSetSleepModeParams->minimumIntervalUs,
            };

            pInfo = &info;
        } else {
            info = VkLatencySleepModeInfoNV{};
            pInfo = nullptr;
        }

        auto vr = context->pvkSetLatencySleepModeNV(device, ::GetSwapchain(device), pInfo);

        if (vr != VK_SUCCESS)
            return Error(n, vr);

        context->lowLatencyMode = info.lowLatencyMode;

        return Ok(n, alreadyLoggedOk);
    }

    NvAPI_Status __cdecl NvAPI_Vulkan_Sleep(HANDLE vkDevice, NvU64 signalValue) {
        constexpr auto n = __func__;
        thread_local bool alreadyLoggedOk = false;

        auto device = reinterpret_cast<VkDevice>(vkDevice);

        if (log::tracing())
            log::trace(n, log::fmt::ptr(vkDevice), signalValue);

        if (!device)
            return InvalidArgument(n);

        auto context = ::GetContext(device);

        if (!context)
            return HandleInvalidated(n);

        auto info = VkLatencySleepInfoNV{
            .sType = VK_STRUCTURE_TYPE_LATENCY_SLEEP_INFO_NV,
            .pNext = nullptr,
            .signalSemaphore = context->semaphore,
            .value = signalValue,
        };

        auto vr = context->pvkLatencySleepNV(device, ::GetSwapchain(device), &info);

        return vr == VK_SUCCESS ? Ok(n, alreadyLoggedOk) : Error(n, vr);
    }

    NvAPI_Status __cdecl NvAPI_Vulkan_GetLatency(HANDLE vkDevice, NV_VULKAN_LATENCY_RESULT_PARAMS* pGetLatencyParams) {
        constexpr auto n = __func__;
        thread_local bool alreadyLoggedOk = false;

        auto device = reinterpret_cast<VkDevice>(vkDevice);

        if (log::tracing())
            log::trace(n, log::fmt::ptr(vkDevice), log::fmt::nv_vk_latency_result_params(pGetLatencyParams));

        if (!device)
            return InvalidArgument(n);

        if (!pGetLatencyParams)
            return InvalidPointer(n);

        if (pGetLatencyParams->version != NV_VULKAN_LATENCY_RESULT_PARAMS_VER1)
            return IncompatibleStructVersion(n, pGetLatencyParams->version);

        auto context = ::GetContext(device);

        if (!context)
            return HandleInvalidated(n);

        static constexpr auto count = sizeof(pGetLatencyParams->frameReport) / sizeof(*pGetLatencyParams->frameReport);

        std::array<VkLatencyTimingsFrameReportNV, count> timings;

        for (auto& timing : timings) {
            timing.sType = VK_STRUCTURE_TYPE_LATENCY_TIMINGS_FRAME_REPORT_NV;
            timing.pNext = nullptr;
        }

        auto info = VkGetLatencyMarkerInfoNV{
            .sType = VK_STRUCTURE_TYPE_GET_LATENCY_MARKER_INFO_NV,
            .pNext = nullptr,
            .timingCount = timings.size(),
            .pTimings = timings.data(),
        };

        context->pvkGetLatencyTimingsNV(device, ::GetSwapchain(device), &info);

        if (info.timingCount != count) {
            std::memset(pGetLatencyParams->frameReport, 0, sizeof(pGetLatencyParams->frameReport));
        } else {
            for (size_t i = 0; i < count; ++i) {
                std::memcpy(
                    &pGetLatencyParams->frameReport[i].frameID,
                    &timings[i].presentID,
                    offsetof(NV_VULKAN_LATENCY_RESULT_PARAMS::vkFrameReport, rsvd));
            }
        }

        return Ok(n, alreadyLoggedOk);
    }

    NvAPI_Status __cdecl NvAPI_Vulkan_SetLatencyMarker(HANDLE vkDevice, NV_VULKAN_LATENCY_MARKER_PARAMS* pSetLatencyMarkerParams) {
        constexpr auto n = __func__;
        thread_local bool alreadyLoggedOk = false;

        auto device = reinterpret_cast<VkDevice>(vkDevice);

        if (log::tracing())
            log::trace(n, log::fmt::ptr(vkDevice), log::fmt::nv_vk_latency_marker_params(pSetLatencyMarkerParams));

        if (!device)
            return InvalidArgument(n);

        if (!pSetLatencyMarkerParams)
            return InvalidPointer(n);

        if (pSetLatencyMarkerParams->version != NV_VULKAN_LATENCY_MARKER_PARAMS_VER1)
            return IncompatibleStructVersion(n, pSetLatencyMarkerParams->version);

        auto context = ::GetContext(device);

        if (!context)
            return HandleInvalidated(n);

        auto marker = ::ToVkLatencyMarkerNV(pSetLatencyMarkerParams->markerType);

        if (marker == VK_LATENCY_MARKER_MAX_ENUM_NV) {
            log::info(str::format("unsupported NV_VULKAN_LATENCY_MARKER_TYPE (", pSetLatencyMarkerParams->markerType, "), frameID = ", pSetLatencyMarkerParams->frameID, ", ignoring"));
            return Ok(n);
        }

        auto info = VkSetLatencyMarkerInfoNV{
            .sType = VK_STRUCTURE_TYPE_SET_LATENCY_MARKER_INFO_NV,
            .pNext = nullptr,
            .presentID = pSetLatencyMarkerParams->frameID,
            .marker = marker,
        };

        context->pvkSetLatencyMarkerNV(device, ::GetSwapchain(device), &info);

        return Ok(n, alreadyLoggedOk);
    }

    NvAPI_Status __cdecl NvAPI_Vulkan_NotifyOutOfBandVkQueue(HANDLE vkDevice, HANDLE queueHandle, NV_VULKAN_OUT_OF_BAND_QUEUE_TYPE queueType) {
        constexpr auto n = __func__;

        auto device = reinterpret_cast<VkDevice>(vkDevice);
        auto queue = reinterpret_cast<VkQueue>(queueHandle);

        if (log::tracing())
            log::trace(n, log::fmt::ptr(vkDevice), log::fmt::ptr(queueHandle), queueType);

        if (!vkDevice || !queueHandle)
            return InvalidArgument(n);

        auto context = ::GetContext(device);

        if (!context)
            return InvalidPointer(n);

        static_assert(static_cast<VkOutOfBandQueueTypeNV>(VULKAN_OUT_OF_BAND_QUEUE_TYPE_PRESENT) == VK_OUT_OF_BAND_QUEUE_TYPE_PRESENT_NV);
        static_assert(static_cast<VkOutOfBandQueueTypeNV>(VULKAN_OUT_OF_BAND_QUEUE_TYPE_RENDER) == VK_OUT_OF_BAND_QUEUE_TYPE_RENDER_NV);

        auto info = VkOutOfBandQueueTypeInfoNV{
            .sType = VK_STRUCTURE_TYPE_OUT_OF_BAND_QUEUE_TYPE_INFO_NV,
            .pNext = nullptr,
            .queueType = static_cast<VkOutOfBandQueueTypeNV>(queueType),
        };

        context->pvkQueueNotifyOutOfBandNV(queue, &info);

        return Ok(n);
    }
}