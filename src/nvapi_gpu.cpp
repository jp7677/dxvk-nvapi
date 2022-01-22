#include "nvapi_private.h"
#include "nvapi_static.h"
#include "util/util_statuscode.h"
#include "util/util_string.h"

extern "C" {
    using namespace dxvk;

    NvAPI_Status __cdecl NvAPI_GPU_GetGPUType(NvPhysicalGpuHandle hPhysicalGpu, NV_GPU_TYPE *pGpuType) {
        constexpr auto n = __func__;

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (hPhysicalGpu == nullptr || pGpuType == nullptr)
            return InvalidArgument(n);

        auto adapter = reinterpret_cast<NvapiAdapter*>(hPhysicalGpu);
        if (!nvapiAdapterRegistry->IsAdapter(adapter))
            return ExpectedPhysicalGpuHandle(n);

        *pGpuType = (NV_GPU_TYPE) adapter->GetGpuType();

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_GPU_GetPCIIdentifiers(NvPhysicalGpuHandle hPhysicalGpu, NvU32 *pDeviceId, NvU32 *pSubSystemId, NvU32 *pRevisionId, NvU32 *pExtDeviceId) {
        constexpr auto n = __func__;

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (hPhysicalGpu == nullptr || pDeviceId == nullptr || pSubSystemId == nullptr || pRevisionId == nullptr || pExtDeviceId == nullptr)
            return InvalidArgument(n);

        auto adapter = reinterpret_cast<NvapiAdapter*>(hPhysicalGpu);
        if (!nvapiAdapterRegistry->IsAdapter(adapter))
            return ExpectedPhysicalGpuHandle(n);

        *pDeviceId = adapter->GetDeviceId();
        *pSubSystemId = adapter->GetSubSystemId();
        *pRevisionId = 0; // In most cases this will be 0xa1
        *pExtDeviceId = adapter->GetExternalDeviceId();

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_GPU_GetFullName(NvPhysicalGpuHandle hPhysicalGpu, NvAPI_ShortString szName) {
        constexpr auto n = __func__;

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (hPhysicalGpu == nullptr || szName == nullptr)
            return InvalidArgument(n);

        auto adapter = reinterpret_cast<NvapiAdapter*>(hPhysicalGpu);
        if (!nvapiAdapterRegistry->IsAdapter(adapter))
            return ExpectedPhysicalGpuHandle(n);

        strcpy(szName, adapter->GetDeviceName().c_str());

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_GPU_GetBusId(NvPhysicalGpuHandle hPhysicalGpu, NvU32 *pBusId) {
        constexpr auto n = __func__;

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (hPhysicalGpu == nullptr || pBusId == nullptr)
            return InvalidArgument(n);

        auto adapter = reinterpret_cast<NvapiAdapter*>(hPhysicalGpu);
        if (!nvapiAdapterRegistry->IsAdapter(adapter))
            return ExpectedPhysicalGpuHandle(n);

        *pBusId = adapter->GetPciBusId();

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_GPU_GetBusSlotId(NvPhysicalGpuHandle hPhysicalGpu, NvU32 *pBusSlotId) {
        constexpr auto n = __func__;

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (hPhysicalGpu == nullptr || pBusSlotId == nullptr)
            return InvalidArgument(n);

        auto adapter = reinterpret_cast<NvapiAdapter*>(hPhysicalGpu);
        if (!nvapiAdapterRegistry->IsAdapter(adapter))
            return ExpectedPhysicalGpuHandle(n);

        // The bus slot ID seems to be the PCI Device ID. This is based on the CUDA documentation which says:
        // 'PCI device (also known as slot) identifier of the device',
        // see https://docs.nvidia.com/cuda/cuda-runtime-api/group__CUDART__DEVICE.html#group__CUDART__DEVICE_1g1bf9d625a931d657e08db2b4391170f0
        *pBusSlotId = adapter->GetPciDeviceId();

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_GPU_GetBusType(NvPhysicalGpuHandle hPhysicalGpu,NV_GPU_BUS_TYPE *pBusType) {
        constexpr auto n = __func__;

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (hPhysicalGpu == nullptr || pBusType == nullptr)
            return InvalidArgument(n);

        auto adapter = reinterpret_cast<NvapiAdapter*>(hPhysicalGpu);
        if (!nvapiAdapterRegistry->IsAdapter(adapter))
            return ExpectedPhysicalGpuHandle(n);

        if (adapter->GetArchitectureId() >= NV_GPU_ARCHITECTURE_GM200)
            *pBusType = NVAPI_GPU_BUS_TYPE_PCI_EXPRESS; // Assume PCIe on Maxwell and newer
        else
            *pBusType = NVAPI_GPU_BUS_TYPE_UNDEFINED;


        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_GPU_GetPhysicalFrameBufferSize(NvPhysicalGpuHandle hPhysicalGpu, NvU32 *pSize) {
        constexpr auto n = __func__;

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (pSize == nullptr)
            return InvalidArgument(n);

        auto adapter = reinterpret_cast<NvapiAdapter*>(hPhysicalGpu);
        if (!nvapiAdapterRegistry->IsAdapter(adapter))
            return ExpectedPhysicalGpuHandle(n);

        *pSize = adapter->GetVRamSize();

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_GPU_GetAdapterIdFromPhysicalGpu(NvPhysicalGpuHandle hPhysicalGpu, void *pOSAdapterId) {
        constexpr auto n = __func__;

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (pOSAdapterId == nullptr)
            return InvalidArgument(n);

        auto adapter = reinterpret_cast<NvapiAdapter*>(hPhysicalGpu);
        if (!nvapiAdapterRegistry->IsAdapter(adapter))
            return ExpectedPhysicalGpuHandle(n);

        auto luid = adapter->GetLuid();
        if (!luid.has_value())
            return Error(n);

        memcpy(pOSAdapterId, &luid.value(), sizeof(luid));

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_GPU_GetArchInfo(NvPhysicalGpuHandle hPhysicalGpu, NV_GPU_ARCH_INFO *pGpuArchInfo) {
        constexpr auto n = __func__;

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (pGpuArchInfo == nullptr)
            return InvalidArgument(n);

        auto adapter = reinterpret_cast<NvapiAdapter*>(hPhysicalGpu);
        if (!nvapiAdapterRegistry->IsAdapter(adapter))
            return ExpectedPhysicalGpuHandle(n);

        if (pGpuArchInfo->version != NV_GPU_ARCH_INFO_VER_1 && pGpuArchInfo->version != NV_GPU_ARCH_INFO_VER_2)
            return IncompatibleStructVersion(n);

        if (adapter->GetDriverId() != VK_DRIVER_ID_NVIDIA_PROPRIETARY)
            return NvidiaDeviceNotFound(n);

        pGpuArchInfo->architecture_id = adapter->GetArchitectureId();

        // Assume the implementation ID from the architecture ID. No simple way
        // to do a more fine-grained query at this time. Would need wine-nvml
        // usage.
        switch(pGpuArchInfo->architecture_id) {
            case NV_GPU_ARCHITECTURE_GA100:
                pGpuArchInfo->implementation_id = NV_GPU_ARCH_IMPLEMENTATION_GA102;
                break;
            case NV_GPU_ARCHITECTURE_TU100:
                pGpuArchInfo->implementation_id = NV_GPU_ARCH_IMPLEMENTATION_TU102;
                break;
            case NV_GPU_ARCHITECTURE_GV100:
                pGpuArchInfo->implementation_id = NV_GPU_ARCH_IMPLEMENTATION_GV100;
                break;
            case NV_GPU_ARCHITECTURE_GP100:
                pGpuArchInfo->implementation_id = NV_GPU_ARCH_IMPLEMENTATION_GP102;
                break;
            case NV_GPU_ARCHITECTURE_GM200:
                pGpuArchInfo->implementation_id = NV_GPU_ARCH_IMPLEMENTATION_GM204;
                break;
            case NV_GPU_ARCHITECTURE_GK100:
                pGpuArchInfo->implementation_id = NV_GPU_ARCH_IMPLEMENTATION_GK104;
                break;
            default:
                return Error(n);
        }

        // Assume first revision, no way to query currently.
        pGpuArchInfo->revision_id = NV_GPU_CHIP_REV_A01;

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_GPU_CudaEnumComputeCapableGpus(NV_COMPUTE_GPU_TOPOLOGY *pComputeTopo) {
        constexpr auto n = __func__;

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (pComputeTopo == nullptr)
            return InvalidArgument(n);

        if (pComputeTopo->version != NV_COMPUTE_GPU_TOPOLOGY_VER && pComputeTopo->version != NV_COMPUTE_GPU_TOPOLOGY_VER1)
            return IncompatibleStructVersion(n);

        auto pComputeTopoV1 = reinterpret_cast<NV_COMPUTE_GPU_TOPOLOGY_V1*>(pComputeTopo);

        auto cudaCapableGpuCount = 0U;
        // Those flags match NVAPI on Windows for a normal desktop machine
        auto flags = NV_COMPUTE_GPU_TOPOLOGY_PHYSICS_CAPABLE | NV_COMPUTE_GPU_TOPOLOGY_PHYSICS_ENABLE | NV_COMPUTE_GPU_TOPOLOGY_PHYSICS_RECOMMENDED;
        for (auto i = 0U; i < nvapiAdapterRegistry->GetAdapterCount(); i++) {
            auto adapter = nvapiAdapterRegistry->GetAdapter(i);
            if (adapter->GetDriverId() != VK_DRIVER_ID_NVIDIA_PROPRIETARY ||
                adapter->GetArchitectureId() < NV_GPU_ARCHITECTURE_GM200) // Maxwell is the oldest generation we can detect
                continue;

            if (pComputeTopo->version == NV_COMPUTE_GPU_TOPOLOGY_VER1) {
                pComputeTopoV1->computeGpus[cudaCapableGpuCount].hPhysicalGpu = reinterpret_cast<NvPhysicalGpuHandle>(adapter);
                pComputeTopoV1->computeGpus[cudaCapableGpuCount].flags = flags;
            } else {
                pComputeTopo->computeGpus[cudaCapableGpuCount].hPhysicalGpu = reinterpret_cast<NvPhysicalGpuHandle>(adapter);
                pComputeTopo->computeGpus[cudaCapableGpuCount].flags = flags;
            }

            cudaCapableGpuCount++;
        }

        if (pComputeTopo->version == NV_COMPUTE_GPU_TOPOLOGY_VER1)
            pComputeTopoV1->gpuCount = cudaCapableGpuCount;
        else
            pComputeTopo->gpuCount = cudaCapableGpuCount;

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_GPU_GetVbiosVersionString(NvPhysicalGpuHandle hPhysicalGpu, NvAPI_ShortString szBiosRevision) {
        constexpr auto n = __func__;

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (szBiosRevision == nullptr)
            return InvalidArgument(n);

        auto adapter = reinterpret_cast<NvapiAdapter*>(hPhysicalGpu);
        if (!nvapiAdapterRegistry->IsAdapter(adapter))
            return ExpectedPhysicalGpuHandle(n);

        if (!adapter->HasNvml() || !adapter->HasNvmlDevice()) {
            strcpy(szBiosRevision, "N/A");
            return Ok(n);
        }

        char version[NVML_DEVICE_INFOROM_VERSION_BUFFER_SIZE];
        auto result = adapter->GetNvmlDeviceVbiosVersion(version, NVML_DEVICE_INFOROM_VERSION_BUFFER_SIZE);
        switch (result) {
            case NVML_SUCCESS:
                strcpy(szBiosRevision, version);
                return Ok(n);
            case NVML_ERROR_NOT_SUPPORTED:
                return NotSupported(n);
            case NVML_ERROR_GPU_IS_LOST:
                return HandleInvalidated(n);
            default:
                return Error(str::format(n, ": ", adapter->GetNvmlErrorString(result)));
        }
    }

    NvAPI_Status __cdecl NvAPI_GPU_GetDynamicPstatesInfoEx(NvPhysicalGpuHandle hPhysicalGpu, NV_GPU_DYNAMIC_PSTATES_INFO_EX *pDynamicPstatesInfoEx) {
        constexpr auto n = __func__;
        static bool alreadyLoggedNoNvml = false;
        static bool alreadyLoggedHandleInvalidated = false;
        static bool alreadyLoggedOk = false;

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (pDynamicPstatesInfoEx == nullptr)
            return InvalidArgument(n);

        if (pDynamicPstatesInfoEx->version != NV_GPU_DYNAMIC_PSTATES_INFO_EX_VER)
            return IncompatibleStructVersion(n);

        auto adapter = reinterpret_cast<NvapiAdapter*>(hPhysicalGpu);
        if (!nvapiAdapterRegistry->IsAdapter(adapter))
            return ExpectedPhysicalGpuHandle(n);

        if (!adapter->HasNvml())
            return NoImplementation(n, alreadyLoggedNoNvml);

        if (!adapter->HasNvmlDevice())
            return HandleInvalidated(str::format(n, ": NVML available but current adapter is not NVML compatible"), alreadyLoggedHandleInvalidated);

        nvmlUtilization_t utilization;
        auto result = adapter->GetNvmlDeviceUtilizationRates(&utilization);
        switch (result) {
            case NVML_SUCCESS:
                pDynamicPstatesInfoEx->flags = 0;
                pDynamicPstatesInfoEx->utilization[0].bIsPresent = 1;
                pDynamicPstatesInfoEx->utilization[0].percentage = utilization.gpu;
                pDynamicPstatesInfoEx->utilization[1].bIsPresent = 1;
                pDynamicPstatesInfoEx->utilization[1].percentage = utilization.memory;
                pDynamicPstatesInfoEx->utilization[2].bIsPresent = 1;
                pDynamicPstatesInfoEx->utilization[2].percentage = 0;
                pDynamicPstatesInfoEx->utilization[3].bIsPresent = 1;
                pDynamicPstatesInfoEx->utilization[3].percentage = 0;
                for (auto i = 4U; i < NVAPI_MAX_GPU_UTILIZATIONS; i++)
                    pDynamicPstatesInfoEx->utilization[i].bIsPresent = 0;

                return Ok(n, alreadyLoggedOk);
            case NVML_ERROR_NOT_SUPPORTED:
                pDynamicPstatesInfoEx->flags = 0;
                for (auto& util : pDynamicPstatesInfoEx->utilization)
                    util.bIsPresent = 0;

                return NotSupported(n);
            case NVML_ERROR_GPU_IS_LOST:
                return HandleInvalidated(n);
            default:
                return Error(str::format(n, ": ", adapter->GetNvmlErrorString(result)));
        }
    }

    NvAPI_Status __cdecl NvAPI_GPU_GetThermalSettings(NvPhysicalGpuHandle hPhysicalGpu, NvU32 sensorIndex, NV_GPU_THERMAL_SETTINGS *pThermalSettings) {
        constexpr auto n = __func__;
        static bool alreadyLoggedNoNvml = false;
        static bool alreadyLoggedHandleInvalidated = false;
        static bool alreadyLoggedOk = false;

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (pThermalSettings == nullptr)
            return InvalidArgument(n);

        if (pThermalSettings->version != NV_GPU_THERMAL_SETTINGS_VER_1 && pThermalSettings->version != NV_GPU_THERMAL_SETTINGS_VER_2)
            return IncompatibleStructVersion(n);

        auto adapter = reinterpret_cast<NvapiAdapter*>(hPhysicalGpu);
        if (!nvapiAdapterRegistry->IsAdapter(adapter))
            return ExpectedPhysicalGpuHandle(n);

        if (sensorIndex != 0 && sensorIndex != NVAPI_THERMAL_TARGET_ALL) {
            pThermalSettings->count = 0;
            return Ok(n, alreadyLoggedOk);
        }

        if (!adapter->HasNvml())
            return NoImplementation(n, alreadyLoggedNoNvml);

        if (!adapter->HasNvmlDevice())
            return HandleInvalidated(str::format(n, ": NVML available but current adapter is not NVML compatible"), alreadyLoggedHandleInvalidated);

        unsigned int temp;
        auto result = adapter->GetNvmlDeviceTemperature(NVML_TEMPERATURE_GPU, &temp);
        switch (result) {
            case NVML_SUCCESS:
                switch (pThermalSettings->version) {
                    case NV_GPU_THERMAL_SETTINGS_VER_2:
                        pThermalSettings->count = 1;
                        pThermalSettings->sensor[0].controller = NVAPI_THERMAL_CONTROLLER_UNKNOWN;
                        pThermalSettings->sensor[0].target = NVAPI_THERMAL_TARGET_GPU;
                        pThermalSettings->sensor[0].currentTemp = static_cast<int>(temp);
                        pThermalSettings->sensor[0].defaultMaxTemp = 127;
                        pThermalSettings->sensor[0].defaultMinTemp = -256;
                        break;
                    case NV_GPU_THERMAL_SETTINGS_VER_1:
                        auto pThermalSettingsV1 = reinterpret_cast<NV_GPU_THERMAL_SETTINGS_V1*>(pThermalSettings);
                        pThermalSettingsV1->count = 1;
                        pThermalSettingsV1->sensor[0].controller = NVAPI_THERMAL_CONTROLLER_UNKNOWN;
                        pThermalSettingsV1->sensor[0].target = NVAPI_THERMAL_TARGET_GPU;
                        pThermalSettingsV1->sensor[0].currentTemp = temp;
                        pThermalSettingsV1->sensor[0].defaultMaxTemp = 127;
                        pThermalSettingsV1->sensor[0].defaultMinTemp = 0;
                        break;
                }

                return Ok(n, alreadyLoggedOk);
            case NVML_ERROR_NOT_SUPPORTED:
                switch (pThermalSettings->version) {
                    case NV_GPU_THERMAL_SETTINGS_VER_2:
                        pThermalSettings->count = 0;
                        break;
                    case NV_GPU_THERMAL_SETTINGS_VER_1:
                        auto pThermalSettingsV1 = reinterpret_cast<NV_GPU_THERMAL_SETTINGS_V1*>(pThermalSettings);
                        pThermalSettingsV1->count = 0;
                        break;
                }

                return NotSupported(n);
            case NVML_ERROR_GPU_IS_LOST:
                return HandleInvalidated(n);
            default:
                return Error(str::format(n, ": ", adapter->GetNvmlErrorString(result)));
        }
    }

    NvAPI_Status __cdecl NvAPI_GPU_GetCurrentPstate(NvPhysicalGpuHandle hPhysicalGpu, NV_GPU_PERF_PSTATE_ID *pCurrentPstate) {
        constexpr auto n = __func__;
        static bool alreadyLoggedNoNvml = false;
        static bool alreadyLoggedHandleInvalidated = false;
        static bool alreadyLoggedOk = false;

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (pCurrentPstate == nullptr)
            return InvalidArgument(n);

        auto adapter = reinterpret_cast<NvapiAdapter*>(hPhysicalGpu);
        if (!nvapiAdapterRegistry->IsAdapter(adapter))
            return ExpectedPhysicalGpuHandle(n);

        if (!adapter->HasNvml())
            return NoImplementation(n, alreadyLoggedNoNvml);

        if (!adapter->HasNvmlDevice())
            return HandleInvalidated(str::format(n, ": NVML available but current adapter is not NVML compatible"), alreadyLoggedHandleInvalidated);

        nvmlPstates_t pState;
        auto result = adapter->GetNvmlPerformanceState(&pState);
        switch (result) {
            case NVML_SUCCESS:
                *pCurrentPstate = static_cast<NV_GPU_PERF_PSTATE_ID>(pState);
                return Ok(n, alreadyLoggedOk);
            case NVML_ERROR_NOT_SUPPORTED:
                return NotSupported(n);
            case NVML_ERROR_GPU_IS_LOST:
                return HandleInvalidated(n);
            default:
                return Error(str::format(n, ": ", adapter->GetNvmlErrorString(result)));
       }
    }

    NvAPI_Status __cdecl NvAPI_GPU_GetAllClockFrequencies(NvPhysicalGpuHandle hPhysicalGpu, NV_GPU_CLOCK_FREQUENCIES *pClkFreqs) {
        constexpr auto n = __func__;
        static bool alreadyLoggedNotSupported = false;
        static bool alreadyLoggedNoNvml = false;
        static bool alreadyLoggedHandleInvalidated = false;
        static bool alreadyLoggedOk = false;

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (pClkFreqs == nullptr)
            return InvalidArgument(n);

        if (pClkFreqs->version != NV_GPU_CLOCK_FREQUENCIES_VER_1 && pClkFreqs->version != NV_GPU_CLOCK_FREQUENCIES_VER_2 && pClkFreqs->version != NV_GPU_CLOCK_FREQUENCIES_VER_3)
            return IncompatibleStructVersion(n);

        // Only check for CURRENT_FREQ, and not for the other types ie. BOOST or DEFAULT for now
        if (pClkFreqs->ClockType != static_cast<unsigned int>(NV_GPU_CLOCK_FREQUENCIES_CURRENT_FREQ))
            return NotSupported(n, alreadyLoggedNotSupported);

        auto adapter = reinterpret_cast<NvapiAdapter*>(hPhysicalGpu);
        if (!nvapiAdapterRegistry->IsAdapter(adapter))
            return ExpectedPhysicalGpuHandle(n);

        if (!adapter->HasNvml())
            return NoImplementation(n, alreadyLoggedNoNvml);

        if (!adapter->HasNvmlDevice())
            return HandleInvalidated(str::format(n, ": NVML available but current adapter is not NVML compatible"), alreadyLoggedHandleInvalidated);

        // Reset all clock data for all domains
        for (auto& domain : pClkFreqs->domain){
            domain.bIsPresent = 0;
            domain.frequency = 0;
        }

        unsigned int clock;
        // Seemingly we need to do nvml call on a "per clock unit" to get the clock
        // Set the availability of the clock to TRUE and the nvml read clock in the nvapi struct
        auto resultGpu = adapter->GetNvmlDeviceClockInfo(NVML_CLOCK_GRAPHICS, &clock);
            switch (resultGpu) {
                case NVML_SUCCESS:
                    pClkFreqs->domain[NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS].bIsPresent = 1;
                    pClkFreqs->domain[NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS].frequency = (clock * 1000);
                    break;
                case NVML_ERROR_NOT_SUPPORTED:
                    break;
                case NVML_ERROR_GPU_IS_LOST:
                    return HandleInvalidated(n);
                default:
                    return Error(str::format(n, ": ", adapter->GetNvmlErrorString(resultGpu)));
            }

        auto resultMem = adapter->GetNvmlDeviceClockInfo(NVML_CLOCK_MEM, &clock);
            switch (resultMem) {
                case NVML_SUCCESS:
                    pClkFreqs->domain[NVAPI_GPU_PUBLIC_CLOCK_MEMORY].bIsPresent = 1;
                    pClkFreqs->domain[NVAPI_GPU_PUBLIC_CLOCK_MEMORY].frequency = (clock * 1000);
                    break;
                case NVML_ERROR_NOT_SUPPORTED:
                    break;
                case NVML_ERROR_GPU_IS_LOST:
                    return HandleInvalidated(n);
                default:
                    return Error(str::format(n, ": ", adapter->GetNvmlErrorString(resultMem)));
            }

        auto resultVid = adapter->GetNvmlDeviceClockInfo(NVML_CLOCK_VIDEO, &clock);
            switch (resultVid) {
                case NVML_SUCCESS:
                    pClkFreqs->domain[NVAPI_GPU_PUBLIC_CLOCK_VIDEO].bIsPresent = 1;
                    pClkFreqs->domain[NVAPI_GPU_PUBLIC_CLOCK_VIDEO].frequency = (clock * 1000);
                    break;
                case NVML_ERROR_NOT_SUPPORTED:
                    break;
                case NVML_ERROR_GPU_IS_LOST:
                    return HandleInvalidated(n);
                default:
                    return Error(str::format(n, ": ", adapter->GetNvmlErrorString(resultVid)));
            }

        if (resultGpu == NVML_ERROR_NOT_SUPPORTED
            && resultMem == NVML_ERROR_NOT_SUPPORTED
            && resultVid == NVML_ERROR_NOT_SUPPORTED)
            return NotSupported(n);

       return Ok(n, alreadyLoggedOk);
    }
}
