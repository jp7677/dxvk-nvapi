#include "./nvapi_vulkan_low_latency_device.h"

namespace dxvk {
    std::unique_ptr<Vk> NvapiVulkanLowLatencyDevice::m_vk = nullptr;
    std::unordered_map<VkDevice, NvapiVulkanLowLatencyDevice> NvapiVulkanLowLatencyDevice::m_nvapiDeviceMap = {};
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

        for (auto& [_, lowLatencyDevice] : m_nvapiDeviceMap)
            lowLatencyDevice.m_vkDestroySemaphore(lowLatencyDevice.m_device, lowLatencyDevice.m_semaphore, nullptr);

        m_nvapiDeviceMap.clear();
        m_vk.reset();
    }

    std::pair<NvapiVulkanLowLatencyDevice*, VkResult> NvapiVulkanLowLatencyDevice::GetOrCreate(VkDevice device) {
        std::scoped_lock lock{m_mutex};

        if (auto lowLatencyDevice = Get(device))
            return {lowLatencyDevice, VK_SUCCESS};

        if (!m_vk || !m_vk->IsAvailable())
            return {nullptr, VK_ERROR_INITIALIZATION_FAILED};

#define VK_GET_DEVICE_PROC_ADDR(proc) auto proc = reinterpret_cast<PFN_##proc>(m_vk->GetDeviceProcAddr(device, #proc))

        VK_GET_DEVICE_PROC_ADDR(vkCreateSemaphore);
        VK_GET_DEVICE_PROC_ADDR(vkDestroySemaphore);
        VK_GET_DEVICE_PROC_ADDR(vkSetLatencySleepModeNV);
        VK_GET_DEVICE_PROC_ADDR(vkLatencySleepNV);
        VK_GET_DEVICE_PROC_ADDR(vkGetLatencyTimingsNV);
        VK_GET_DEVICE_PROC_ADDR(vkSetLatencyMarkerNV);
        VK_GET_DEVICE_PROC_ADDR(vkQueueNotifyOutOfBandNV);
        VK_GET_DEVICE_PROC_ADDR(vkSignalSemaphore);

        if (!vkSetLatencySleepModeNV || !vkLatencySleepNV || !vkGetLatencyTimingsNV || !vkSetLatencyMarkerNV || !vkQueueNotifyOutOfBandNV) {
            // VK_NV_low_latency2 was not requested -> our Vulkan Reflex layer is not present -> grab vkSignalSemaphore to fake that reflex is working
            //
            // The app should have requested either the Vulkan 1.2 or the VK_KHR_timeline_semaphore extension
            // We'll query whichever is available
            if (!vkSignalSemaphore) {
                VK_GET_DEVICE_PROC_ADDR(vkSignalSemaphoreKHR);
                vkSignalSemaphore = vkSignalSemaphoreKHR;
            }

            if (!vkSignalSemaphore)
                return {nullptr, VK_ERROR_EXTENSION_NOT_PRESENT};
        } else {
            vkSignalSemaphore = nullptr; // We don't need this, we're signalling with vkLatencySleepNV
        }

        if (!vkCreateSemaphore || !vkDestroySemaphore)
            return {nullptr, VK_ERROR_INCOMPATIBLE_DRIVER};

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

        if (vr != VK_SUCCESS)
            return {nullptr, vr};

        auto [it, inserted] = m_nvapiDeviceMap.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(device),
            std::forward_as_tuple(
                device,
                semaphore,
                vkDestroySemaphore,
                vkSetLatencySleepModeNV,
                vkLatencySleepNV,
                vkGetLatencyTimingsNV,
                vkSetLatencyMarkerNV,
                vkQueueNotifyOutOfBandNV,
                vkSignalSemaphore));

        if (!inserted)
            return {nullptr, VK_ERROR_UNKNOWN};

        return {&it->second, vr};
    }

    NvapiVulkanLowLatencyDevice* NvapiVulkanLowLatencyDevice::Get(VkDevice device) {
        auto it = m_nvapiDeviceMap.find(device);
        return it == m_nvapiDeviceMap.end() ? nullptr : &it->second;
    }

    bool NvapiVulkanLowLatencyDevice::Destroy(VkDevice device) {
        std::scoped_lock lock{m_mutex};

        auto node = m_nvapiDeviceMap.extract(device);

        if (node.empty())
            return false;

        auto& lowLatencyDevice = node.mapped();

        lowLatencyDevice.m_vkDestroySemaphore(lowLatencyDevice.m_device, lowLatencyDevice.m_semaphore, nullptr);

        return true;
    }

    VkLatencyMarkerNV NvapiVulkanLowLatencyDevice::ToVkLatencyMarkerNV(NV_VULKAN_LATENCY_MARKER_TYPE marker) {
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

#define PFN_PARAM(proc) PFN_##proc proc
#define PFN_INIT(proc) m_##proc(proc)
    NvapiVulkanLowLatencyDevice::NvapiVulkanLowLatencyDevice(
        VkDevice device,
        VkSemaphore semaphore,
        PFN_PARAM(vkDestroySemaphore),
        PFN_PARAM(vkSetLatencySleepModeNV),
        PFN_PARAM(vkLatencySleepNV),
        PFN_PARAM(vkGetLatencyTimingsNV),
        PFN_PARAM(vkSetLatencyMarkerNV),
        PFN_PARAM(vkQueueNotifyOutOfBandNV),
        PFN_PARAM(vkSignalSemaphore))
        : m_device(device),
          m_semaphore(semaphore),
          // If the Vulkan Reflex Layer is present it will enable VK_NV_low_latency2 which will let us query
          // these function pointers.
          // Without the layer we'll pretend that Reflex is happening so that apps don't get a pink tint.
          m_layerPresent(vkSetLatencySleepModeNV && vkLatencySleepNV && vkGetLatencyTimingsNV
              && vkSetLatencyMarkerNV && vkQueueNotifyOutOfBandNV),
          PFN_INIT(vkDestroySemaphore),
          PFN_INIT(vkSetLatencySleepModeNV),
          PFN_INIT(vkLatencySleepNV),
          PFN_INIT(vkGetLatencyTimingsNV),
          PFN_INIT(vkSetLatencyMarkerNV),
          PFN_INIT(vkQueueNotifyOutOfBandNV),
          PFN_INIT(vkSignalSemaphore) {}

    bool NvapiVulkanLowLatencyDevice::IsLayerPresent() const {
        return m_layerPresent;
    }

    VkSemaphore NvapiVulkanLowLatencyDevice::GetSemaphore() const {
        return m_semaphore;
    }

    bool NvapiVulkanLowLatencyDevice::GetLowLatencyMode() const {
        return m_lowLatencyMode;
    }

    static inline VkSwapchainKHR GetSwapchain(VkDevice device) {
        // winevulkan expects valid Vulkan usage so it never checks if swapchains passed to VK_NV_low_latency2 functions
        // are null handles or not, it just unconditionally dereferences them, and we just need our calls to make it
        // to the Linux-side Vulkan layer without crashing in the meantime, this hack makes winevulkan happy enough
        return reinterpret_cast<VkSwapchainKHR>(device);
    }

    VkResult NvapiVulkanLowLatencyDevice::SetLatencySleepMode(std::nullptr_t) {
        if (!m_layerPresent)
            return VK_SUCCESS;

        auto vr = m_vkSetLatencySleepModeNV(m_device, GetSwapchain(m_device), nullptr);

        if (vr == VK_SUCCESS)
            m_lowLatencyMode = false;

        return vr;
    }

    VkResult NvapiVulkanLowLatencyDevice::SetLatencySleepMode(bool lowLatencyMode, bool lowLatencyBoost, uint32_t minimumIntervalUs) {
        if (!m_layerPresent)
            return VK_SUCCESS;

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

    VkResult NvapiVulkanLowLatencyDevice::LatencySleep(uint64_t value) {
        if (m_layerPresent) {
            auto info = VkLatencySleepInfoNV{
                .sType = VK_STRUCTURE_TYPE_LATENCY_SLEEP_INFO_NV,
                .pNext = nullptr,
                .signalSemaphore = m_semaphore,
                .value = value,
            };

            return m_vkLatencySleepNV(m_device, GetSwapchain(m_device), &info);
        } else {
            auto info = VkSemaphoreSignalInfoKHR{
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO_KHR,
                .pNext = nullptr,
                .semaphore = m_semaphore,
                .value = value};

            return m_vkSignalSemaphore(m_device, &info);
        }
    }

    bool NvapiVulkanLowLatencyDevice::GetLatencyTimings(std::array<VkLatencyTimingsFrameReportNV, 64>& timings) {
        if (!m_layerPresent)
            return false;

        for (auto& timing : timings) {
            timing.sType = VK_STRUCTURE_TYPE_LATENCY_TIMINGS_FRAME_REPORT_NV;
            timing.pNext = nullptr;
        }

        auto info = VkGetLatencyMarkerInfoNV{
            .sType = VK_STRUCTURE_TYPE_GET_LATENCY_MARKER_INFO_NV,
            .pNext = nullptr,
            .timingCount = 64,
            .pTimings = timings.data(),
        };

        m_vkGetLatencyTimingsNV(m_device, GetSwapchain(m_device), &info);

        return info.timingCount == 64;
    }

    void NvapiVulkanLowLatencyDevice::SetLatencyMarker(uint64_t presentID, VkLatencyMarkerNV marker) {
        if (!m_layerPresent)
            return;

        auto info = VkSetLatencyMarkerInfoNV{
            .sType = VK_STRUCTURE_TYPE_SET_LATENCY_MARKER_INFO_NV,
            .pNext = nullptr,
            .presentID = presentID,
            .marker = marker,
        };

        m_vkSetLatencyMarkerNV(m_device, GetSwapchain(m_device), &info);
    }

    void NvapiVulkanLowLatencyDevice::QueueNotifyOutOfBand(VkQueue queue, VkOutOfBandQueueTypeNV queueType) {
        if (!m_layerPresent)
            return;

        auto info = VkOutOfBandQueueTypeInfoNV{
            .sType = VK_STRUCTURE_TYPE_OUT_OF_BAND_QUEUE_TYPE_INFO_NV,
            .pNext = nullptr,
            .queueType = queueType,
        };

        m_vkQueueNotifyOutOfBandNV(queue, &info);
    }
}
