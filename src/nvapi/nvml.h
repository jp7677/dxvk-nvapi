#pragma once

#include "../nvapi_private.h"

namespace dxvk {
    class Nvml {

      public:
        Nvml();
        virtual ~Nvml();

        [[nodiscard]] virtual bool IsAvailable() const;
        [[nodiscard]] virtual const char* ErrorString(nvmlReturn_t result) const;
        [[nodiscard]] virtual nvmlReturn_t DeviceGetHandleByPciBusId_v2(const char* pciBusId, nvmlDevice_t* device) const;
        [[nodiscard]] virtual nvmlReturn_t DeviceGetMemoryInfo_v2(nvmlDevice_t device, nvmlMemory_v2_t* memory) const;
        [[nodiscard]] virtual nvmlReturn_t DeviceGetPciInfo_v3(nvmlDevice_t device, nvmlPciInfo_t* pci) const;
        [[nodiscard]] virtual nvmlReturn_t DeviceGetClockInfo(nvmlDevice_t device, nvmlClockType_t type, unsigned int* clock) const;
        [[nodiscard]] virtual nvmlReturn_t DeviceGetMaxClockInfo(nvmlDevice_t device, nvmlClockType_t type, unsigned int* clock) const;
        [[nodiscard]] virtual nvmlReturn_t DeviceGetTemperatureV(nvmlDevice_t device, nvmlTemperature_t* temperature) const;
        [[nodiscard]] virtual nvmlReturn_t DeviceGetThermalSettings(nvmlDevice_t device, unsigned int sensorIndex, nvmlGpuThermalSettings_t* pThermalSettings) const;
        [[nodiscard]] virtual nvmlReturn_t DeviceGetFanSpeedRPM(nvmlDevice_t device, nvmlFanSpeedInfo_t* fanSpeed) const;
        [[nodiscard]] virtual nvmlReturn_t DeviceGetPerformanceState(nvmlDevice_t device, nvmlPstates_t* pState) const;
        [[nodiscard]] virtual nvmlReturn_t DeviceGetUtilizationRates(nvmlDevice_t device, nvmlUtilization_t* utilization) const;
        [[nodiscard]] virtual nvmlReturn_t DeviceGetVbiosVersion(nvmlDevice_t device, char* version, unsigned int length) const;
        [[nodiscard]] virtual nvmlReturn_t DeviceGetCurrPcieLinkWidth(nvmlDevice_t device, unsigned int* width) const;
        [[nodiscard]] virtual nvmlReturn_t DeviceGetIrqNum(nvmlDevice_t device, unsigned int* irqNum) const;
        [[nodiscard]] virtual nvmlReturn_t DeviceGetNumGpuCores(nvmlDevice_t device, unsigned int* numCores) const;
        [[nodiscard]] virtual nvmlReturn_t DeviceGetBusType(nvmlDevice_t device, nvmlBusType_t* type) const;
        [[nodiscard]] virtual nvmlReturn_t DeviceGetDynamicPstatesInfo(nvmlDevice_t device, nvmlGpuDynamicPstatesInfo_t* pDynamicPstatesInfo) const;

        [[nodiscard]] static NV_THERMAL_TARGET ToNvThermalTarget(nvmlThermalTarget_t target);
        [[nodiscard]] static NV_THERMAL_CONTROLLER ToNvThermalController(nvmlThermalController_t controller);
        [[nodiscard]] static NV_GPU_BUS_TYPE ToNvGpuBusType(nvmlBusType_t type);

      private:
        HMODULE m_nvmlModule{};

#define DECLARE_PFN(x) \
    decltype(&x) m_##x {}

        DECLARE_PFN(nvmlInit_v2);
        DECLARE_PFN(nvmlShutdown);
        DECLARE_PFN(nvmlErrorString);
        DECLARE_PFN(nvmlDeviceGetHandleByPciBusId_v2);
        DECLARE_PFN(nvmlDeviceGetMemoryInfo_v2);
        DECLARE_PFN(nvmlDeviceGetPciInfo_v3);
        DECLARE_PFN(nvmlDeviceGetClockInfo);
        DECLARE_PFN(nvmlDeviceGetMaxClockInfo);
        DECLARE_PFN(nvmlDeviceGetTemperatureV);
        DECLARE_PFN(nvmlDeviceGetThermalSettings);
        DECLARE_PFN(nvmlDeviceGetFanSpeedRPM);
        DECLARE_PFN(nvmlDeviceGetPerformanceState);
        DECLARE_PFN(nvmlDeviceGetUtilizationRates);
        DECLARE_PFN(nvmlDeviceGetVbiosVersion);
        DECLARE_PFN(nvmlDeviceGetCurrPcieLinkWidth);
        DECLARE_PFN(nvmlDeviceGetIrqNum);
        DECLARE_PFN(nvmlDeviceGetNumGpuCores);
        DECLARE_PFN(nvmlDeviceGetBusType);
        DECLARE_PFN(nvmlDeviceGetDynamicPstatesInfo);

#undef DECLARE_PFN

        template <typename T>
        T GetProcAddress(const char* name);
    };
}
