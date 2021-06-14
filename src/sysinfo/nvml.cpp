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

        m_nvmlInit_v2 =
            reinterpret_cast<PFN_nvmlInit_v2>(
                reinterpret_cast<void*>(
                    ::GetProcAddress(m_nvmlModule, "nvmlInit_v2")));

        m_nvmlShutdown =
            reinterpret_cast<PFN_nvmlShutdown>(
                reinterpret_cast<void*>(
                    ::GetProcAddress(m_nvmlModule, "nvmlShutdown")));

        m_nvmlErrorString =
            reinterpret_cast<PFN_nvmlErrorString>(
                reinterpret_cast<void*>(
                    ::GetProcAddress(m_nvmlModule, "nvmlErrorString")));

        m_nvmlDeviceGetHandleByPciBusId_v2 =
            reinterpret_cast<PFN_nvmlDeviceGetHandleByPciBusId_v2>(
                reinterpret_cast<void*>(
                    ::GetProcAddress(m_nvmlModule, "nvmlDeviceGetHandleByPciBusId_v2")));

        m_nvmlDeviceGetTemperature =
            reinterpret_cast<PFN_nvmlDeviceGetTemperature>(
                reinterpret_cast<void*>(
                    ::GetProcAddress(m_nvmlModule, "nvmlDeviceGetTemperature")));

        m_nvmlDeviceGetUtilizationRates =
            reinterpret_cast<PFN_nvmlDeviceGetUtilizationRates>(
                reinterpret_cast<void*>(
                    ::GetProcAddress(m_nvmlModule, "nvmlDeviceGetUtilizationRates")));

        if (m_nvmlInit_v2 == nullptr
            || m_nvmlShutdown == nullptr
            || m_nvmlErrorString == nullptr
            || m_nvmlDeviceGetHandleByPciBusId_v2 == nullptr
            || m_nvmlDeviceGetTemperature == nullptr
            || m_nvmlDeviceGetUtilizationRates == nullptr)
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
}