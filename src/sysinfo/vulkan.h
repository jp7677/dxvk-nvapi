#pragma once

#include "../nvapi_private.h"

namespace dxvk {
    class Vulkan {

    public:
        Vulkan();
        ~Vulkan();

        bool IsLoaded();
        std::set<std::string> GetDeviceExtensions(VkInstance vkInstance, VkPhysicalDevice vkDevice);
        void vkGetPhysicalDeviceProperties2(VkInstance vkInstance, VkPhysicalDevice vkDevice, VkPhysicalDeviceProperties2* deviceProperties2);
        void vkGetPhysicalDeviceMemoryProperties2(VkInstance vkInstance, VkPhysicalDevice vkDevice, VkPhysicalDeviceMemoryProperties2* memoryProperties2);

    private:
        HMODULE m_vkModule{};
        PFN_vkGetInstanceProcAddr m_vkGetInstanceProcAddr{};
    };
}
