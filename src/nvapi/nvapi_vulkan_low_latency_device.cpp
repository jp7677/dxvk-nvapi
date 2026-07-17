#include "./nvapi_vulkan_low_latency_device.h"
#include "../util/util_env.h"
#include "../util/util_log.h"

namespace dxvk {

#define INVOKE(call)                                                            \
    switch (m_implementation) {                                                 \
        case LowLatencyDeviceImplementation::LowLatencyLegacy:                  \
            return static_cast<NvapiVulkanLowLatencyLegacyDevice*>(this)->call; \
        case LowLatencyDeviceImplementation::LowLatency2:                       \
            return static_cast<NvapiVulkanLowLatency2LayerDevice*>(this)->call; \
        case LowLatencyDeviceImplementation::VkReflexFake:                      \
            return static_cast<NvapiVulkanLowLatencyFakeDevice*>(this)->call;   \
        default:                                                                \
            __builtin_unreachable();                                            \
    }

    NvBool NvapiVulkanLowLatencyDevice::GetLowLatencyMode(){
        INVOKE(GetLowLatencyModeImpl()) // NOLINT(*-pro-type-static-cast-downcast)
    }

    VkResult NvapiVulkanLowLatencyDevice::SetLatencySleepMode(std::nullptr_t){
        INVOKE(SetLatencySleepModeImpl(nullptr)) // NOLINT(*-pro-type-static-cast-downcast)
    }

    VkResult NvapiVulkanLowLatencyDevice::SetLatencySleepMode(bool lowLatencyMode, bool lowLatencyBoost, uint32_t minimumIntervalUs){
        INVOKE(SetLatencySleepModeImpl(lowLatencyMode, lowLatencyBoost, minimumIntervalUs)) // NOLINT(*-pro-type-static-cast-downcast)
    }

    VkResult NvapiVulkanLowLatencyDevice::LatencySleep(uint64_t value) {
        INVOKE(LatencySleepImpl(value)) // NOLINT(*-pro-type-static-cast-downcast)
    }

    void NvapiVulkanLowLatencyDevice::GetLatencyTimings(std::span<NV_VULKAN_LATENCY_RESULT_PARAMS_V1::vkFrameReport, 64> frameReports) {
        INVOKE(GetLatencyTimingsImpl(frameReports)) // NOLINT(*-pro-type-static-cast-downcast)
    }

    bool NvapiVulkanLowLatencyDevice::SetLatencyMarker(uint64_t frameID, NV_VULKAN_LATENCY_MARKER_TYPE marker) {
        INVOKE(SetLatencyMarkerImpl(frameID, marker)) // NOLINT(*-pro-type-static-cast-downcast)
    }

    void NvapiVulkanLowLatencyDevice::QueueNotifyOutOfBand(VkQueue queue, NV_VULKAN_OUT_OF_BAND_QUEUE_TYPE queueType){
        INVOKE(QueueNotifyOutOfBandImpl(queue, queueType)) // NOLINT(*-pro-type-static-cast-downcast)
    }

#undef INVOKE

#define PFN_PARAM(proc) PFN_##proc proc
#define PFN_INIT(proc) m_##proc(proc)

    std::pair<VkSemaphore, VkResult> CreateVkSemaphore(VkDevice device, PFN_PARAM(vkCreateSemaphore)) {
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

        VkSemaphore semaphore;
        auto vr = vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphore);

        return {semaphore, vr};
    }

