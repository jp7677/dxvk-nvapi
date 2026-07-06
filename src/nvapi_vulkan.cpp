#include "nvapi_private.h"
#include "nvapi_globals.h"
#include "nvapi/nvapi_vulkan_low_latency_device.h"
#include "util/util_statuscode.h"

using namespace dxvk;

NVAPI_FUNCTION NvAPI_Vulkan_InitLowLatencyDevice(HANDLE vkDevice, HANDLE* signalSemaphoreHandle) {
    static constexpr auto n = FUNC;

    if (log::tracing())
        log::trace(n, log::fmt::ptr(vkDevice), log::fmt::ptr(signalSemaphoreHandle));

    if (!resourceFactory)
        return ApiNotInitialized(n);

    NvapiVulkanLowLatencyDevice::Initialize(*resourceFactory);

    auto device = static_cast<VkDevice>(vkDevice);
    auto semaphore = reinterpret_cast<VkSemaphore*>(signalSemaphoreHandle);

    if (!device)
        return InvalidArgument(n);

    if (!semaphore)
        return InvalidPointer(n);

    auto [lowLatencyDevice, vr] = NvapiVulkanLowLatencyDevice::GetOrCreate(device);

    if (!lowLatencyDevice) {
        switch (vr) {
            case VK_ERROR_INCOMPATIBLE_DRIVER:
            case VK_ERROR_EXTENSION_NOT_PRESENT:
                return NotSupported(n);
            case VK_ERROR_FEATURE_NOT_PRESENT:
                return NoImplementation(n);
            default:
                return Error(n, vr);
        }
    }

    *semaphore = lowLatencyDevice->GetSemaphore();

    return Ok(n);
}

NVAPI_FUNCTION NvAPI_Vulkan_DestroyLowLatencyDevice(HANDLE vkDevice) {
    static constexpr auto n = FUNC;

    if (log::tracing())
        log::trace(n, log::fmt::ptr(vkDevice));

    auto device = static_cast<VkDevice>(vkDevice);

    if (!device)
        return InvalidArgument(n);

    return NvapiVulkanLowLatencyDevice::Destroy(device) ? Ok(n) : HandleInvalidated(n);
}

NVAPI_FUNCTION NvAPI_Vulkan_GetSleepStatus(HANDLE vkDevice, NV_VULKAN_GET_SLEEP_STATUS_PARAMS* pGetSleepStatusParams) {
    static constexpr auto n = FUNC;
    thread_local bool alreadyLoggedOk = false;
    thread_local bool alreadyLoggedHandleInvalidated = false;

    if (log::tracing())
        log::trace(n, log::fmt::ptr(vkDevice), log::fmt::nv_vk_get_sleep_status_params(pGetSleepStatusParams));

    auto device = static_cast<VkDevice>(vkDevice);

    if (!device)
        return InvalidArgument(n);

    if (!pGetSleepStatusParams)
        return InvalidPointer(n);

    if (pGetSleepStatusParams->version != NV_VULKAN_GET_SLEEP_STATUS_PARAMS_VER1)
        return IncompatibleStructVersion(n, pGetSleepStatusParams->version);

    auto lowLatencyDevice = NvapiVulkanLowLatencyDevice::Get(device);

    // TODO: check native behavior for this case
    if (!lowLatencyDevice)
        return HandleInvalidated(n, alreadyLoggedHandleInvalidated);

    pGetSleepStatusParams->bLowLatencyMode = lowLatencyDevice->GetLowLatencyMode();

    return Ok(n, alreadyLoggedOk);
}

NVAPI_FUNCTION NvAPI_Vulkan_SetSleepMode(HANDLE vkDevice, NV_VULKAN_SET_SLEEP_MODE_PARAMS* pSetSleepModeParams) {
    static constexpr auto n = FUNC;
    thread_local bool alreadyLoggedOk = false;
    thread_local bool alreadyLoggedHandleInvalidated = false;

    if (log::tracing())
        log::trace(n, log::fmt::ptr(vkDevice), log::fmt::nv_vk_set_sleep_status_params(pSetSleepModeParams));

    auto device = static_cast<VkDevice>(vkDevice);

    if (!device)
        return InvalidArgument(n);

    if (pSetSleepModeParams && pSetSleepModeParams->version != NV_VULKAN_SET_SLEEP_MODE_PARAMS_VER1)
        return IncompatibleStructVersion(n, pSetSleepModeParams->version);

    auto lowLatencyDevice = NvapiVulkanLowLatencyDevice::Get(device);

    if (!lowLatencyDevice)
        return HandleInvalidated(n, alreadyLoggedHandleInvalidated);

    auto vr = pSetSleepModeParams
        ? lowLatencyDevice->SetLatencySleepMode(pSetSleepModeParams->bLowLatencyMode, pSetSleepModeParams->bLowLatencyBoost, pSetSleepModeParams->minimumIntervalUs)
        : lowLatencyDevice->SetLatencySleepMode(nullptr);

    return vr == VK_SUCCESS ? Ok(n, alreadyLoggedOk) : Error(n, vr);
}

