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

#define GETPROCADDR(x) m_##x = GetProcAddress<decltype(&x)>(#x)

        GETPROCADDR(nvmlInit_v2);
        GETPROCADDR(nvmlShutdown);
        GETPROCADDR(nvmlErrorString);
        GETPROCADDR(nvmlDeviceGetHandleByPciBusId_v2);
        GETPROCADDR(nvmlDeviceGetPciInfo_v3);
        GETPROCADDR(nvmlDeviceGetClockInfo);
        GETPROCADDR(nvmlDeviceGetTemperature);
        GETPROCADDR(nvmlDeviceGetThermalSettings);
        GETPROCADDR(nvmlDeviceGetPerformanceState);
        GETPROCADDR(nvmlDeviceGetUtilizationRates);
        GETPROCADDR(nvmlDeviceGetVbiosVersion);
        GETPROCADDR(nvmlDeviceGetCurrPcieLinkWidth);
        GETPROCADDR(nvmlDeviceGetIrqNum);
        GETPROCADDR(nvmlDeviceGetNumGpuCores);
        GETPROCADDR(nvmlDeviceGetBusType);
        GETPROCADDR(nvmlDeviceGetDynamicPstatesInfo);

#undef GETPROCADDR

        if (m_nvmlInit_v2 == nullptr
            || m_nvmlShutdown == nullptr
            || m_nvmlErrorString == nullptr
            || m_nvmlDeviceGetHandleByPciBusId_v2 == nullptr)
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

    bool Nvml::IsAvailable() const {
        return m_nvmlModule != nullptr;
    }

    const char* Nvml::ErrorString(nvmlReturn_t result) const {
        return m_nvmlErrorString(result);
    }

    nvmlReturn_t Nvml::DeviceGetHandleByPciBusId_v2(const char* pciBusId, nvmlDevice_t* device) const {
        return m_nvmlDeviceGetHandleByPciBusId_v2(pciBusId, device);
    }

    nvmlReturn_t Nvml::DeviceGetPciInfo_v3(nvmlDevice_t device, nvmlPciInfo_t* pci) const {
        return m_nvmlDeviceGetPciInfo_v3
            ? m_nvmlDeviceGetPciInfo_v3(device, pci)
            : NVML_ERROR_FUNCTION_NOT_FOUND;
    }

    nvmlReturn_t Nvml::DeviceGetClockInfo(nvmlDevice_t device, nvmlClockType_t type, unsigned int* clock) const {
        return m_nvmlDeviceGetClockInfo
            ? m_nvmlDeviceGetClockInfo(device, type, clock)
            : NVML_ERROR_FUNCTION_NOT_FOUND;
    }

    nvmlReturn_t Nvml::DeviceGetTemperature(nvmlDevice_t device, nvmlTemperatureSensors_t sensorType, unsigned int* temp) const {
        return m_nvmlDeviceGetTemperature
            ? m_nvmlDeviceGetTemperature(device, sensorType, temp)
            : NVML_ERROR_FUNCTION_NOT_FOUND;
    }

    nvmlReturn_t Nvml::DeviceGetThermalSettings(nvmlDevice_t device, unsigned int sensorIndex, nvmlGpuThermalSettings_t* pThermalSettings) const {
        return m_nvmlDeviceGetThermalSettings
            ? m_nvmlDeviceGetThermalSettings(device, sensorIndex, pThermalSettings)
            : NVML_ERROR_FUNCTION_NOT_FOUND;
    }

    nvmlReturn_t Nvml::DeviceGetPerformanceState(nvmlDevice_t device, nvmlPstates_t* pState) const {
        return m_nvmlDeviceGetPerformanceState
            ? m_nvmlDeviceGetPerformanceState(device, pState)
            : NVML_ERROR_FUNCTION_NOT_FOUND;
    }

    nvmlReturn_t Nvml::DeviceGetUtilizationRates(nvmlDevice_t device, nvmlUtilization_t* utilization) const {
        return m_nvmlDeviceGetUtilizationRates
            ? m_nvmlDeviceGetUtilizationRates(device, utilization)
            : NVML_ERROR_FUNCTION_NOT_FOUND;
    }

    nvmlReturn_t Nvml::DeviceGetVbiosVersion(nvmlDevice_t device, char* version, unsigned int length) const {
        return m_nvmlDeviceGetVbiosVersion
            ? m_nvmlDeviceGetVbiosVersion(device, version, length)
            : NVML_ERROR_FUNCTION_NOT_FOUND;
    }

    nvmlReturn_t Nvml::DeviceGetCurrPcieLinkWidth(nvmlDevice_t device, unsigned int* width) const {
        return m_nvmlDeviceGetCurrPcieLinkWidth
            ? m_nvmlDeviceGetCurrPcieLinkWidth(device, width)
            : NVML_ERROR_FUNCTION_NOT_FOUND;
    }

    nvmlReturn_t Nvml::DeviceGetIrqNum(nvmlDevice_t device, unsigned int* irqNum) const {
        return m_nvmlDeviceGetIrqNum
            ? m_nvmlDeviceGetIrqNum(device, irqNum)
            : NVML_ERROR_FUNCTION_NOT_FOUND;
    }

    nvmlReturn_t Nvml::DeviceGetNumGpuCores(nvmlDevice_t device, unsigned int* numCores) const {
        return m_nvmlDeviceGetNumGpuCores
            ? m_nvmlDeviceGetNumGpuCores(device, numCores)
            : NVML_ERROR_FUNCTION_NOT_FOUND;
    }

    nvmlReturn_t Nvml::DeviceGetBusType(nvmlDevice_t device, nvmlBusType_t* type) const {
        return m_nvmlDeviceGetBusType
            ? m_nvmlDeviceGetBusType(device, type)
            : NVML_ERROR_FUNCTION_NOT_FOUND;
    }

    nvmlReturn_t Nvml::DeviceGetDynamicPstatesInfo(nvmlDevice_t device, nvmlGpuDynamicPstatesInfo_t* pDynamicPstatesInfo) const {
        return m_nvmlDeviceGetDynamicPstatesInfo
            ? m_nvmlDeviceGetDynamicPstatesInfo(device, pDynamicPstatesInfo)
            : NVML_ERROR_FUNCTION_NOT_FOUND;
    }

    NV_THERMAL_TARGET Nvml::ToNvThermalTarget(nvmlThermalTarget_t target) {
        static_assert(static_cast<int>(NVML_THERMAL_TARGET_NONE) == static_cast<int>(NVAPI_THERMAL_TARGET_NONE));
        static_assert(static_cast<int>(NVML_THERMAL_TARGET_GPU) == static_cast<int>(NVAPI_THERMAL_TARGET_GPU));
        static_assert(static_cast<int>(NVML_THERMAL_TARGET_MEMORY) == static_cast<int>(NVAPI_THERMAL_TARGET_MEMORY));
        static_assert(static_cast<int>(NVML_THERMAL_TARGET_POWER_SUPPLY) == static_cast<int>(NVAPI_THERMAL_TARGET_POWER_SUPPLY));
        static_assert(static_cast<int>(NVML_THERMAL_TARGET_BOARD) == static_cast<int>(NVAPI_THERMAL_TARGET_BOARD));
        static_assert(static_cast<int>(NVML_THERMAL_TARGET_VCD_BOARD) == static_cast<int>(NVAPI_THERMAL_TARGET_VCD_BOARD));
        static_assert(static_cast<int>(NVML_THERMAL_TARGET_VCD_INLET) == static_cast<int>(NVAPI_THERMAL_TARGET_VCD_INLET));
        static_assert(static_cast<int>(NVML_THERMAL_TARGET_VCD_OUTLET) == static_cast<int>(NVAPI_THERMAL_TARGET_VCD_OUTLET));
        static_assert(static_cast<int>(NVML_THERMAL_TARGET_ALL) == static_cast<int>(NVAPI_THERMAL_TARGET_ALL));
        static_assert(static_cast<int>(NVML_THERMAL_TARGET_UNKNOWN) == static_cast<int>(NVAPI_THERMAL_TARGET_UNKNOWN));
        return static_cast<NV_THERMAL_TARGET>(target);
    }

    NV_THERMAL_CONTROLLER Nvml::ToNvThermalController(nvmlThermalController_t controller) {
        switch (controller) {
            case NVML_THERMAL_CONTROLLER_NONE:
                return NVAPI_THERMAL_CONTROLLER_NONE;
            case NVML_THERMAL_CONTROLLER_GPU_INTERNAL:
                return NVAPI_THERMAL_CONTROLLER_GPU_INTERNAL;
            case NVML_THERMAL_CONTROLLER_ADM1032:
                return NVAPI_THERMAL_CONTROLLER_ADM1032;
            // case NVML_THERMAL_CONTROLLER_ADT7461:
            //     return NVAPI_THERMAL_CONTROLLER_ADT7461;
            case NVML_THERMAL_CONTROLLER_MAX6649:
                return NVAPI_THERMAL_CONTROLLER_MAX6649;
            case NVML_THERMAL_CONTROLLER_MAX1617:
                return NVAPI_THERMAL_CONTROLLER_MAX1617;
            case NVML_THERMAL_CONTROLLER_LM99:
                return NVAPI_THERMAL_CONTROLLER_LM99;
            case NVML_THERMAL_CONTROLLER_LM89:
                return NVAPI_THERMAL_CONTROLLER_LM89;
            case NVML_THERMAL_CONTROLLER_LM64:
                return NVAPI_THERMAL_CONTROLLER_LM64;
            // case NVML_THERMAL_CONTROLLER_G781:
            //     return NVAPI_THERMAL_CONTROLLER_G781;
            case NVML_THERMAL_CONTROLLER_ADT7473:
                return NVAPI_THERMAL_CONTROLLER_ADT7473;
            case NVML_THERMAL_CONTROLLER_SBMAX6649:
                return NVAPI_THERMAL_CONTROLLER_SBMAX6649;
            case NVML_THERMAL_CONTROLLER_VBIOSEVT:
                return NVAPI_THERMAL_CONTROLLER_VBIOSEVT;
            case NVML_THERMAL_CONTROLLER_OS:
                return NVAPI_THERMAL_CONTROLLER_OS;
            // case NVML_THERMAL_CONTROLLER_NVSYSCON_CANOAS:
            //     return NVAPI_THERMAL_CONTROLLER_NVSYSCON_CANOAS;
            // case NVML_THERMAL_CONTROLLER_NVSYSCON_E551:
            //     return NVAPI_THERMAL_CONTROLLER_NVSYSCON_E551;
            // case NVML_THERMAL_CONTROLLER_MAX6649R:
            //     return NVAPI_THERMAL_CONTROLLER_MAX6649R;
            // case NVML_THERMAL_CONTROLLER_ADT7473S:
            //     return NVAPI_THERMAL_CONTROLLER_ADT7473S;
            default:
                return NVAPI_THERMAL_CONTROLLER_UNKNOWN;
        }
    }

    NV_GPU_BUS_TYPE Nvml::ToNvGpuBusType(nvmlBusType_t type) {
        switch (type) {
            case NVML_BUS_TYPE_PCI:
                return NVAPI_GPU_BUS_TYPE_PCI;
            case NVML_BUS_TYPE_PCIE:
                return NVAPI_GPU_BUS_TYPE_PCI_EXPRESS;
            case NVML_BUS_TYPE_FPCI:
                return NVAPI_GPU_BUS_TYPE_FPCI;
            case NVML_BUS_TYPE_AGP:
                return NVAPI_GPU_BUS_TYPE_AGP;
            default:
                return NVAPI_GPU_BUS_TYPE_UNDEFINED;
        }
    }

    template <typename T>
    T Nvml::GetProcAddress(const char* name) {
        return reinterpret_cast<T>(reinterpret_cast<void*>(::GetProcAddress(m_nvmlModule, name)));
    }
}
