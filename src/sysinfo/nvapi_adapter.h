#pragma once

#include "../nvapi_private.h"
#include "../dxvk/dxvk_interfaces.h"
#include "../util/com_pointer.h"

#include "nvapi_output.h"

namespace dxvk {

    class NvapiAdapter {

    public:

        NvapiAdapter();
        ~NvapiAdapter();

        bool Initialize(Com<IDXGIAdapter>& dxgiAdapter, std::vector<NvapiOutput*>& outputs);

        [[nodiscard]] std::string GetDeviceName() const;
        [[nodiscard]] u_int GetDriverVersion() const;
        [[nodiscard]] u_int GetDeviceId() const;
        [[nodiscard]] u_int GetGpuType() const;
        [[nodiscard]] u_int GetBusId() const;
        [[nodiscard]] u_int GetVRamSize() const;

    private:

        VkPhysicalDevice m_vkDevice = VK_NULL_HANDLE;
        VkPhysicalDeviceProperties m_deviceProperties{};
        VkPhysicalDevicePCIBusInfoPropertiesEXT m_devicePciBusProperties{};
        VkPhysicalDeviceMemoryProperties m_memoryProperties{};
        u_int m_vkDriverVersion{};

    };
}
