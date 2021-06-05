#pragma once

#include "../nvapi_private.h"
#include "../util/com_pointer.h"
#include "nvapi_output.h"

#include <set>

namespace dxvk {
    class NvapiAdapter {

    public:
        NvapiAdapter();
        ~NvapiAdapter();

        bool Initialize(Com<IDXGIAdapter>& dxgiAdapter, std::vector<NvapiOutput*>& outputs, HMODULE vkModule);
        [[nodiscard]] std::string GetDeviceName() const;
        [[nodiscard]] VkDriverIdKHR GetDriverId() const;
        [[nodiscard]] uint32_t GetDriverVersion() const;
        [[nodiscard]] uint32_t GetDeviceId() const;
        [[nodiscard]] uint32_t GetGpuType() const;
        [[nodiscard]] uint32_t GetBusId() const;
        [[nodiscard]] uint32_t GetVRamSize() const;
        [[nodiscard]] bool GetLUID(LUID* luid) const;
        [[nodiscard]] NV_GPU_ARCHITECTURE_ID GetArchitectureId() const;

    private:
        [[nodiscard]] bool isVkDeviceExtensionSupported(std::string name) const;

        VkPhysicalDeviceProperties m_deviceProperties{};
        VkPhysicalDeviceIDProperties m_deviceIdProperties{};
        VkPhysicalDevicePCIBusInfoPropertiesEXT m_devicePciBusProperties{};
        VkPhysicalDeviceMemoryProperties m_memoryProperties{};
        VkPhysicalDeviceDriverPropertiesKHR m_deviceDriverProperties{};
        VkPhysicalDeviceFragmentShadingRatePropertiesKHR m_deviceFragmentShadingRateProperties{};
        uint32_t m_vkDriverVersion{};
        std::set<std::string> m_deviceExtensions{};
    };
}