#define VK_GET_DEVICE_PROC_ADDR(proc) auto proc = reinterpret_cast<PFN_##proc>(vk->GetDeviceProcAddr(device, #proc))

    // NvapiVulkanLowLatencyLegacyDevice

    std::pair<std::unique_ptr<NvapiVulkanLowLatencyLegacyDevice>, VkResult> NvapiVulkanLowLatencyLegacyDevice::TryCreate(Vk* vk, VkDevice device) {
        VK_GET_DEVICE_PROC_ADDR(vkCreateSemaphore);
        VK_GET_DEVICE_PROC_ADDR(vkDestroySemaphore);

        if (!vkCreateSemaphore || !vkDestroySemaphore) {
            log::info("Initializing Vulkan Low-Latency with VkNvLowLatencyLegacy implementation failed: device does not appear to support semaphores?!");
            return {nullptr, VK_ERROR_INCOMPATIBLE_DRIVER};
        }

        VK_GET_DEVICE_PROC_ADDR(vkSetLatencySleepModeLegacyNV);
        VK_GET_DEVICE_PROC_ADDR(vkLatencySleepLegacyNV);
        VK_GET_DEVICE_PROC_ADDR(vkGetLatencyTimingsLegacyNV);
        VK_GET_DEVICE_PROC_ADDR(vkSetLatencyMarkerLegacyNV);
        VK_GET_DEVICE_PROC_ADDR(vkQueueNotifyOutOfBandLegacyNV);
        VK_GET_DEVICE_PROC_ADDR(vkGetSleepStatusLegacyNV);
        VK_GET_DEVICE_PROC_ADDR(vkShutdownLatencyDeviceLegacyNV);

        if (!(vkSetLatencySleepModeLegacyNV && vkLatencySleepLegacyNV && vkGetLatencyTimingsLegacyNV && vkSetLatencyMarkerLegacyNV && vkQueueNotifyOutOfBandLegacyNV && vkGetSleepStatusLegacyNV && vkShutdownLatencyDeviceLegacyNV)) {
            log::info("Initializing Vulkan Low-Latency with VkNvLowLatencyLegacy implementation failed, device (or winevulkan) does not support revision 2 of VK_NV_low_latency extension");
            return {nullptr, VK_ERROR_EXTENSION_NOT_PRESENT};
        }

        auto [semaphore, vr] = CreateVkSemaphore(device, vkCreateSemaphore);

        if (vr != VK_SUCCESS) {
            log::info(str::format("Initializing Vulkan Low-Latency with VkNvLowLatencyLegacy implementation failed: create semaphore failed (", vr, ")?!"));
            return {nullptr, vr};
        }

        auto lowLatencyDevice = std::make_unique<NvapiVulkanLowLatencyLegacyDevice>(
            device,
            semaphore,
            vkDestroySemaphore,
            vkSetLatencySleepModeLegacyNV,
            vkLatencySleepLegacyNV,
            vkGetLatencyTimingsLegacyNV,
            vkSetLatencyMarkerLegacyNV,
            vkQueueNotifyOutOfBandLegacyNV,
            vkGetSleepStatusLegacyNV,
            vkShutdownLatencyDeviceLegacyNV);

        log::info("Successfully initialized Vulkan Low-Latency with VkNvLowLatencyLegacy implementation");
        return {std::move(lowLatencyDevice), VK_SUCCESS};
    }

    NvapiVulkanLowLatencyLegacyDevice::NvapiVulkanLowLatencyLegacyDevice(
        VkDevice device,
        VkSemaphore semaphore,
        PFN_PARAM(vkDestroySemaphore),
        PFN_PARAM(vkSetLatencySleepModeLegacyNV),
        PFN_PARAM(vkLatencySleepLegacyNV),
        PFN_PARAM(vkGetLatencyTimingsLegacyNV),
        PFN_PARAM(vkSetLatencyMarkerLegacyNV),
        PFN_PARAM(vkQueueNotifyOutOfBandLegacyNV),
        PFN_PARAM(vkGetSleepStatusLegacyNV),
        PFN_PARAM(vkShutdownLatencyDeviceLegacyNV))
        : NvapiVulkanLowLatencyDevice(LowLatencyDeviceImplementation::LowLatencyLegacy, device, semaphore, vkDestroySemaphore),
          PFN_INIT(vkSetLatencySleepModeLegacyNV),
          PFN_INIT(vkLatencySleepLegacyNV),
          PFN_INIT(vkGetLatencyTimingsLegacyNV),
          PFN_INIT(vkSetLatencyMarkerLegacyNV),
          PFN_INIT(vkQueueNotifyOutOfBandLegacyNV),
          PFN_INIT(vkGetSleepStatusLegacyNV),
          PFN_INIT(vkShutdownLatencyDeviceLegacyNV) {}

    NvBool NvapiVulkanLowLatencyLegacyDevice::GetLowLatencyModeImpl() {
        VkBool32 lowLatencyMode;

        m_vkGetSleepStatusLegacyNV(m_device, &lowLatencyMode);

        return lowLatencyMode ? NV_TRUE : NV_FALSE;
    }

    VkResult NvapiVulkanLowLatencyLegacyDevice::SetLatencySleepModeImpl(std::nullptr_t) {
        m_vkSetLatencySleepModeLegacyNV(m_device, VK_FALSE, VK_FALSE, 0);

        return VK_SUCCESS;
    }

    VkResult NvapiVulkanLowLatencyLegacyDevice::SetLatencySleepModeImpl(bool lowLatencyMode, bool lowLatencyBoost, uint32_t minimumIntervalUs) {
        m_vkSetLatencySleepModeLegacyNV(m_device, lowLatencyMode ? VK_TRUE : VK_FALSE, lowLatencyBoost ? VK_TRUE : VK_FALSE, minimumIntervalUs);

        return VK_SUCCESS;
    }

    VkResult NvapiVulkanLowLatencyLegacyDevice::LatencySleepImpl(uint64_t value) {
        m_vkLatencySleepLegacyNV(m_device, m_semaphore, value);

        return VK_SUCCESS;
    }

    void NvapiVulkanLowLatencyLegacyDevice::GetLatencyTimingsImpl(std::span<NV_VULKAN_LATENCY_RESULT_PARAMS_V1::vkFrameReport, 64> frameReports) {
        m_vkGetLatencyTimingsLegacyNV(m_device, frameReports.data());
    }

    bool NvapiVulkanLowLatencyLegacyDevice::SetLatencyMarkerImpl(uint64_t frameID, NV_VULKAN_LATENCY_MARKER_TYPE marker) {
        m_vkSetLatencyMarkerLegacyNV(m_device, frameID, marker);

        return true;
    }

    void NvapiVulkanLowLatencyLegacyDevice::QueueNotifyOutOfBandImpl(VkQueue queue, NV_VULKAN_OUT_OF_BAND_QUEUE_TYPE queueType) {
        m_vkQueueNotifyOutOfBandLegacyNV(queue, queueType);
    }

    // NvapiVulkanLowLatency2LayerDevice

    std::pair<std::unique_ptr<NvapiVulkanLowLatency2LayerDevice>, VkResult> NvapiVulkanLowLatency2LayerDevice::TryCreate(Vk* vk, VkDevice device) {
        VK_GET_DEVICE_PROC_ADDR(vkCreateSemaphore);
        VK_GET_DEVICE_PROC_ADDR(vkDestroySemaphore);

        if (!vkCreateSemaphore || !vkDestroySemaphore) {
            log::info("Initializing Vulkan Low-Latency with VkNvLowLatency2 implementation failed: device does not appear to support semaphores?!");
            return {nullptr, VK_ERROR_INCOMPATIBLE_DRIVER};
        }

        // If the Vulkan Reflex Layer is present it will enable VK_NV_low_latency2 which will let us query
        // these function pointers.
        VK_GET_DEVICE_PROC_ADDR(vkSetLatencySleepModeNV);
        VK_GET_DEVICE_PROC_ADDR(vkLatencySleepNV);
        VK_GET_DEVICE_PROC_ADDR(vkGetLatencyTimingsNV);
        VK_GET_DEVICE_PROC_ADDR(vkSetLatencyMarkerNV);
        VK_GET_DEVICE_PROC_ADDR(vkQueueNotifyOutOfBandNV);

        if (!(vkSetLatencySleepModeNV && vkLatencySleepNV && vkGetLatencyTimingsNV && vkSetLatencyMarkerNV && vkQueueNotifyOutOfBandNV)) {
            log::info("Initializing Vulkan Low-Latency with VkNvLowLatency2 implementation failed, DXVK-NVAPI's Vulkan layer is not present");
            return {nullptr, VK_ERROR_EXTENSION_NOT_PRESENT};
        }

        // VK_NV_low_latency2 was requested -> our compatibility layer is present

        auto [semaphore, vr] = CreateVkSemaphore(device, vkCreateSemaphore);

        if (vr != VK_SUCCESS) {
            log::info(str::format("Initializing Vulkan Low-Latency with VkNvLowLatency2 implementation failed: create semaphore failed (", vr, ")?!"));
            return {nullptr, vr};
        }

        auto lowLatencyDevice = std::make_unique<NvapiVulkanLowLatency2LayerDevice>(
            device,
            semaphore,
            vkDestroySemaphore,
            vkSetLatencySleepModeNV,
            vkLatencySleepNV,
            vkGetLatencyTimingsNV,
            vkSetLatencyMarkerNV,
            vkQueueNotifyOutOfBandNV);

        log::info("Successfully initialized Vulkan Low-Latency with VkNvLowLatency2 implementation, DXVK-NVAPI's Vulkan layer is present");
        return {std::move(lowLatencyDevice), VK_SUCCESS};
    }

    NvapiVulkanLowLatency2LayerDevice::NvapiVulkanLowLatency2LayerDevice(
        VkDevice device,
        VkSemaphore semaphore,
        PFN_PARAM(vkDestroySemaphore),
        PFN_PARAM(vkSetLatencySleepModeNV),
        PFN_PARAM(vkLatencySleepNV),
        PFN_PARAM(vkGetLatencyTimingsNV),
        PFN_PARAM(vkSetLatencyMarkerNV),
        PFN_PARAM(vkQueueNotifyOutOfBandNV))
        : NvapiVulkanLowLatencyDevice(LowLatencyDeviceImplementation::LowLatency2, device, semaphore, vkDestroySemaphore),
          PFN_INIT(vkSetLatencySleepModeNV),
          PFN_INIT(vkLatencySleepNV),
          PFN_INIT(vkGetLatencyTimingsNV),
          PFN_INIT(vkSetLatencyMarkerNV),
          PFN_INIT(vkQueueNotifyOutOfBandNV) {}

    NvBool NvapiVulkanLowLatency2LayerDevice::GetLowLatencyModeImpl() {
        return m_lowLatencyMode ? NV_TRUE : NV_FALSE;
    }

    static inline VkSwapchainKHR GetSwapchain(VkDevice device) {
        // winevulkan expects valid Vulkan usage so it never checks if swapchains passed to VK_NV_low_latency2 functions
        // are null handles or not, it just unconditionally dereferences them, and we just need our calls to make it
        // to the Linux-side Vulkan layer without crashing in the meantime, this hack makes winevulkan happy enough
        return reinterpret_cast<VkSwapchainKHR>(device);
    }

    VkResult NvapiVulkanLowLatency2LayerDevice::SetLatencySleepModeImpl(std::nullptr_t) {
        auto vr = m_vkSetLatencySleepModeNV(m_device, GetSwapchain(m_device), nullptr);

        if (vr == VK_SUCCESS)
            m_lowLatencyMode = false;

        return vr;
    }

    VkResult NvapiVulkanLowLatency2LayerDevice::SetLatencySleepModeImpl(bool lowLatencyMode, bool lowLatencyBoost, uint32_t minimumIntervalUs) {
        auto info = VkLatencySleepModeInfoNV{
            .sType = VK_STRUCTURE_TYPE_LATENCY_SLEEP_MODE_INFO_NV,
            .pNext = nullptr,
            .lowLatencyMode = lowLatencyMode,
            .lowLatencyBoost = lowLatencyBoost,
            .minimumIntervalUs = minimumIntervalUs,
        };

        auto vr = m_vkSetLatencySleepModeNV(m_device, GetSwapchain(m_device), &info);

        if (vr == VK_SUCCESS)
            m_lowLatencyMode = lowLatencyMode;

        return vr;
    }

    VkResult NvapiVulkanLowLatency2LayerDevice::LatencySleepImpl(uint64_t value) {
        auto info = VkLatencySleepInfoNV{
            .sType = VK_STRUCTURE_TYPE_LATENCY_SLEEP_INFO_NV,
            .pNext = nullptr,
            .signalSemaphore = m_semaphore,
            .value = value,
        };

        return m_vkLatencySleepNV(m_device, GetSwapchain(m_device), &info);
    }

    void NvapiVulkanLowLatency2LayerDevice::GetLatencyTimingsImpl(std::span<NV_VULKAN_LATENCY_RESULT_PARAMS_V1::vkFrameReport, 64> frameReports) {
        std::array<VkLatencyTimingsFrameReportNV, 64> latencyTimingsFrameReports;

        for (auto& report : latencyTimingsFrameReports) {
            report.sType = VK_STRUCTURE_TYPE_LATENCY_TIMINGS_FRAME_REPORT_NV;
            report.pNext = nullptr;
        }

        auto info = VkGetLatencyMarkerInfoNV{
            .sType = VK_STRUCTURE_TYPE_GET_LATENCY_MARKER_INFO_NV,
            .pNext = nullptr,
            .timingCount = 64,
            .pTimings = latencyTimingsFrameReports.data(),
        };

        m_vkGetLatencyTimingsNV(m_device, GetSwapchain(m_device), &info);

        if (info.timingCount == 64) {
            for (size_t i = 0; i < 64; ++i) {
                std::memcpy(
                    &frameReports[i].frameID,
                    &latencyTimingsFrameReports[i].presentID,
                    offsetof(NV_VULKAN_LATENCY_RESULT_PARAMS::vkFrameReport, rsvd));
            }
        } else {
            std::memset(frameReports.data(), 0, frameReports.size_bytes());
        }
    }

    static inline VkLatencyMarkerNV ToVkLatencyMarkerNV(NV_VULKAN_LATENCY_MARKER_TYPE marker) {
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

    bool NvapiVulkanLowLatency2LayerDevice::SetLatencyMarkerImpl(uint64_t frameID, NV_VULKAN_LATENCY_MARKER_TYPE marker) {
        auto markerType = ToVkLatencyMarkerNV(marker);

        if (markerType == VK_LATENCY_MARKER_MAX_ENUM_NV)
            return false;

        auto info = VkSetLatencyMarkerInfoNV{
            .sType = VK_STRUCTURE_TYPE_SET_LATENCY_MARKER_INFO_NV,
            .pNext = nullptr,
            .presentID = frameID,
            .marker = markerType,
        };

        m_vkSetLatencyMarkerNV(m_device, GetSwapchain(m_device), &info);

        return true;
    }

    void NvapiVulkanLowLatency2LayerDevice::QueueNotifyOutOfBandImpl(VkQueue queue, NV_VULKAN_OUT_OF_BAND_QUEUE_TYPE queueType) {
        static_assert(static_cast<VkOutOfBandQueueTypeNV>(VULKAN_OUT_OF_BAND_QUEUE_TYPE_PRESENT) == VK_OUT_OF_BAND_QUEUE_TYPE_PRESENT_NV);
        static_assert(static_cast<VkOutOfBandQueueTypeNV>(VULKAN_OUT_OF_BAND_QUEUE_TYPE_RENDER) == VK_OUT_OF_BAND_QUEUE_TYPE_RENDER_NV);

        auto info = VkOutOfBandQueueTypeInfoNV{
            .sType = VK_STRUCTURE_TYPE_OUT_OF_BAND_QUEUE_TYPE_INFO_NV,
            .pNext = nullptr,
            .queueType = static_cast<VkOutOfBandQueueTypeNV>(queueType),
        };

        m_vkQueueNotifyOutOfBandNV(queue, &info);
    }

    // NvapiVulkanLowLatencyFakeDevice

    std::pair<std::unique_ptr<NvapiVulkanLowLatencyFakeDevice>, VkResult> NvapiVulkanLowLatencyFakeDevice::TryCreate(Vk* vk, VkDevice device) {
        VK_GET_DEVICE_PROC_ADDR(vkCreateSemaphore);
        VK_GET_DEVICE_PROC_ADDR(vkDestroySemaphore);

        if (!vkCreateSemaphore || !vkDestroySemaphore) {
            log::info("Initializing Vulkan Low-Latency with VkFakeReflex implementation failed: device does not appear to support semaphores?!");
            return {nullptr, VK_ERROR_INCOMPATIBLE_DRIVER};
        }

        auto fakeVkReflex = env::getEnvVariable("DXVK_NVAPI_FAKE_VKREFLEX");
        if ((!env::needsLowLatencyDevice() && fakeVkReflex != "1") || fakeVkReflex == "0") {
            log::info("Initializing Vulkan Low-Latency with VkFakeReflex implementation failed: DXVK_NVAPI_FAKE_VKREFLEX not set");
            return {nullptr, VK_ERROR_FEATURE_NOT_PRESENT};
        }

        // Grab vkSignalSemaphore to fake that Reflex is working
        // The app should have requested either the Vulkan 1.2 or the VK_KHR_timeline_semaphore extension
        // We'll use whichever is available
        VK_GET_DEVICE_PROC_ADDR(vkSignalSemaphore);

        if (!(vkSignalSemaphore || (vkSignalSemaphore = reinterpret_cast<PFN_vkSignalSemaphoreKHR>(vk->GetDeviceProcAddr(device, "vkSignalSemaphoreKHR"))))) {
            log::info("Initializing Vulkan Low-Latency with VkFakeReflex implementation failed: could not find vkSignalSemaphore commands in VkDevice's dispatch table");
            return {nullptr, VK_ERROR_EXTENSION_NOT_PRESENT};
        }

        auto [semaphore, vr] = CreateVkSemaphore(device, vkCreateSemaphore);

        if (vr != VK_SUCCESS) {
            log::info(str::format("Initializing Vulkan Low-Latency with VkFakeReflex implementation failed: create semaphore failed (", vr, ")?!"));
            return {nullptr, vr};
        }

        // Pretend that Reflex is happening so that apps don't get a pink tint.
        auto lowLatencyDevice = std::make_unique<NvapiVulkanLowLatencyFakeDevice>(
            device,
            semaphore,
            vkDestroySemaphore,
            vkSignalSemaphore);

        log::info("Successfully initialized Vulkan Low-Latency with VkFakeReflex implementation: faking success as a workaround but latency will not be reduced");
        return {std::move(lowLatencyDevice), VK_SUCCESS};
    }

    NvapiVulkanLowLatencyFakeDevice::NvapiVulkanLowLatencyFakeDevice(
        VkDevice device,
        VkSemaphore semaphore,
        PFN_PARAM(vkDestroySemaphore),
        PFN_PARAM(vkSignalSemaphore))
        : NvapiVulkanLowLatencyDevice(LowLatencyDeviceImplementation::VkReflexFake, device, semaphore, vkDestroySemaphore),
          PFN_INIT(vkSignalSemaphore) {}

    NvBool NvapiVulkanLowLatencyFakeDevice::GetLowLatencyModeImpl() {
        return NV_FALSE;
    }

    VkResult NvapiVulkanLowLatencyFakeDevice::SetLatencySleepModeImpl(std::nullptr_t) {
        return VK_SUCCESS;
    }

    VkResult NvapiVulkanLowLatencyFakeDevice::SetLatencySleepModeImpl(bool lowLatencyMode, bool lowLatencyBoost, uint32_t minimumIntervalUs) {
        return VK_SUCCESS;
    }

    VkResult NvapiVulkanLowLatencyFakeDevice::LatencySleepImpl(uint64_t value) {
        auto info = VkSemaphoreSignalInfoKHR{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO_KHR,
            .pNext = nullptr,
            .semaphore = m_semaphore,
            .value = value};

        return m_vkSignalSemaphore(m_device, &info);
    }

    void NvapiVulkanLowLatencyFakeDevice::GetLatencyTimingsImpl(std::span<NV_VULKAN_LATENCY_RESULT_PARAMS_V1::vkFrameReport, 64> frameReports) {
        memset(frameReports.data(), 0, frameReports.size_bytes());
    }

    bool NvapiVulkanLowLatencyFakeDevice::SetLatencyMarkerImpl(uint64_t frameID, NV_VULKAN_LATENCY_MARKER_TYPE marker) {
        return true;
    }

    void NvapiVulkanLowLatencyFakeDevice::QueueNotifyOutOfBandImpl(VkQueue queue, NV_VULKAN_OUT_OF_BAND_QUEUE_TYPE queueType) {}
}
