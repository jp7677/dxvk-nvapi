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
        [[nodiscard]] virtual nvmlReturn_t DeviceGetPciInfo_v3(nvmlDevice_t device, nvmlPciInfo_t* pci) const;
        [[nodiscard]] virtual nvmlReturn_t DeviceGetClockInfo(nvmlDevice_t device, nvmlClockType_t type, unsigned int* clock) const;
        [[nodiscard]] virtual nvmlReturn_t DeviceGetTemperature(nvmlDevice_t device, nvmlTemperatureSensors_t sensorType, unsigned int* temp) const;
        [[nodiscard]] virtual nvmlReturn_t DeviceGetThermalSettings(nvmlDevice_t device, unsigned int sensorIndex, nvmlGpuThermalSettings_t* pThermalSettings) const;
        [[nodiscard]] virtual nvmlReturn_t DeviceGetPerformanceState(nvmlDevice_t device, nvmlPstates_t* pState) const;
        [[nodiscard]] virtual nvmlReturn_t DeviceGetUtilizationRates(nvmlDevice_t device, nvmlUtilization_t* utilization) const;
        [[nodiscard]] virtual nvmlReturn_t DeviceGetVbiosVersion(nvmlDevice_t device, char* version, unsigned int length) const;
        [[nodiscard]] virtual nvmlReturn_t DeviceGetBusType(nvmlDevice_t device, nvmlBusType_t* type) const;
        [[nodiscard]] virtual nvmlReturn_t DeviceGetDynamicPstatesInfo(nvmlDevice_t device, nvmlGpuDynamicPstatesInfo_t* pDynamicPstatesInfo) const;

      private:
        typedef decltype(&nvmlInit_v2) PFN_nvmlInit_v2;
        typedef decltype(&nvmlShutdown) PFN_nvmlShutdown;
        typedef decltype(&nvmlErrorString) PFN_nvmlErrorString;
        typedef decltype(&nvmlDeviceGetHandleByPciBusId_v2) PFN_nvmlDeviceGetHandleByPciBusId_v2;
        typedef decltype(&nvmlDeviceGetPciInfo_v3) PFN_nvmlDeviceGetPciInfo_v3;
        typedef decltype(&nvmlDeviceGetClockInfo) PFN_nvmlDeviceGetClockInfo;
        typedef decltype(&nvmlDeviceGetTemperature) PFN_nvmlDeviceGetTemperature;
        typedef decltype(&nvmlDeviceGetThermalSettings) PFN_nvmlDeviceGetThermalSettings;
        typedef decltype(&nvmlDeviceGetPerformanceState) PFN_nvmlDeviceGetPerformanceState;
        typedef decltype(&nvmlDeviceGetUtilizationRates) PFN_nvmlDeviceGetUtilizationRates;
        typedef decltype(&nvmlDeviceGetVbiosVersion) PFN_nvmlDeviceGetVbiosVersion;
        typedef decltype(&nvmlDeviceGetBusType) PFN_nvmlDeviceGetBusType;
        typedef decltype(&nvmlDeviceGetDynamicPstatesInfo) PFN_nvmlDeviceGetDynamicPstatesInfo;

        HMODULE m_nvmlModule{};
        PFN_nvmlInit_v2 m_nvmlInit_v2{};
        PFN_nvmlShutdown m_nvmlShutdown{};
        PFN_nvmlErrorString m_nvmlErrorString{};
        PFN_nvmlDeviceGetHandleByPciBusId_v2 m_nvmlDeviceGetHandleByPciBusId_v2{};
        PFN_nvmlDeviceGetPciInfo_v3 m_nvmlDeviceGetPciInfo_v3{};
        PFN_nvmlDeviceGetClockInfo m_nvmlDeviceGetClockInfo{};
        PFN_nvmlDeviceGetTemperature m_nvmlDeviceGetTemperature{};
        PFN_nvmlDeviceGetThermalSettings m_nvmlDeviceGetThermalSettings{};
        PFN_nvmlDeviceGetPerformanceState m_nvmlDeviceGetPerformanceState{};
        PFN_nvmlDeviceGetUtilizationRates m_nvmlDeviceGetUtilizationRates{};
        PFN_nvmlDeviceGetVbiosVersion m_nvmlDeviceGetVbiosVersion{};
        PFN_nvmlDeviceGetBusType m_nvmlDeviceGetBusType{};
        PFN_nvmlDeviceGetDynamicPstatesInfo m_nvmlDeviceGetDynamicPstatesInfo{};

        template <typename T>
        T GetProcAddress(const char* name);
    };
}
