#pragma once

#include "../nvapi_private.h"

namespace dxvk {
    class Nvml {

    public:
        Nvml();
        virtual ~Nvml();

        [[nodiscard]] virtual bool IsAvailable() const;
        [[nodiscard]] virtual const char* ErrorString(nvmlReturn_t result) const;
        [[nodiscard]] virtual nvmlReturn_t DeviceGetHandleByPciBusId_v2(const char *pciBusId, nvmlDevice_t *device) const;
        [[nodiscard]] virtual nvmlReturn_t DeviceGetTemperature(nvmlDevice_t device, nvmlTemperatureSensors_t sensorType, unsigned int *temp) const;
        [[nodiscard]] virtual nvmlReturn_t DeviceGetUtilizationRates(nvmlDevice_t device, nvmlUtilization_t *utilization) const;
        [[nodiscard]] virtual nvmlReturn_t DeviceGetVbiosVersion(nvmlDevice_t device, char *version, unsigned int length) const;
        [[nodiscard]] virtual nvmlReturn_t DeviceGetPerformanceState(nvmlDevice_t device, nvmlPstates_t *pState) const;
        [[nodiscard]] virtual nvmlReturn_t DeviceGetClockInfo(nvmlDevice_t device, nvmlClockType_t type, unsigned int *clock) const;

    private:
        typedef decltype(&nvmlInit_v2) PFN_nvmlInit_v2;
        typedef decltype(&nvmlShutdown) PFN_nvmlShutdown;
        typedef decltype(&nvmlErrorString) PFN_nvmlErrorString;
        typedef decltype(&nvmlDeviceGetHandleByPciBusId_v2) PFN_nvmlDeviceGetHandleByPciBusId_v2;
        typedef decltype(&nvmlDeviceGetTemperature) PFN_nvmlDeviceGetTemperature;
        typedef decltype(&nvmlDeviceGetUtilizationRates) PFN_nvmlDeviceGetUtilizationRates;
        typedef decltype(&nvmlDeviceGetVbiosVersion) PFN_nvmlDeviceGetVbiosVersion;
        typedef decltype(&nvmlDeviceGetPerformanceState) PFN_nvmlDeviceGetPerformanceState;
        typedef decltype(&nvmlDeviceGetClockInfo) PFN_nvmlDeviceGetClockInfo;

        HMODULE m_nvmlModule{};
        PFN_nvmlInit_v2 m_nvmlInit_v2{};
        PFN_nvmlShutdown m_nvmlShutdown{};
        PFN_nvmlErrorString m_nvmlErrorString{};
        PFN_nvmlDeviceGetHandleByPciBusId_v2 m_nvmlDeviceGetHandleByPciBusId_v2{};
        PFN_nvmlDeviceGetTemperature m_nvmlDeviceGetTemperature{};
        PFN_nvmlDeviceGetUtilizationRates m_nvmlDeviceGetUtilizationRates{};
        PFN_nvmlDeviceGetVbiosVersion m_nvmlDeviceGetVbiosVersion{};
        PFN_nvmlDeviceGetPerformanceState m_nvmlDeviceGetPerformanceState{};
        PFN_nvmlDeviceGetClockInfo m_nvmlDeviceGetClockInfo{};

        template<typename T> T GetProcAddress(const char* name);
    };
}
