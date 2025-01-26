#include "vk.h"
#include "../util/util_log.h"

namespace dxvk {
    Vk::Vk() = default;

    Vk::Vk(Com<IDXGIVkInteropFactory>&& dxgiVkInteropFactory)
        : m_dxgiVkInteropFactory(dxgiVkInteropFactory) {
        VkInstance vkInstance;
        m_dxgiVkInteropFactory->GetVulkanInstance(&vkInstance, &m_vkGetInstanceProcAddr);

        if (vkInstance && m_vkGetInstanceProcAddr) {
            log::info(str::format("Successfully acquired Vulkan vkGetInstanceProcAddr @ 0x", std::hex, reinterpret_cast<uintptr_t>(m_vkGetInstanceProcAddr)));

            m_vkGetDeviceProcAddr = GetInstanceProcAddress<PFN_vkGetDeviceProcAddr>(vkInstance, "vkGetDeviceProcAddr");
        }
    }

    Vk::Vk(PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr, PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr)
        : m_vkGetInstanceProcAddr(vkGetInstanceProcAddr), m_vkGetDeviceProcAddr(vkGetDeviceProcAddr) {}

    Vk::~Vk() = default;

    bool Vk::IsAvailable() const {
        return m_vkGetInstanceProcAddr != nullptr && m_vkGetDeviceProcAddr != nullptr;
    }

    PFN_vkVoidFunction Vk::GetInstanceProcAddr(VkInstance vkInstance, const char* name) const {
        return GetInstanceProcAddress<PFN_vkVoidFunction>(vkInstance, name);
    }

    PFN_vkVoidFunction Vk::GetDeviceProcAddr(VkDevice vkDevice, const char* name) const {
        return m_vkGetDeviceProcAddr(vkDevice, name);
    }

    std::set<std::string> Vk::GetDeviceExtensions(VkInstance vkInstance, VkPhysicalDevice vkDevice) const {
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

    void Vk::GetPhysicalDeviceProperties2(VkInstance vkInstance, VkPhysicalDevice vkDevice, VkPhysicalDeviceProperties2* deviceProperties2) const {
        auto vkGetPhysicalDeviceProperties2 =
            GetInstanceProcAddress<PFN_vkGetPhysicalDeviceProperties2>(
                vkInstance, "vkGetPhysicalDeviceProperties2");

        vkGetPhysicalDeviceProperties2(vkDevice, deviceProperties2);
    }

    NV_GPU_TYPE Vk::ToNvGpuType(VkPhysicalDeviceType vkDeviceType) {
        // Assert enum value equality between Vulkan and NVAPI at compile time
        static_assert(static_cast<int>(VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) == static_cast<int>(NV_SYSTEM_TYPE_DGPU));
        static_assert(static_cast<int>(VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) == static_cast<int>(NV_SYSTEM_TYPE_IGPU));
        static_assert(static_cast<int>(VK_PHYSICAL_DEVICE_TYPE_OTHER) == static_cast<int>(NV_SYSTEM_TYPE_GPU_UNKNOWN));

        return vkDeviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU || vkDeviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU
            ? static_cast<NV_GPU_TYPE>(vkDeviceType)
            : NV_SYSTEM_TYPE_GPU_UNKNOWN;
    }

    template <typename T>
    T Vk::GetInstanceProcAddress(VkInstance vkInstance, const char* name) const {
        return reinterpret_cast<T>(m_vkGetInstanceProcAddr(vkInstance, name));
    }
}