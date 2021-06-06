#include "vulkan.h"

#include "../util/util_string.h"
#include "../util/util_log.h"

namespace dxvk {
    Vulkan::Vulkan() {
        const auto vkModuleName = "vulkan-1.dll";
        m_vkModule = ::LoadLibraryA(vkModuleName);
        if (m_vkModule == nullptr) {
            log::write(str::format("Loading ", vkModuleName, " failed with error code ", ::GetLastError()));
            return;
        }

        m_vkGetInstanceProcAddr =
            reinterpret_cast<PFN_vkGetInstanceProcAddr>(
                reinterpret_cast<void*>(
                    ::GetProcAddress(m_vkModule, "vkGetInstanceProcAddr")));
    }

    Vulkan::~Vulkan() {
        if (m_vkModule == nullptr)
            return;

        ::FreeLibrary(m_vkModule);
        m_vkModule = nullptr;
    }

    bool Vulkan::IsLoaded() {
        return m_vkModule != nullptr;
    }

    std::set<std::string> Vulkan::GetDeviceExtensions(VkInstance vkInstance, VkPhysicalDevice vkDevice) {
        auto vkEnumerateDeviceExtensionProperties =
            reinterpret_cast<PFN_vkEnumerateDeviceExtensionProperties>(
                m_vkGetInstanceProcAddr(vkInstance, "vkEnumerateDeviceExtensionProperties"));

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

    void Vulkan::vkGetPhysicalDeviceProperties2(VkInstance vkInstance, VkPhysicalDevice vkDevice, VkPhysicalDeviceProperties2* deviceProperties2) {
        auto vkGetPhysicalDeviceProperties2 =
            reinterpret_cast<PFN_vkGetPhysicalDeviceProperties2>(
                m_vkGetInstanceProcAddr(vkInstance, "vkGetPhysicalDeviceProperties2"));

        vkGetPhysicalDeviceProperties2(vkDevice, deviceProperties2);
    }

    void Vulkan::vkGetPhysicalDeviceMemoryProperties2(VkInstance vkInstance, VkPhysicalDevice vkDevice, VkPhysicalDeviceMemoryProperties2* memoryProperties2) {
        auto vkGetPhysicalDeviceMemoryProperties2 =
            reinterpret_cast<PFN_vkGetPhysicalDeviceMemoryProperties2>(
                m_vkGetInstanceProcAddr(vkInstance, "vkGetPhysicalDeviceMemoryProperties2"));

        vkGetPhysicalDeviceMemoryProperties2(vkDevice, memoryProperties2);
    }
}