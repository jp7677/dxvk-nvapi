#include "nvml.h"

namespace dxvk {
    Nvml* Nvml::TryLoadLibrary() {
        auto nvml = new Nvml;

        if (nvml->_nvml == nullptr) {
            delete nvml;
            return nullptr;
        }

        nvml->_initResult = nvml->_nvmlInit_v2();

        return nvml;
    }

    nvmlReturn_t Nvml::InitGetResult() const { return _initResult; }

    const char* Nvml::ErrorString(nvmlReturn_t result) const {
        return _nvmlErrorString(result);
    }

    nvmlReturn_t Nvml::DeviceGetHandleByPciBusId_v2(const char *pciBusId, nvmlDevice_t *device) const {
        return _nvmlDeviceGetHandleByPciBusId_v2(pciBusId, device);
    }

    nvmlReturn_t Nvml::DeviceGetTemperature(nvmlDevice_t device, nvmlTemperatureSensors_t sensorType, unsigned int *temp) const {
        return _nvmlDeviceGetTemperature(device, sensorType, temp);
    }

    nvmlReturn_t Nvml::DeviceGetUtilizationRates(nvmlDevice_t device, nvmlUtilization_t *utilization) const {
        return _nvmlDeviceGetUtilizationRates(device, utilization);
    }

    Nvml::Nvml() : _nvml(LoadLibraryA("nvml.dll"), FreeLibrary) {
        if (_nvml.get() == nullptr)
            return;

        #define LOAD_FUNCPTR(f) if (!(*(void**)(&_##f) = (void*)GetProcAddress(_nvml.get(), #f))) { _nvml.reset(nullptr); return; }

        LOAD_FUNCPTR(nvmlErrorString);
        LOAD_FUNCPTR(nvmlInitWithFlags);
        LOAD_FUNCPTR(nvmlInit_v2);
        LOAD_FUNCPTR(nvmlShutdown);
        LOAD_FUNCPTR(nvmlDeviceGetHandleByPciBusId_v2);
        LOAD_FUNCPTR(nvmlDeviceGetTemperature);
        LOAD_FUNCPTR(nvmlDeviceGetUtilizationRates);

        #undef LOAD_FUNCPTR
    }

    Nvml::~Nvml() {
        if (_nvml && _initResult == NVML_SUCCESS)
            _nvmlShutdown();
    }
}