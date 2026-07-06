#include "./nvapi_vulkan_low_latency_device.h"
#include "../util/util_env.h"
#include "../util/util_log.h"

namespace dxvk {
    std::unique_ptr<Vk> NvapiVulkanLowLatencyDevice::m_vk = nullptr;
    std::unordered_map<VkDevice, std::unique_ptr<NvapiVulkanLowLatencyDevice>> NvapiVulkanLowLatencyDevice::m_nvapiDeviceMap = {};
    std::mutex NvapiVulkanLowLatencyDevice::m_mutex = {};

    bool NvapiVulkanLowLatencyDevice::Initialize(NvapiResourceFactory& resourceFactory) {
        std::scoped_lock lock{m_mutex};

        if (m_vk && m_vk->IsAvailable())
            return true;

        m_vk = resourceFactory.CreateVulkan("vulkan-1.dll");

        if (m_vk && m_vk->IsAvailable())
            return true;

        m_vk = resourceFactory.CreateVulkan("winevulkan.dll");

        return m_vk && m_vk->IsAvailable();
    }

    void NvapiVulkanLowLatencyDevice::Reset() {
        std::scoped_lock lock{m_mutex};

        m_nvapiDeviceMap.clear();
        m_vk.reset();
    }

    std::pair<NvapiVulkanLowLatencyDevice*, VkResult> NvapiVulkanLowLatencyDevice::GetOrCreate(VkDevice device) {
        std::scoped_lock lock{m_mutex};

        if (auto lowLatencyDevice = Get(device))
            return {lowLatencyDevice, VK_SUCCESS};

        if (!m_vk || !m_vk->IsAvailable()) {
            log::info("Initializing Vulkan Low-Latency failed: could not find usable Vulkan loader (or winevulkan) module in the current process");
            return {nullptr, VK_ERROR_INITIALIZATION_FAILED};
        }

        std::unique_ptr<NvapiVulkanLowLatencyDevice> lowLatencyDevice;
        VkResult vr;

        std::tie(lowLatencyDevice, vr) = NvapiVulkanLowLatency2LayerDevice::TryCreate(device);

        if (!lowLatencyDevice || vr != VK_SUCCESS)
            std::tie(lowLatencyDevice, vr) = NvapiVulkanLowLatencyFakeDevice::TryCreate(device);

        if (!lowLatencyDevice || vr != VK_SUCCESS)
            return {nullptr, vr};

        auto [it, inserted] = m_nvapiDeviceMap.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(device),
            std::forward_as_tuple(std::move(lowLatencyDevice)));

        if (!inserted)
            return {nullptr, VK_ERROR_UNKNOWN};

        return {it->second.get(), vr};
    }

    NvapiVulkanLowLatencyDevice* NvapiVulkanLowLatencyDevice::Get(VkDevice device) {
        auto it = m_nvapiDeviceMap.find(device);
        return it == m_nvapiDeviceMap.end() ? nullptr : it->second.get();
    }

    bool NvapiVulkanLowLatencyDevice::Destroy(VkDevice device) {
        std::scoped_lock lock{m_mutex};

        auto node = m_nvapiDeviceMap.extract(device);

        if (node.empty())
            return false;

        auto lowLatencyDevice = std::move(node.mapped());
        // the destructor will take care of semaphore cleanup

        return true;
    }

