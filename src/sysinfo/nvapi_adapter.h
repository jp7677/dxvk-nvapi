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
        [[nodiscard]] std::string GetNvmlErrorString(nvmlReturn_t result) const;
        [[nodiscard]] nvmlReturn_t GetNvmlDeviceTemperature(nvmlTemperatureSensors_t sensorType, unsigned int* temp) const;
        [[nodiscard]] nvmlReturn_t GetNvmlDeviceUtilizationRates(nvmlUtilization_t* utilization) const;
        [[nodiscard]] nvmlReturn_t GetNvmlDeviceVbiosVersion(char* version, unsigned int length) const;
        [[nodiscard]] nvmlReturn_t GetNvmlPerformanceState(nvmlPstates_t* pState) const;
        [[nodiscard]] nvmlReturn_t GetNvmlDeviceClockInfo(nvmlClockType_t type, unsigned int* clock) const;

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

        uint32_t m_driverVersionOverride{};

        [[nodiscard]] bool IsVkDeviceExtensionSupported(std::string name) const;
    };
}
