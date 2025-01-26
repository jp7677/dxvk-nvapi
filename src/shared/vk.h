#pragma once

#include "../nvapi_private.h"
#include "../interfaces/dxvk_interfaces.h"
#include "../util/com_pointer.h"

namespace dxvk {
    class Vk {

      public:
        Vk();
        explicit Vk(Com<IDXGIVkInteropFactory>&& dxgiVkInteropFactory);
        explicit Vk(PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr, PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr);
        virtual ~Vk();

        [[nodiscard]] virtual bool IsAvailable() const;
        [[nodiscard]] virtual PFN_vkVoidFunction GetInstanceProcAddr(VkInstance vkInstance, const char* name) const;
        [[nodiscard]] virtual PFN_vkVoidFunction GetDeviceProcAddr(VkDevice vkDevice, const char* name) const;
        [[nodiscard]] virtual std::set<std::string> GetDeviceExtensions(VkInstance vkInstance, VkPhysicalDevice vkDevice) const;
        virtual void GetPhysicalDeviceProperties2(VkInstance vkInstance, VkPhysicalDevice vkDevice, VkPhysicalDeviceProperties2* deviceProperties2) const;

        [[nodiscard]] static NV_GPU_TYPE ToNvGpuType(VkPhysicalDeviceType vkDeviceType);

      private:
        Com<IDXGIVkInteropFactory> m_dxgiVkInteropFactory;
        PFN_vkGetInstanceProcAddr m_vkGetInstanceProcAddr{};
        PFN_vkGetDeviceProcAddr m_vkGetDeviceProcAddr{};

        template <typename T>
        T GetInstanceProcAddress(VkInstance vkInstance, const char* name) const;
    };
}