#define PFN_PARAM(proc) PFN_##proc proc
#define PFN_INIT(proc) m_##proc(proc)
    NvapiVulkanLowLatencyDevice::NvapiVulkanLowLatencyDevice(
        VkDevice device,
        VkSemaphore semaphore,
        PFN_PARAM(vkDestroySemaphore))
        : m_device(device),
          m_semaphore(semaphore),
          PFN_INIT(vkDestroySemaphore) {}

    VkSemaphore NvapiVulkanLowLatencyDevice::GetSemaphore() const {
        return m_semaphore;
    }

    std::pair<VkSemaphore, VkResult> NvapiVulkanLowLatencyDevice::CreateVkSemaphore(VkDevice device, PFN_PARAM(vkCreateSemaphore)) {
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

    NvapiVulkanLowLatencyDevice::~NvapiVulkanLowLatencyDevice() {
        m_vkDestroySemaphore(m_device, m_semaphore, nullptr);
    }

#define VK_GET_DEVICE_PROC_ADDR(proc) auto proc = reinterpret_cast<PFN_##proc>(m_vk->GetDeviceProcAddr(device, #proc))

    NvapiVulkanLowLatencyFakeDevice::NvapiVulkanLowLatencyFakeDevice(
        VkDevice device,
        VkSemaphore semaphore,
        PFN_PARAM(vkDestroySemaphore),
        PFN_PARAM(vkSignalSemaphore))
        : NvapiVulkanLowLatencyDevice(device, semaphore, vkDestroySemaphore),
          PFN_INIT(vkSignalSemaphore) {}

    // NvapiVulkanLowLatencyFakeDevice

    std::pair<std::unique_ptr<NvapiVulkanLowLatencyFakeDevice>, VkResult> NvapiVulkanLowLatencyFakeDevice::TryCreate(VkDevice device) {
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

        if (!(vkSignalSemaphore || (vkSignalSemaphore = reinterpret_cast<PFN_vkSignalSemaphoreKHR>(m_vk->GetDeviceProcAddr(device, "vkSignalSemaphoreKHR"))))) {
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

    NvBool NvapiVulkanLowLatencyFakeDevice::GetLowLatencyMode() const {
        return NV_FALSE;
    }

    VkResult NvapiVulkanLowLatencyFakeDevice::SetLatencySleepMode(std::nullptr_t) {
        return VK_SUCCESS;
    }

    VkResult NvapiVulkanLowLatencyFakeDevice::SetLatencySleepMode(bool lowLatencyMode, bool lowLatencyBoost, uint32_t minimumIntervalUs) {
        return VK_SUCCESS;
    }

    VkResult NvapiVulkanLowLatencyFakeDevice::LatencySleep(uint64_t value) {
        auto info = VkSemaphoreSignalInfoKHR{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO_KHR,
            .pNext = nullptr,
            .semaphore = m_semaphore,
            .value = value};

        return m_vkSignalSemaphore(m_device, &info);
    }

    void NvapiVulkanLowLatencyFakeDevice::GetLatencyTimings(std::span<NV_VULKAN_LATENCY_RESULT_PARAMS_V1::vkFrameReport, 64> frameReports) {
        memset(frameReports.data(), 0, frameReports.size_bytes());
    }

    bool NvapiVulkanLowLatencyFakeDevice::SetLatencyMarker(uint64_t frameID, NV_VULKAN_LATENCY_MARKER_TYPE marker) {
        return true;
    }

    void NvapiVulkanLowLatencyFakeDevice::QueueNotifyOutOfBand(VkQueue queue, NV_VULKAN_OUT_OF_BAND_QUEUE_TYPE queueType) {}

    // NvapiVulkanLowLatency2LayerDevice

    NvapiVulkanLowLatency2LayerDevice::NvapiVulkanLowLatency2LayerDevice(
        VkDevice device,
        VkSemaphore semaphore,
        PFN_PARAM(vkDestroySemaphore),
        PFN_PARAM(vkSetLatencySleepModeNV),
        PFN_PARAM(vkLatencySleepNV),
        PFN_PARAM(vkGetLatencyTimingsNV),
        PFN_PARAM(vkSetLatencyMarkerNV),
        PFN_PARAM(vkQueueNotifyOutOfBandNV))
        : NvapiVulkanLowLatencyDevice(device, semaphore, vkDestroySemaphore),
          PFN_INIT(vkSetLatencySleepModeNV),
          PFN_INIT(vkLatencySleepNV),
          PFN_INIT(vkGetLatencyTimingsNV),
          PFN_INIT(vkSetLatencyMarkerNV),
          PFN_INIT(vkQueueNotifyOutOfBandNV) {}

    std::pair<std::unique_ptr<NvapiVulkanLowLatency2LayerDevice>, VkResult> NvapiVulkanLowLatency2LayerDevice::TryCreate(VkDevice device) {
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

    NvBool NvapiVulkanLowLatency2LayerDevice::GetLowLatencyMode() const {
        return m_lowLatencyMode ? NV_TRUE : NV_FALSE;
    }

    static inline VkSwapchainKHR GetSwapchain(VkDevice device) {
        // winevulkan expects valid Vulkan usage so it never checks if swapchains passed to VK_NV_low_latency2 functions
        // are null handles or not, it just unconditionally dereferences them, and we just need our calls to make it
        // to the Linux-side Vulkan layer without crashing in the meantime, this hack makes winevulkan happy enough
        return reinterpret_cast<VkSwapchainKHR>(device);
    }

    VkResult NvapiVulkanLowLatency2LayerDevice::SetLatencySleepMode(std::nullptr_t) {
        auto vr = m_vkSetLatencySleepModeNV(m_device, GetSwapchain(m_device), nullptr);

        if (vr == VK_SUCCESS)
            m_lowLatencyMode = false;

        return vr;
    }

    VkResult NvapiVulkanLowLatency2LayerDevice::SetLatencySleepMode(bool lowLatencyMode, bool lowLatencyBoost, uint32_t minimumIntervalUs) {
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

    VkResult NvapiVulkanLowLatency2LayerDevice::LatencySleep(uint64_t value) {
        auto info = VkLatencySleepInfoNV{
            .sType = VK_STRUCTURE_TYPE_LATENCY_SLEEP_INFO_NV,
            .pNext = nullptr,
            .signalSemaphore = m_semaphore,
            .value = value,
        };

        return m_vkLatencySleepNV(m_device, GetSwapchain(m_device), &info);
    }

    void NvapiVulkanLowLatency2LayerDevice::GetLatencyTimings(std::span<NV_VULKAN_LATENCY_RESULT_PARAMS_V1::vkFrameReport, 64> frameReports) {
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

    bool NvapiVulkanLowLatency2LayerDevice::SetLatencyMarker(uint64_t frameID, NV_VULKAN_LATENCY_MARKER_TYPE marker) {
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

    void NvapiVulkanLowLatency2LayerDevice::QueueNotifyOutOfBand(VkQueue queue, NV_VULKAN_OUT_OF_BAND_QUEUE_TYPE queueType) {
        static_assert(static_cast<VkOutOfBandQueueTypeNV>(VULKAN_OUT_OF_BAND_QUEUE_TYPE_PRESENT) == VK_OUT_OF_BAND_QUEUE_TYPE_PRESENT_NV);
        static_assert(static_cast<VkOutOfBandQueueTypeNV>(VULKAN_OUT_OF_BAND_QUEUE_TYPE_RENDER) == VK_OUT_OF_BAND_QUEUE_TYPE_RENDER_NV);

        auto info = VkOutOfBandQueueTypeInfoNV{
            .sType = VK_STRUCTURE_TYPE_OUT_OF_BAND_QUEUE_TYPE_INFO_NV,
            .pNext = nullptr,
            .queueType = static_cast<VkOutOfBandQueueTypeNV>(queueType),
        };

        m_vkQueueNotifyOutOfBandNV(queue, &info);
    }
}
