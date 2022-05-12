#include "vulkan.h"
#include "../util/util_string.h"
#include "../util/util_log.h"

namespace dxvk {
    Vulkan::Vulkan() {
        const auto vkModuleName = "vulkan-1.dll";
        m_vkModule = ::LoadLibraryA(vkModuleName);
        if (m_vkModule == nullptr) {
            log::write(str::format("Loading ", vkModuleName, " failed with error code: ", ::GetLastError()));
            return;
        }

        m_vkGetInstanceProcAddr = GetProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
    }

    Vulkan::~Vulkan() {
        if (m_vkModule == nullptr)
            return;

        ::FreeLibrary(m_vkModule);
        m_vkModule = nullptr;
    }

    bool Vulkan::IsAvailable() const {
        return m_vkModule != nullptr;
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

    void Vulkan::GetPhysicalDeviceMemoryProperties2(VkInstance vkInstance, VkPhysicalDevice vkDevice, VkPhysicalDeviceMemoryProperties2* memoryProperties2) const {
        auto vkGetPhysicalDeviceMemoryProperties2 =
            GetInstanceProcAddress<PFN_vkGetPhysicalDeviceMemoryProperties2>(
                vkInstance, "vkGetPhysicalDeviceMemoryProperties2");

        vkGetPhysicalDeviceMemoryProperties2(vkDevice, memoryProperties2);
    }

    template <typename T>
    T Vulkan::GetProcAddress(const char* name) const {
        return reinterpret_cast<T>(reinterpret_cast<void*>(::GetProcAddress(m_vkModule, name)));
    }

    template <typename T>
    T Vulkan::GetInstanceProcAddress(VkInstance vkInstance, const char* name) const {
        return reinterpret_cast<T>(m_vkGetInstanceProcAddr(vkInstance, name));
    }
}