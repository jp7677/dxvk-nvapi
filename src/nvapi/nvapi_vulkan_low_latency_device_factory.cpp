#include "./nvapi_vulkan_low_latency_device_factory.h"
#include "../util/util_env.h"
#include "../util/util_log.h"

namespace dxvk {
    std::unique_ptr<Vk> NvapiVulkanLowLatencyDeviceFactory::m_vk = nullptr;
    std::unordered_map<VkDevice, std::unique_ptr<NvapiVulkanLowLatencyDevice>> NvapiVulkanLowLatencyDeviceFactory::m_nvapiDeviceMap = {};
    std::mutex NvapiVulkanLowLatencyDeviceFactory::m_mutex = {};

    bool NvapiVulkanLowLatencyDeviceFactory::Initialize(NvapiResourceFactory& resourceFactory) {
        std::scoped_lock lock{m_mutex};

        if (m_vk && m_vk->IsAvailable())
            return true;

        m_vk = resourceFactory.CreateVulkan("vulkan-1.dll");

        if (m_vk && m_vk->IsAvailable())
            return true;

        m_vk = resourceFactory.CreateVulkan("winevulkan.dll");

        return m_vk && m_vk->IsAvailable();
    }

    void NvapiVulkanLowLatencyDeviceFactory::Reset() {
        std::scoped_lock lock{m_mutex};

        m_nvapiDeviceMap.clear();
        m_vk.reset();
    }

    std::pair<NvapiVulkanLowLatencyDevice*, VkResult> NvapiVulkanLowLatencyDeviceFactory::GetOrCreate(VkDevice device) {
        std::scoped_lock lock{m_mutex};

        if (auto lowLatencyDevice = Get(device))
            return {lowLatencyDevice, VK_SUCCESS};

        if (!m_vk || !m_vk->IsAvailable()) {
            log::info("Initializing Vulkan Low-Latency failed: could not find usable Vulkan loader (or winevulkan) module in the current process");
            return {nullptr, VK_ERROR_INITIALIZATION_FAILED};
        }

        std::unique_ptr<NvapiVulkanLowLatencyDevice> lowLatencyDevice;
        VkResult vr;

        std::tie(lowLatencyDevice, vr) = NvapiVulkanLowLatencyLegacyDevice::TryCreate(m_vk.get(), device);

        if (!lowLatencyDevice || vr != VK_SUCCESS)
            std::tie(lowLatencyDevice, vr) = NvapiVulkanLowLatency2LayerDevice::TryCreate(m_vk.get(), device);

        if (!lowLatencyDevice || vr != VK_SUCCESS)
            std::tie(lowLatencyDevice, vr) = NvapiVulkanLowLatencyFakeDevice::TryCreate(m_vk.get(), device);

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

    NvapiVulkanLowLatencyDevice* NvapiVulkanLowLatencyDeviceFactory::Get(VkDevice device) {
        auto it = m_nvapiDeviceMap.find(device);
        return it == m_nvapiDeviceMap.end() ? nullptr : it->second.get();
    }

    bool NvapiVulkanLowLatencyDeviceFactory::Destroy(VkDevice device) {
        std::scoped_lock lock{m_mutex};

        auto node = m_nvapiDeviceMap.extract(device);

        if (node.empty())
            return false;

        auto lowLatencyDevice = std::move(node.mapped());
        // the destructor will take care of semaphore cleanup

        return true;
    }
}
