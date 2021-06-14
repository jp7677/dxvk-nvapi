#pragma once

#include "../nvapi_private.h"

namespace dxvk {
    class Nvml final {

    public:
        Nvml();
        ~Nvml();

        [[nodiscard]] bool IsAvailable();
        [[nodiscard]] const char* ErrorString(nvmlReturn_t result) const;
        [[nodiscard]] nvmlReturn_t DeviceGetHandleByPciBusId_v2(const char *pciBusId, nvmlDevice_t *device) const;
        [[nodiscard]] nvmlReturn_t DeviceGetTemperature(nvmlDevice_t device, nvmlTemperatureSensors_t sensorType, unsigned int *temp) const;
        [[nodiscard]] nvmlReturn_t DeviceGetUtilizationRates(nvmlDevice_t device, nvmlUtilization_t *utilization) const;

    private:
        typedef decltype(&nvmlInit_v2) PFN_nvmlInit_v2;
        typedef decltype(&nvmlShutdown) PFN_nvmlShutdown;
        typedef decltype(&nvmlErrorString) PFN_nvmlErrorString;
        typedef decltype(&nvmlDeviceGetHandleByPciBusId_v2) PFN_nvmlDeviceGetHandleByPciBusId_v2;
        typedef decltype(&nvmlDeviceGetTemperature) PFN_nvmlDeviceGetTemperature;
        typedef decltype(&nvmlDeviceGetUtilizationRates) PFN_nvmlDeviceGetUtilizationRates;

        HMODULE m_nvmlModule{};
        PFN_nvmlInit_v2 m_nvmlInit_v2{};
        PFN_nvmlShutdown m_nvmlShutdown{};
        PFN_nvmlErrorString m_nvmlErrorString{};
        PFN_nvmlDeviceGetHandleByPciBusId_v2 m_nvmlDeviceGetHandleByPciBusId_v2{};
        PFN_nvmlDeviceGetTemperature m_nvmlDeviceGetTemperature{};
        PFN_nvmlDeviceGetUtilizationRates m_nvmlDeviceGetUtilizationRates{};
    };
}
