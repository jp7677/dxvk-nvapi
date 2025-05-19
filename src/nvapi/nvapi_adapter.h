#pragma once

#include "../nvapi_private.h"
#include "../util/com_pointer.h"
#include "../shared/vk.h"
#include "nvml.h"
#include "nvapi_output.h"

namespace dxvk {
    class NvapiAdapter {

      public:
        explicit NvapiAdapter(Vk& vk, Nvml& nvml, Com<IDXGIAdapter3> dxgiAdapter);
        ~NvapiAdapter();

        struct MemoryInfo {
            uint64_t DedicatedVideoMemory;
            uint64_t ReservedVideoMemory;
            uint64_t DedicatedSystemMemory;
            uint64_t SharedSystemMemory;
        };
        struct MemoryBudgetInfo {
            uint64_t Budget;
            uint64_t CurrentUsage;
        };

        bool Initialize(uint32_t index, std::vector<NvapiOutput*>& outputs);
        [[nodiscard]] std::string GetDeviceName() const;
        [[nodiscard]] bool HasNvProprietaryDriver() const;
        [[nodiscard]] bool HasNvkDriver() const;
        [[nodiscard]] uint32_t GetDriverVersion() const;
        [[nodiscard]] uint32_t GetDeviceId() const;
        [[nodiscard]] uint32_t GetExternalDeviceId() const;
        [[nodiscard]] uint32_t GetSubSystemId() const;
        [[nodiscard]] NV_GPU_TYPE GetGpuType() const;
        [[nodiscard]] uint32_t GetPciBusId() const;
        [[nodiscard]] uint32_t GetPciDeviceId() const;
        [[nodiscard]] uint32_t GetBoardId() const;
        [[nodiscard]] std::optional<LUID> GetLuid() const;
        [[nodiscard]] NV_GPU_ARCHITECTURE_ID GetArchitectureId() const;
        [[nodiscard]] bool IsVkDeviceExtensionSupported(const std::string& name) const;
        [[nodiscard]] const MemoryInfo& GetMemoryInfo() const;
        [[nodiscard]] MemoryBudgetInfo GetCurrentMemoryBudgetInfo() const;
        [[nodiscard]] Nvml* GetNvml() const;
        [[nodiscard]] nvmlDevice_t GetNvmlDevice() const;

      private:
        Vk& m_vk;
        Nvml& m_nvml;
        Com<IDXGIAdapter3> m_dxgiAdapter;

        std::set<std::string> m_vkExtensions;
        VkPhysicalDeviceProperties m_vkProperties{};
        VkPhysicalDeviceIDProperties m_vkIdProperties{};
        VkPhysicalDevicePCIBusInfoPropertiesEXT m_vkPciBusProperties{};
        VkPhysicalDeviceDriverPropertiesKHR m_vkDriverProperties{};
        VkPhysicalDeviceFragmentShadingRatePropertiesKHR m_vkFragmentShadingRateProperties{};
        VkPhysicalDeviceComputeShaderDerivativesPropertiesKHR m_vkComputeShaderDerivativesProperties{};
        uint32_t m_vkDriverVersion{};
        uint32_t m_dxgiVendorId{};
        uint32_t m_dxgiDeviceId{};
        MemoryInfo m_memoryInfo{};

        nvmlDevice_t m_nvmlDevice{};

        uint32_t m_driverVersionOverride = 0;

        constexpr static auto driverVersionEnvName = "DXVK_NVAPI_DRIVER_VERSION";
        constexpr static auto allowOtherDriversEnvName = "DXVK_NVAPI_ALLOW_OTHER_DRIVERS";
        constexpr static uint16_t NvidiaPciVendorId = 0x10de;
    };
}
