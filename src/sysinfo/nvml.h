#pragma once

#include "../nvapi_private.h"

namespace dxvk {
    class Nvml final {

    public:
        Nvml(const Nvml&) = delete;
        Nvml(Nvml&&) = default;
        Nvml& operator=(const Nvml&) = delete;
        Nvml& operator=(Nvml&&) = default;
        ~Nvml();

        [[nodiscard]] bool IsAvailable();
        [[nodiscard]] static Nvml* TryLoadLibrary();
        [[nodiscard]] nvmlReturn_t InitGetResult() const;
        [[nodiscard]] const char* ErrorString(nvmlReturn_t result) const;
        [[nodiscard]] nvmlReturn_t DeviceGetHandleByPciBusId_v2(const char *pciBusId, nvmlDevice_t *device) const;
        [[nodiscard]] nvmlReturn_t DeviceGetTemperature(nvmlDevice_t device, nvmlTemperatureSensors_t sensorType, unsigned int *temp) const;
        [[nodiscard]] nvmlReturn_t DeviceGetUtilizationRates(nvmlDevice_t device, nvmlUtilization_t *utilization) const;

    private:
        Nvml();

        std::unique_ptr<std::remove_pointer_t<HMODULE>, decltype(&FreeLibrary)> _nvml;

        #define DECLARE_FUNCPTR(f) decltype(&f) _##f;

        DECLARE_FUNCPTR(nvmlErrorString);
        DECLARE_FUNCPTR(nvmlInitWithFlags);
        DECLARE_FUNCPTR(nvmlInit_v2);
        DECLARE_FUNCPTR(nvmlShutdown);
        DECLARE_FUNCPTR(nvmlDeviceGetHandleByPciBusId_v2);
        DECLARE_FUNCPTR(nvmlDeviceGetTemperature);
        DECLARE_FUNCPTR(nvmlDeviceGetUtilizationRates);

        #undef DECLARE_FUNCPTR

        nvmlReturn_t _initResult;
    };
}
