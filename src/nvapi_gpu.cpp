#include "nvapi_private.h"
#include "nvapi_globals.h"
#include "util/util_statuscode.h"
#include "util/util_string.h"
#include "util/util_env.h"

extern "C" {
    using namespace dxvk;

    NvAPI_Status __cdecl NvAPI_GPU_GetCurrentPCIEDownstreamWidth(NvPhysicalGpuHandle hPhysicalGpu, NvU32* pWidth) {
        constexpr auto n = __func__;
        static bool alreadyLoggedNoNvml = false;
        static bool alreadyLoggedHandleInvalidated = false;
        static bool alreadyLoggedOk = false;

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (pWidth == nullptr)
            return InvalidArgument(n);

        auto adapter = reinterpret_cast<NvapiAdapter*>(hPhysicalGpu);
        if (!nvapiAdapterRegistry->IsAdapter(adapter))
            return ExpectedPhysicalGpuHandle(n);

        if (!adapter->HasNvml())
            return NoImplementation(n, alreadyLoggedNoNvml);

        if (!adapter->HasNvmlDevice())
            return HandleInvalidated(str::format(n, ": NVML available but current adapter is not NVML compatible"), alreadyLoggedHandleInvalidated);

        unsigned int width;
        auto result = adapter->GetNvmlDeviceGetCurrPcieLinkWidth(&width);
        switch (result) {
            case NVML_SUCCESS:
                *pWidth = width;
                return Ok(n, alreadyLoggedOk);
            case NVML_ERROR_FUNCTION_NOT_FOUND:
                return NoImplementation(n, alreadyLoggedNoNvml);
            case NVML_ERROR_NOT_SUPPORTED:
                return NotSupported(n);
            case NVML_ERROR_GPU_IS_LOST:
                return HandleInvalidated(n);
            default:
                return Error(str::format(n, ": ", adapter->GetNvmlErrorString(result)));
        }
    }

    NvAPI_Status __cdecl NvAPI_GPU_GetIRQ(NvPhysicalGpuHandle hPhysicalGpu, NvU32* pIRQ) {
        constexpr auto n = __func__;
        static bool alreadyLoggedNoNvml = false;
        static bool alreadyLoggedHandleInvalidated = false;
        static bool alreadyLoggedOk = false;

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (pIRQ == nullptr)
            return InvalidArgument(n);

        auto adapter = reinterpret_cast<NvapiAdapter*>(hPhysicalGpu);
        if (!nvapiAdapterRegistry->IsAdapter(adapter))
            return ExpectedPhysicalGpuHandle(n);

        if (!adapter->HasNvml())
            return NoImplementation(n, alreadyLoggedNoNvml);

        if (!adapter->HasNvmlDevice())
            return HandleInvalidated(str::format(n, ": NVML available but current adapter is not NVML compatible"), alreadyLoggedHandleInvalidated);

        unsigned int irq;
        auto result = adapter->GetNvmlDeviceGetIrqNum(&irq);
        switch (result) {
            case NVML_SUCCESS:
                *pIRQ = irq;
                return Ok(n, alreadyLoggedOk);
            case NVML_ERROR_FUNCTION_NOT_FOUND:
                return NoImplementation(n, alreadyLoggedNoNvml);
            case NVML_ERROR_NOT_SUPPORTED:
                return NotSupported(n);
            case NVML_ERROR_GPU_IS_LOST:
                return HandleInvalidated(n);
            default:
                return Error(str::format(n, ": ", adapter->GetNvmlErrorString(result)));
        }
    }

    NvAPI_Status __cdecl NvAPI_GPU_GetGpuCoreCount(NvPhysicalGpuHandle hPhysicalGpu, NvU32* pCount) {
        constexpr auto n = __func__;
        static bool alreadyLoggedNoNvml = false;
        static bool alreadyLoggedHandleInvalidated = false;
        static bool alreadyLoggedOk = false;

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (pCount == nullptr)
            return InvalidArgument(n);

        auto adapter = reinterpret_cast<NvapiAdapter*>(hPhysicalGpu);
        if (!nvapiAdapterRegistry->IsAdapter(adapter))
            return ExpectedPhysicalGpuHandle(n);

        if (!adapter->HasNvml())
            return NoImplementation(n, alreadyLoggedNoNvml);

        if (!adapter->HasNvmlDevice())
            return HandleInvalidated(str::format(n, ": NVML available but current adapter is not NVML compatible"), alreadyLoggedHandleInvalidated);

        unsigned int cores;
        auto result = adapter->GetNvmlDeviceNumGpuCores(&cores);
        switch (result) {
            case NVML_SUCCESS:
                *pCount = cores;
                return Ok(n, alreadyLoggedOk);
            case NVML_ERROR_FUNCTION_NOT_FOUND:
                return NoImplementation(n, alreadyLoggedNoNvml);
            case NVML_ERROR_NOT_SUPPORTED:
                return NotSupported(n);
            case NVML_ERROR_GPU_IS_LOST:
                return HandleInvalidated(n);
            default:
                return Error(str::format(n, ": ", adapter->GetNvmlErrorString(result)));
        }
    }

    NvAPI_Status __cdecl NvAPI_GPU_GetGPUType(NvPhysicalGpuHandle hPhysicalGpu, NV_GPU_TYPE* pGpuType) {
        constexpr auto n = __func__;

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (hPhysicalGpu == nullptr || pGpuType == nullptr)
            return InvalidArgument(n);

        auto adapter = reinterpret_cast<NvapiAdapter*>(hPhysicalGpu);
        if (!nvapiAdapterRegistry->IsAdapter(adapter))
            return ExpectedPhysicalGpuHandle(n);

        *pGpuType = adapter->GetGpuType();

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_GPU_GetPCIIdentifiers(NvPhysicalGpuHandle hPhysicalGpu, NvU32* pDeviceId, NvU32* pSubSystemId, NvU32* pRevisionId, NvU32* pExtDeviceId) {
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

        str::tonvss(szName, adapter->GetDeviceName());

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_GPU_GetBusId(NvPhysicalGpuHandle hPhysicalGpu, NvU32* pBusId) {
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

    NvAPI_Status __cdecl NvAPI_GPU_GetBusSlotId(NvPhysicalGpuHandle hPhysicalGpu, NvU32* pBusSlotId) {
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

    NvAPI_Status __cdecl NvAPI_GPU_GetBusType(NvPhysicalGpuHandle hPhysicalGpu, NV_GPU_BUS_TYPE* pBusType) {
        constexpr auto n = __func__;

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (hPhysicalGpu == nullptr || pBusType == nullptr)
            return InvalidArgument(n);

        auto adapter = reinterpret_cast<NvapiAdapter*>(hPhysicalGpu);
        if (!nvapiAdapterRegistry->IsAdapter(adapter))
            return ExpectedPhysicalGpuHandle(n);

        if (adapter->HasNvmlDevice()) {
            nvmlBusType_t busType;
            auto result = adapter->GetNvmlDeviceBusType(&busType);
            if (result == NVML_SUCCESS) {
                *pBusType = Nvml::ToNvGpuBusType(busType);
                return Ok(n);
            }
        }

        if (adapter->GetArchitectureId() >= NV_GPU_ARCHITECTURE_GM200)
            *pBusType = NVAPI_GPU_BUS_TYPE_PCI_EXPRESS; // Assume PCIe on Maxwell like generation and newer
        else
            *pBusType = NVAPI_GPU_BUS_TYPE_UNDEFINED;

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_GPU_GetPhysicalFrameBufferSize(NvPhysicalGpuHandle hPhysicalGpu, NvU32* pSize) {
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

    NvAPI_Status __cdecl NvAPI_GPU_GetAdapterIdFromPhysicalGpu(NvPhysicalGpuHandle hPhysicalGpu, void* pOSAdapterId) {
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

    NvAPI_Status __cdecl NvAPI_GPU_GetLogicalGpuInfo(NvLogicalGpuHandle hLogicalGpu, NV_LOGICAL_GPU_DATA* pLogicalGpuData) {
        constexpr auto n = __func__;

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (pLogicalGpuData == nullptr)
            return InvalidArgument(n);

        auto adapter = reinterpret_cast<NvapiAdapter*>(hLogicalGpu);
        if (!nvapiAdapterRegistry->IsAdapter(adapter))
            return ExpectedLogicalGpuHandle(n);

        if (pLogicalGpuData->version != NV_LOGICAL_GPU_DATA_VER1)
            return IncompatibleStructVersion(n);

        auto luid = adapter->GetLuid();
        if (!luid.has_value())
            return Error(n);

        memcpy(pLogicalGpuData->pOSAdapterId, &luid.value(), sizeof(luid));
        pLogicalGpuData->physicalGpuHandles[0] = reinterpret_cast<NvPhysicalGpuHandle>(adapter);
        pLogicalGpuData->physicalGpuCount = 1;

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_GPU_GetArchInfo(NvPhysicalGpuHandle hPhysicalGpu, NV_GPU_ARCH_INFO* pGpuArchInfo) {
        constexpr auto n = __func__;
        auto returnAddress = _ReturnAddress();

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (pGpuArchInfo == nullptr)
            return InvalidArgument(n);

        auto adapter = reinterpret_cast<NvapiAdapter*>(hPhysicalGpu);
        if (!nvapiAdapterRegistry->IsAdapter(adapter))
            return ExpectedPhysicalGpuHandle(n);

        if (pGpuArchInfo->version != NV_GPU_ARCH_INFO_VER_1 && pGpuArchInfo->version != NV_GPU_ARCH_INFO_VER_2)
            return IncompatibleStructVersion(n);

        if (!adapter->HasNvProprietaryDriver())
            return NvidiaDeviceNotFound(n);

        auto architectureId = adapter->GetArchitectureId();

        if (env::needsAmpereSpoofing(architectureId, returnAddress))
            architectureId = NV_GPU_ARCHITECTURE_GA100;

        if (env::needsPascalSpoofing(architectureId))
            architectureId = NV_GPU_ARCHITECTURE_GP100;

        // Assume the implementation ID from the architecture ID. No simple way
        // to do a more fine-grained query at this time. Would need wine-nvml
        // usage.
        NV_GPU_ARCH_IMPLEMENTATION_ID implementationId;
        switch (architectureId) {
            case NV_GPU_ARCHITECTURE_AD100:
                implementationId = NV_GPU_ARCH_IMPLEMENTATION_AD102;
                break;
            case NV_GPU_ARCHITECTURE_GA100:
                implementationId = NV_GPU_ARCH_IMPLEMENTATION_GA102;
                break;
            case NV_GPU_ARCHITECTURE_TU100:
                implementationId = NV_GPU_ARCH_IMPLEMENTATION_TU102;
                break;
            case NV_GPU_ARCHITECTURE_GV100:
                implementationId = NV_GPU_ARCH_IMPLEMENTATION_GV100;
                break;
            case NV_GPU_ARCHITECTURE_GP100:
                implementationId = NV_GPU_ARCH_IMPLEMENTATION_GP102;
                break;
            case NV_GPU_ARCHITECTURE_GM200:
                implementationId = NV_GPU_ARCH_IMPLEMENTATION_GM204;
                break;
            case NV_GPU_ARCHITECTURE_GK100:
                implementationId = NV_GPU_ARCH_IMPLEMENTATION_GK104;
                break;
            default:
                return Error(n);
        }

        // Assume first revision, no way to query currently.
        auto revisionId = NV_GPU_CHIP_REV_A01;

        switch (pGpuArchInfo->version) {
            case NV_GPU_ARCH_INFO_VER_1: {
                auto pGpuArchInfoV1 = reinterpret_cast<NV_GPU_ARCH_INFO_V1*>(pGpuArchInfo);
                pGpuArchInfoV1->architecture = architectureId;
                pGpuArchInfoV1->implementation = implementationId;
                pGpuArchInfoV1->revision = revisionId;
                break;
            }
            case NV_GPU_ARCH_INFO_VER_2:
                pGpuArchInfo->architecture_id = architectureId;
                pGpuArchInfo->implementation_id = implementationId;
                pGpuArchInfo->revision_id = revisionId;
                break;
            default:
                return Error(n); // Unreachable, but just to be sure
        }

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_GPU_CudaEnumComputeCapableGpus(NV_COMPUTE_GPU_TOPOLOGY* pComputeTopo) {
        constexpr auto n = __func__;

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (pComputeTopo == nullptr)
            return InvalidArgument(n);

        if (pComputeTopo->version != NV_COMPUTE_GPU_TOPOLOGY_VER && pComputeTopo->version != NV_COMPUTE_GPU_TOPOLOGY_VER1)
            return IncompatibleStructVersion(n);

        auto cudaCapableGpus = std::vector<NvPhysicalGpuHandle>(0);
        for (auto i = 0U; i < nvapiAdapterRegistry->GetAdapterCount(); i++) {
            auto adapter = nvapiAdapterRegistry->GetAdapter(i);
            if (!adapter->HasNvProprietaryDriver() || adapter->GetArchitectureId() < NV_GPU_ARCHITECTURE_GM200) // Maxwell is the oldest generation we can detect
                continue;

            cudaCapableGpus.push_back(reinterpret_cast<NvPhysicalGpuHandle>(adapter));
        }

        // Those flags match NVAPI on Windows for a normal desktop machine
        auto flags = NV_COMPUTE_GPU_TOPOLOGY_PHYSICS_CAPABLE | NV_COMPUTE_GPU_TOPOLOGY_PHYSICS_ENABLE | NV_COMPUTE_GPU_TOPOLOGY_PHYSICS_RECOMMENDED;

        switch (pComputeTopo->version) {
            case NV_COMPUTE_GPU_TOPOLOGY_VER1: {
                auto pComputeTopoV1 = reinterpret_cast<NV_COMPUTE_GPU_TOPOLOGY_V1*>(pComputeTopo);
                pComputeTopoV1->gpuCount = cudaCapableGpus.size();
                for (auto i = 0U; i < cudaCapableGpus.size(); i++) {
                    pComputeTopoV1->computeGpus[i].hPhysicalGpu = cudaCapableGpus[i];
                    pComputeTopoV1->computeGpus[i].flags = flags;
                }
                break;
            }
            case NV_COMPUTE_GPU_TOPOLOGY_VER:
                pComputeTopo->gpuCount = cudaCapableGpus.size();
                pComputeTopo->computeGpus = new NV_COMPUTE_GPU[cudaCapableGpus.size()];
                for (auto i = 0U; i < cudaCapableGpus.size(); i++) {
                    pComputeTopo->computeGpus[i].hPhysicalGpu = cudaCapableGpus[i];
                    pComputeTopo->computeGpus[i].flags = flags;
                }
                break;
            default:
                return Error(n); // Unreachable, but just to be sure
        }

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
            str::tonvss(szBiosRevision, "N/A");
            return Ok(n);
        }

        char version[NVML_DEVICE_INFOROM_VERSION_BUFFER_SIZE];
        auto result = adapter->GetNvmlDeviceVbiosVersion(version, NVML_DEVICE_INFOROM_VERSION_BUFFER_SIZE);
        switch (result) {
            case NVML_SUCCESS:
                str::tonvss(szBiosRevision, version);
                return Ok(n);
            case NVML_ERROR_FUNCTION_NOT_FOUND:
                str::tonvss(szBiosRevision, "N/A");
                return Ok(n);
            case NVML_ERROR_NOT_SUPPORTED:
                return NotSupported(n);
            case NVML_ERROR_GPU_IS_LOST:
                return HandleInvalidated(n);
            default:
                return Error(str::format(n, ": ", adapter->GetNvmlErrorString(result)));
        }
    }

    NvAPI_Status __cdecl NvAPI_GPU_GetDynamicPstatesInfoEx(NvPhysicalGpuHandle hPhysicalGpu, NV_GPU_DYNAMIC_PSTATES_INFO_EX* pDynamicPstatesInfoEx) {
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

        nvmlGpuDynamicPstatesInfo_t gpuDynamicPstatesInfo;
        auto result = adapter->GetNvmlDeviceDynamicPstatesInfo(&gpuDynamicPstatesInfo);
        switch (result) {
            case NVML_SUCCESS:
                // nvmlGpuDynamicPstatesInfo_t also has `flags` but they are reserved for future use
                pDynamicPstatesInfoEx->flags = 0;
                // order of the first four utilization domains in NVML matches NVAPI
                // to consider: should we avoid blindly copying domains 4-7 in case they don't match once introduced?
                static_assert(NVML_MAX_GPU_UTILIZATIONS == NVAPI_MAX_GPU_UTILIZATIONS);
                for (auto i = 0U; i < NVAPI_MAX_GPU_UTILIZATIONS; i++) {
                    pDynamicPstatesInfoEx->utilization[i].bIsPresent = gpuDynamicPstatesInfo.utilization[i].bIsPresent ? 1 : 0;
                    pDynamicPstatesInfoEx->utilization[i].percentage = gpuDynamicPstatesInfo.utilization[i].percentage;
                }

                return Ok(n, alreadyLoggedOk);
            case NVML_ERROR_FUNCTION_NOT_FOUND:
                // probably an older version of NVML that doesn't support nvmlDeviceGetDynamicPstatesInfo yet
                // retry with nvmlDeviceGetUtilizationRates before giving up
                break;
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

        nvmlUtilization_t utilization;
        result = adapter->GetNvmlDeviceUtilizationRates(&utilization);
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
            case NVML_ERROR_FUNCTION_NOT_FOUND:
                return NoImplementation(n, alreadyLoggedNoNvml);
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

    NvAPI_Status __cdecl NvAPI_GPU_GetThermalSettings(NvPhysicalGpuHandle hPhysicalGpu, NvU32 sensorIndex, NV_GPU_THERMAL_SETTINGS* pThermalSettings) {
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

        if (!adapter->HasNvmlDevice() && sensorIndex != 0 && sensorIndex != NVAPI_THERMAL_TARGET_ALL) {
            pThermalSettings->count = 0;
            return Ok(n, alreadyLoggedOk);
        }

        if (!adapter->HasNvml())
            return NoImplementation(n, alreadyLoggedNoNvml);

        if (!adapter->HasNvmlDevice())
            return HandleInvalidated(str::format(n, ": NVML available but current adapter is not NVML compatible"), alreadyLoggedHandleInvalidated);

        unsigned sensors;
        nvmlGpuThermalSettings_t thermalSettings;
        auto result = adapter->GetNvmlDeviceThermalSettings(sensorIndex, &thermalSettings);
        switch (result) {
            case NVML_SUCCESS:
                // both NvAPI and NVML fill $(count) sensors when sensorIndex == 15,
                // 1 sensor when 0 ≤ sensorIndex < $(count) and 0 sensors otherwise
                sensors = sensorIndex == NVAPI_THERMAL_TARGET_ALL
                    ? thermalSettings.count
                    : sensorIndex < thermalSettings.count;
                switch (pThermalSettings->version) {
                    case NV_GPU_THERMAL_SETTINGS_VER_1: {
                        auto pThermalSettingsV1 = reinterpret_cast<NV_GPU_THERMAL_SETTINGS_V1*>(pThermalSettings);
                        pThermalSettingsV1->count = thermalSettings.count;
                        for (auto i = 0U; i < sensors; i++) {
                            pThermalSettingsV1->sensor[i].controller = Nvml::ToNvThermalController(thermalSettings.sensor[i].controller);
                            pThermalSettingsV1->sensor[i].target = Nvml::ToNvThermalTarget(thermalSettings.sensor[i].target);
                            pThermalSettingsV1->sensor[i].currentTemp = static_cast<NvU32>(std::max(thermalSettings.sensor[i].currentTemp, 0));
                            pThermalSettingsV1->sensor[i].defaultMaxTemp = static_cast<NvU32>(std::max(thermalSettings.sensor[i].defaultMaxTemp, 0));
                            pThermalSettingsV1->sensor[i].defaultMinTemp = static_cast<NvU32>(std::max(thermalSettings.sensor[i].defaultMinTemp, 0));
                        }
                        break;
                    }
                    case NV_GPU_THERMAL_SETTINGS_VER_2:
                        pThermalSettings->count = thermalSettings.count;
                        for (auto i = 0U; i < sensors; i++) {
                            pThermalSettings->sensor[i].controller = Nvml::ToNvThermalController(thermalSettings.sensor[i].controller);
                            pThermalSettings->sensor[i].target = Nvml::ToNvThermalTarget(thermalSettings.sensor[i].target);
                            pThermalSettings->sensor[i].currentTemp = static_cast<NvS32>(thermalSettings.sensor[i].currentTemp);
                            pThermalSettings->sensor[i].defaultMaxTemp = static_cast<NvS32>(thermalSettings.sensor[i].defaultMaxTemp);
                            pThermalSettings->sensor[i].defaultMinTemp = static_cast<NvS32>(thermalSettings.sensor[i].defaultMinTemp);
                        }
                        break;
                    default:
                        return Error(n); // Unreachable, but just to be sure
                }
                return Ok(n, alreadyLoggedOk);
            case NVML_ERROR_FUNCTION_NOT_FOUND:
                // probably an older version of NVML that doesn't support nvmlDeviceGetThermalSettings yet
                // retry with nvmlDeviceGetTemperature before giving up
                break;
            case NVML_ERROR_INVALID_ARGUMENT:
                return InvalidArgument(n);
            case NVML_ERROR_NOT_SUPPORTED:
                switch (pThermalSettings->version) {
                    case NV_GPU_THERMAL_SETTINGS_VER_1: {
                        auto pThermalSettingsV1 = reinterpret_cast<NV_GPU_THERMAL_SETTINGS_V1*>(pThermalSettings);
                        pThermalSettingsV1->count = 0;
                        break;
                    }
                    case NV_GPU_THERMAL_SETTINGS_VER_2:
                        pThermalSettings->count = 0;
                        break;
                    default:
                        return Error(n); // Unreachable, but just to be sure
                }
                return NotSupported(n);
            case NVML_ERROR_GPU_IS_LOST:
                return HandleInvalidated(n);
            default:
                return Error(str::format(n, ": ", adapter->GetNvmlErrorString(result)));
        }

        if (sensorIndex != 0 && sensorIndex != NVAPI_THERMAL_TARGET_ALL) {
            pThermalSettings->count = 0;
            return Ok(n, alreadyLoggedOk);
        }

        unsigned int temp;
        result = adapter->GetNvmlDeviceTemperature(NVML_TEMPERATURE_GPU, &temp);
        switch (result) {
            case NVML_SUCCESS:
                switch (pThermalSettings->version) {
                    case NV_GPU_THERMAL_SETTINGS_VER_1: {
                        auto pThermalSettingsV1 = reinterpret_cast<NV_GPU_THERMAL_SETTINGS_V1*>(pThermalSettings);
                        pThermalSettingsV1->count = 1;
                        pThermalSettingsV1->sensor[0].controller = NVAPI_THERMAL_CONTROLLER_UNKNOWN;
                        pThermalSettingsV1->sensor[0].target = NVAPI_THERMAL_TARGET_GPU;
                        pThermalSettingsV1->sensor[0].currentTemp = temp;
                        pThermalSettingsV1->sensor[0].defaultMaxTemp = 127;
                        pThermalSettingsV1->sensor[0].defaultMinTemp = 0;
                        break;
                    }
                    case NV_GPU_THERMAL_SETTINGS_VER_2:
                        pThermalSettings->count = 1;
                        pThermalSettings->sensor[0].controller = NVAPI_THERMAL_CONTROLLER_UNKNOWN;
                        pThermalSettings->sensor[0].target = NVAPI_THERMAL_TARGET_GPU;
                        pThermalSettings->sensor[0].currentTemp = static_cast<NvS32>(temp);
                        pThermalSettings->sensor[0].defaultMaxTemp = 127;
                        pThermalSettings->sensor[0].defaultMinTemp = -256;
                        break;
                    default:
                        return Error(n); // Unreachable, but just to be sure
                }
                return Ok(n, alreadyLoggedOk);
            case NVML_ERROR_FUNCTION_NOT_FOUND:
                return NoImplementation(n, alreadyLoggedNoNvml);
            case NVML_ERROR_NOT_SUPPORTED:
                switch (pThermalSettings->version) {
                    case NV_GPU_THERMAL_SETTINGS_VER_1: {
                        auto pThermalSettingsV1 = reinterpret_cast<NV_GPU_THERMAL_SETTINGS_V1*>(pThermalSettings);
                        pThermalSettingsV1->count = 0;
                        break;
                    }
                    case NV_GPU_THERMAL_SETTINGS_VER_2:
                        pThermalSettings->count = 0;
                        break;
                    default:
                        return Error(n); // Unreachable, but just to be sure
                }
                return NotSupported(n);
            case NVML_ERROR_GPU_IS_LOST:
                return HandleInvalidated(n);
            default:
                return Error(str::format(n, ": ", adapter->GetNvmlErrorString(result)));
        }
    }

    NvAPI_Status __cdecl NvAPI_GPU_GetCurrentPstate(NvPhysicalGpuHandle hPhysicalGpu, NV_GPU_PERF_PSTATE_ID* pCurrentPstate) {
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
        auto result = adapter->GetNvmlDevicePerformanceState(&pState);
        switch (result) {
            case NVML_SUCCESS:
                *pCurrentPstate = static_cast<NV_GPU_PERF_PSTATE_ID>(pState);
                return Ok(n, alreadyLoggedOk);
            case NVML_ERROR_FUNCTION_NOT_FOUND:
                return NoImplementation(n, alreadyLoggedNoNvml);
            case NVML_ERROR_NOT_SUPPORTED:
                return NotSupported(n);
            case NVML_ERROR_GPU_IS_LOST:
                return HandleInvalidated(n);
            default:
                return Error(str::format(n, ": ", adapter->GetNvmlErrorString(result)));
        }
    }

    NvAPI_Status __cdecl NvAPI_GPU_GetAllClockFrequencies(NvPhysicalGpuHandle hPhysicalGpu, NV_GPU_CLOCK_FREQUENCIES* pClkFreqs) {
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

        // Only check for CURRENT_FREQ, and not for the other types i.e. BOOST or DEFAULT for now
        if ((pClkFreqs->version == NV_GPU_CLOCK_FREQUENCIES_VER_2 || pClkFreqs->version == NV_GPU_CLOCK_FREQUENCIES_VER_3)
            && pClkFreqs->ClockType != static_cast<NvU32>(NV_GPU_CLOCK_FREQUENCIES_CURRENT_FREQ))
            return NotSupported(n, alreadyLoggedNotSupported);

        auto adapter = reinterpret_cast<NvapiAdapter*>(hPhysicalGpu);
        if (!nvapiAdapterRegistry->IsAdapter(adapter))
            return ExpectedPhysicalGpuHandle(n);

        if (!adapter->HasNvml())
            return NoImplementation(n, alreadyLoggedNoNvml);

        if (!adapter->HasNvmlDevice())
            return HandleInvalidated(str::format(n, ": NVML available but current adapter is not NVML compatible"), alreadyLoggedHandleInvalidated);

        // Reset all clock data for all domains
        for (auto& domain : pClkFreqs->domain) {
            domain.bIsPresent = 0;
            domain.frequency = 0;
        }

        unsigned int clock;
        // Seemingly we need to do nvml call on a "per clock unit" to get the clock
        // Set the availability of the clock to TRUE and the nvml read clock in the nvapi struct
        auto resultGpu = adapter->GetNvmlDeviceClockInfo(NVML_CLOCK_GRAPHICS, &clock);
        switch (resultGpu) {
            case NVML_SUCCESS:
                switch (pClkFreqs->version) {
                    case NV_GPU_CLOCK_FREQUENCIES_VER_1: {
                        auto pClkFreqsV1 = reinterpret_cast<NV_GPU_CLOCK_FREQUENCIES_V1*>(pClkFreqs);
                        pClkFreqsV1->domain[NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS].bIsPresent = 1;
                        pClkFreqsV1->domain[NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS].frequency = (clock * 1000);
                        break;
                    }
                    case NV_GPU_CLOCK_FREQUENCIES_VER_2:
                    case NV_GPU_CLOCK_FREQUENCIES_VER_3:
                        pClkFreqs->domain[NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS].bIsPresent = 1;
                        pClkFreqs->domain[NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS].frequency = (clock * 1000);
                        break;
                    default:
                        return Error(n); // Unreachable, but just to be sure
                }
                break;
            case NVML_ERROR_FUNCTION_NOT_FOUND:
                return NoImplementation(n, alreadyLoggedNoNvml);
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
                switch (pClkFreqs->version) {
                    case NV_GPU_CLOCK_FREQUENCIES_VER_1: {
                        auto pClkFreqsV1 = reinterpret_cast<NV_GPU_CLOCK_FREQUENCIES_V1*>(pClkFreqs);
                        pClkFreqsV1->domain[NVAPI_GPU_PUBLIC_CLOCK_MEMORY].bIsPresent = 1;
                        pClkFreqsV1->domain[NVAPI_GPU_PUBLIC_CLOCK_MEMORY].frequency = (clock * 1000);
                        break;
                    }
                    case NV_GPU_CLOCK_FREQUENCIES_VER_2:
                    case NV_GPU_CLOCK_FREQUENCIES_VER_3:
                        pClkFreqs->domain[NVAPI_GPU_PUBLIC_CLOCK_MEMORY].bIsPresent = 1;
                        pClkFreqs->domain[NVAPI_GPU_PUBLIC_CLOCK_MEMORY].frequency = (clock * 1000);
                        break;
                    default:
                        return Error(n); // Unreachable, but just to be sure
                }
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
                switch (pClkFreqs->version) {
                    case NV_GPU_CLOCK_FREQUENCIES_VER_1: {
                        auto pClkFreqsV1 = reinterpret_cast<NV_GPU_CLOCK_FREQUENCIES_V1*>(pClkFreqs);
                        pClkFreqsV1->domain[NVAPI_GPU_PUBLIC_CLOCK_VIDEO].bIsPresent = 1;
                        pClkFreqsV1->domain[NVAPI_GPU_PUBLIC_CLOCK_VIDEO].frequency = (clock * 1000);
                        break;
                    }
                    case NV_GPU_CLOCK_FREQUENCIES_VER_2:
                    case NV_GPU_CLOCK_FREQUENCIES_VER_3:
                        pClkFreqs->domain[NVAPI_GPU_PUBLIC_CLOCK_VIDEO].bIsPresent = 1;
                        pClkFreqs->domain[NVAPI_GPU_PUBLIC_CLOCK_VIDEO].frequency = (clock * 1000);
                        break;
                    default:
                        return Error(n); // Unreachable, but just to be sure
                }
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

    NvAPI_Status __cdecl NvAPI_GPU_GetPstates20(NvPhysicalGpuHandle hPhysicalGpu, NV_GPU_PERF_PSTATES20_INFO* pPstatesInfo) {
        return NoImplementation(__func__);
    }
}
