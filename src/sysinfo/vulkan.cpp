#include "vulkan.h"

namespace dxvk {
    Vulkan::Vulkan() = default;
    Vulkan::Vulkan(Com<IDXGIFactory1> dxgiFactory, PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr)
        : m_dxgiFactory(std::move(dxgiFactory)), m_vkGetInstanceProcAddr(vkGetInstanceProcAddr) {}

    Vulkan::~Vulkan() = default;

    bool Vulkan::IsAvailable() const {
        return m_vkGetInstanceProcAddr != nullptr;
    }

    std::set<std::string> Vulkan::GetDeviceExtensions(VkInstance vkInstance, VkPhysicalDevice vkDevice) const {
        auto vkEnumerateDeviceExtensionProperties =
            GetInstanceProcAddress<PFN_vkEnumerateDeviceExtensionProperties>(
                vkInstance, "vkEnumerateDeviceExtensionProperties");

        std::set<std::string> deviceExtensions;

        // Grab last of valid extensions for this device
        auto count = 0U;
        if (vkEnumerateDeviceExtensionProperties(vkDevice, nullptr, &count, nullptr) != VK_SUCCESS)
            return deviceExtensions;

        std::vector<VkExtensionProperties> extensions(count);
        if (vkEnumerateDeviceExtensionProperties(vkDevice, nullptr, &count, extensions.data()) != VK_SUCCESS)
            return deviceExtensions;

        for (const auto& extension : extensions)
            deviceExtensions.insert(std::string(extension.extensionName));

        return deviceExtensions;
    }

    void Vulkan::GetPhysicalDeviceProperties2(VkInstance vkInstance, VkPhysicalDevice vkDevice, VkPhysicalDeviceProperties2* deviceProperties2) const {
        auto vkGetPhysicalDeviceProperties2 =
            GetInstanceProcAddress<PFN_vkGetPhysicalDeviceProperties2>(
                vkInstance, "vkGetPhysicalDeviceProperties2");

        vkGetPhysicalDeviceProperties2(vkDevice, deviceProperties2);
    }

    void Vulkan::GetPhysicalDeviceFeatures2(VkInstance vkInstance, VkPhysicalDevice vkDevice, VkPhysicalDeviceFeatures2* deviceFeatures2) const {
        auto vkGetPhysicalDeviceFeatures2 =
            GetInstanceProcAddress<PFN_vkGetPhysicalDeviceFeatures2>(
                vkInstance, "vkGetPhysicalDeviceFeatures2");

        vkGetPhysicalDeviceFeatures2(vkDevice, deviceFeatures2);
    }

    NV_GPU_TYPE Vulkan::ToNvGpuType(VkPhysicalDeviceType vkDeviceType) {
        // Assert enum value equality between Vulkan and NVAPI at compile time
        static_assert(static_cast<int>(VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) == static_cast<int>(NV_SYSTEM_TYPE_DGPU));
        static_assert(static_cast<int>(VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) == static_cast<int>(NV_SYSTEM_TYPE_IGPU));
        static_assert(static_cast<int>(VK_PHYSICAL_DEVICE_TYPE_OTHER) == static_cast<int>(NV_SYSTEM_TYPE_GPU_UNKNOWN));

        return vkDeviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU || vkDeviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU
            ? static_cast<NV_GPU_TYPE>(vkDeviceType)
            : NV_SYSTEM_TYPE_GPU_UNKNOWN;
    }

    template <typename T>
    T Vulkan::GetInstanceProcAddress(VkInstance vkInstance, const char* name) const {
        return reinterpret_cast<T>(m_vkGetInstanceProcAddr(vkInstance, name));
    }
}