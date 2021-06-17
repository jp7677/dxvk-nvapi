#include "nvml.h"
#include "../util/util_string.h"
#include "../util/util_log.h"

namespace dxvk {
    Nvml::Nvml() {
        const auto nvmlModuleName = "nvml.dll";
        m_nvmlModule = ::LoadLibraryA(nvmlModuleName);
        if (m_nvmlModule == nullptr) {
            auto lastError = ::GetLastError();
            if (lastError != ERROR_MOD_NOT_FOUND) // Ignore library not found
                log::write(str::format("Loading ", nvmlModuleName, " failed with error code: ", lastError));

            return;
        }

        m_nvmlInit_v2 = GetProcAddress<PFN_nvmlInit_v2>("nvmlInit_v2");
        m_nvmlShutdown = GetProcAddress<PFN_nvmlShutdown>("nvmlShutdown");
        m_nvmlErrorString = GetProcAddress<PFN_nvmlErrorString>("nvmlErrorString");
        m_nvmlDeviceGetHandleByPciBusId_v2 = GetProcAddress<PFN_nvmlDeviceGetHandleByPciBusId_v2>("nvmlDeviceGetHandleByPciBusId_v2");
        m_nvmlDeviceGetTemperature = GetProcAddress<PFN_nvmlDeviceGetTemperature>("nvmlDeviceGetTemperature");
        m_nvmlDeviceGetUtilizationRates = GetProcAddress<PFN_nvmlDeviceGetUtilizationRates>("nvmlDeviceGetUtilizationRates");
        m_nvmlDeviceGetVbiosVersion = GetProcAddress<PFN_nvmlDeviceGetVbiosVersion>("nvmlDeviceGetVbiosVersion");
        m_nvmlDeviceGetClockInfo = GetProcAddress<PFN_nvmlDeviceGetClockInfo>("nvmlDeviceGetClockInfo");

        if (m_nvmlInit_v2 == nullptr
            || m_nvmlShutdown == nullptr
            || m_nvmlErrorString == nullptr
            || m_nvmlDeviceGetHandleByPciBusId_v2 == nullptr
            || m_nvmlDeviceGetTemperature == nullptr
            || m_nvmlDeviceGetUtilizationRates == nullptr
            || m_nvmlDeviceGetVbiosVersion == nullptr
            || m_nvmlDeviceGetClockInfo == nullptr)
            log::write(str::format("NVML loaded but initialization failed"));
        else {
            auto result = m_nvmlInit_v2();
            if (result == NVML_SUCCESS)
                return;

            log::write(str::format("NVML loaded but initialization failed with error: ", ErrorString(result)));
        }

        ::FreeLibrary(m_nvmlModule);
        m_nvmlModule = nullptr;
    }

    Nvml::~Nvml() {
        if (m_nvmlModule == nullptr)
            return;

        m_nvmlShutdown();
        ::FreeLibrary(m_nvmlModule);
        m_nvmlModule = nullptr;
    }

    bool Nvml::IsAvailable() {
        return m_nvmlModule != nullptr;
    }

    const char* Nvml::ErrorString(nvmlReturn_t result) const {
        return m_nvmlErrorString(result);
    }

    nvmlReturn_t Nvml::DeviceGetHandleByPciBusId_v2(const char *pciBusId, nvmlDevice_t *device) const {
        return m_nvmlDeviceGetHandleByPciBusId_v2(pciBusId, device);
    }

    nvmlReturn_t Nvml::DeviceGetTemperature(nvmlDevice_t device, nvmlTemperatureSensors_t sensorType, unsigned int *temp) const {
        return m_nvmlDeviceGetTemperature(device, sensorType, temp);
    }

    nvmlReturn_t Nvml::DeviceGetUtilizationRates(nvmlDevice_t device, nvmlUtilization_t *utilization) const {
        return m_nvmlDeviceGetUtilizationRates(device, utilization);
    }

    nvmlReturn_t Nvml::DeviceGetVbiosVersion(nvmlDevice_t device, char *version, unsigned int length) const {
        return m_nvmlDeviceGetVbiosVersion(device, version, length);
    }

    nvmlReturn_t Nvml::DeviceGetClockInfo(nvmlDevice_t device, nvmlClockType_t type, unsigned int *clock) const {
        return m_nvmlDeviceGetClockInfo(device, type, clock);
    }

    template<typename T>
    T Nvml::GetProcAddress(const char* name) {
        return reinterpret_cast<T>(reinterpret_cast<void*>(::GetProcAddress(m_nvmlModule, name)));
    }
}
