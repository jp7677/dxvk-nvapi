#pragma once

#include "../nvapi_private.h"
#include "../util/com_pointer.h"
#include "vulkan.h"
#include "nvml.h"
#include "nvapi_output.h"

namespace dxvk {
    class NvapiAdapter {

    public:
        NvapiAdapter(Vulkan& vulkan, Nvml& nvml);
        ~NvapiAdapter();

        bool Initialize(Com<IDXGIAdapter>& dxgiAdapter, std::vector<NvapiOutput*>& outputs);
        [[nodiscard]] std::string GetDeviceName() const;
        [[nodiscard]] VkDriverIdKHR GetDriverId() const;
        [[nodiscard]] uint32_t GetDriverVersion() const;
        [[nodiscard]] uint32_t GetDeviceId() const;
        [[nodiscard]] uint32_t GetGpuType() const;
        [[nodiscard]] uint32_t GetBusId() const;
        [[nodiscard]] uint32_t GetVRamSize() const;
        [[nodiscard]] bool GetLUID(LUID* luid) const;
        [[nodiscard]] NV_GPU_ARCHITECTURE_ID GetArchitectureId() const;

        [[nodiscard]] bool HasNvml() const;
        [[nodiscard]] bool HasNvmlDevice() const;
        [[nodiscard]] std::string NvmlErrorString(nvmlReturn_t result) const;
        [[nodiscard]] nvmlReturn_t NvmlDeviceGetTemperature(nvmlTemperatureSensors_t sensorType, unsigned int* temp) const;
        [[nodiscard]] nvmlReturn_t NvmlDeviceGetUtilizationRates(nvmlUtilization_t* utilization) const;

    private:
        Vulkan& m_vulkan;
        Nvml& m_nvml;

        VkPhysicalDeviceProperties m_deviceProperties{};
        VkPhysicalDeviceIDProperties m_deviceIdProperties{};
        VkPhysicalDevicePCIBusInfoPropertiesEXT m_devicePciBusProperties{};
        VkPhysicalDeviceMemoryProperties m_memoryProperties{};
        VkPhysicalDeviceDriverPropertiesKHR m_deviceDriverProperties{};
        VkPhysicalDeviceFragmentShadingRatePropertiesKHR m_deviceFragmentShadingRateProperties{};
        uint32_t m_vkDriverVersion{};
        std::set<std::string> m_deviceExtensions{};

        nvmlDevice_t m_nvmlDevice{};

        [[nodiscard]] bool isVkDeviceExtensionSupported(std::string name) const;
    };
}