NVAPI_FUNCTION NvAPI_Vulkan_Sleep(HANDLE vkDevice, NvU64 signalValue) {
    static constexpr auto n = FUNC;
    thread_local bool alreadyLoggedOk = false;
    thread_local bool alreadyLoggedHandleInvalidated = false;

    if (log::tracing())
        log::trace(n, log::fmt::ptr(vkDevice), signalValue);

    auto device = static_cast<VkDevice>(vkDevice);

    if (!device)
        return InvalidArgument(n);

    auto lowLatencyDevice = NvapiVulkanLowLatencyDevice::Get(device);

    if (!lowLatencyDevice)
        return HandleInvalidated(n, alreadyLoggedHandleInvalidated);

    auto vr = lowLatencyDevice->LatencySleep(signalValue);

    return vr == VK_SUCCESS ? Ok(n, alreadyLoggedOk) : Error(n, vr);
}

NVAPI_FUNCTION NvAPI_Vulkan_GetLatency(HANDLE vkDevice, NV_VULKAN_LATENCY_RESULT_PARAMS* pGetLatencyParams) {
    static constexpr auto n = FUNC;
    thread_local bool alreadyLoggedOk = false;
    thread_local bool alreadyLoggedHandleInvalidated = false;

    if (log::tracing())
        log::trace(n, log::fmt::ptr(vkDevice), log::fmt::nv_vk_latency_result_params(pGetLatencyParams));

    auto device = static_cast<VkDevice>(vkDevice);

    if (!device)
        return InvalidArgument(n);

    if (!pGetLatencyParams)
        return InvalidPointer(n);

    if (pGetLatencyParams->version != NV_VULKAN_LATENCY_RESULT_PARAMS_VER1)
        return IncompatibleStructVersion(n, pGetLatencyParams->version);

    auto lowLatencyDevice = NvapiVulkanLowLatencyDevice::Get(device);

    if (!lowLatencyDevice)
        return HandleInvalidated(n, alreadyLoggedHandleInvalidated);

    static constexpr auto count = sizeof(pGetLatencyParams->frameReport) / sizeof(*pGetLatencyParams->frameReport);
    static_assert(count == 64);

    lowLatencyDevice->GetLatencyTimings(pGetLatencyParams->frameReport);

    return Ok(n, alreadyLoggedOk);
}

NVAPI_FUNCTION NvAPI_Vulkan_SetLatencyMarker(HANDLE vkDevice, NV_VULKAN_LATENCY_MARKER_PARAMS* pSetLatencyMarkerParams) {
    static constexpr auto n = FUNC;
    thread_local bool alreadyLoggedOk = false;
    thread_local bool alreadyLoggedHandleInvalidated = false;

    if (log::tracing())
        log::trace(n, log::fmt::ptr(vkDevice), log::fmt::nv_vk_latency_marker_params(pSetLatencyMarkerParams));

    auto device = static_cast<VkDevice>(vkDevice);

    if (!device)
        return InvalidArgument(n);

    if (!pSetLatencyMarkerParams)
        return InvalidPointer(n);

    if (pSetLatencyMarkerParams->version != NV_VULKAN_LATENCY_MARKER_PARAMS_VER1)
        return IncompatibleStructVersion(n, pSetLatencyMarkerParams->version);

    auto lowLatencyDevice = NvapiVulkanLowLatencyDevice::Get(device);

    if (!lowLatencyDevice)
        return HandleInvalidated(n, alreadyLoggedHandleInvalidated);

    auto markerType = pSetLatencyMarkerParams->markerType;

    if (!lowLatencyDevice->SetLatencyMarker(pSetLatencyMarkerParams->frameID, markerType)) {
        thread_local std::unordered_set<NV_VULKAN_LATENCY_MARKER_TYPE> unsupportedMarkerTypes{};

        if (auto [it, inserted] = unsupportedMarkerTypes.insert(markerType); inserted)
            log::info(str::format("unsupported NV_VULKAN_LATENCY_MARKER_TYPE (", markerType, "), ignoring"));
    }

    return Ok(n, alreadyLoggedOk);
}

NVAPI_FUNCTION NvAPI_Vulkan_NotifyOutOfBandVkQueue(HANDLE vkDevice, HANDLE queueHandle, NV_VULKAN_OUT_OF_BAND_QUEUE_TYPE queueType) {
    static constexpr auto n = FUNC;

    if (log::tracing())
        log::trace(n, log::fmt::ptr(vkDevice), log::fmt::ptr(queueHandle), queueType);

    auto device = static_cast<VkDevice>(vkDevice);
    auto queue = static_cast<VkQueue>(queueHandle);

    if (!device || !queue)
        return InvalidArgument(n);

    auto lowLatencyDevice = NvapiVulkanLowLatencyDevice::Get(device);

    if (!lowLatencyDevice)
        return HandleInvalidated(n);

    lowLatencyDevice->QueueNotifyOutOfBand(queue, queueType);

    return Ok(n);
}
