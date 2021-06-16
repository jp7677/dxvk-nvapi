#pragma once

#include "../nvapi_private.h"

namespace dxvk {
    class Vulkan final {

    public:
        Vulkan();
        ~Vulkan();

        [[nodiscard]] bool IsAvailable();
        [[nodiscard]] std::set<std::string> GetDeviceExtensions(VkInstance vkInstance, VkPhysicalDevice vkDevice);
        void GetPhysicalDeviceProperties2(VkInstance vkInstance, VkPhysicalDevice vkDevice, VkPhysicalDeviceProperties2* deviceProperties2);
        void GetPhysicalDeviceMemoryProperties2(VkInstance vkInstance, VkPhysicalDevice vkDevice, VkPhysicalDeviceMemoryProperties2* memoryProperties2);

    private:
        HMODULE m_vkModule{};
        PFN_vkGetInstanceProcAddr m_vkGetInstanceProcAddr{};

        template<typename T> T GetProcAddress(const char* name);
        template<typename T> T GetInstanceProcAddress(VkInstance vkInstance, const char* name);
    };
}
