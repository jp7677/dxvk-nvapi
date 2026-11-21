#include "nvapi_private.h"
#include "nvapi_globals.h"
#include "util/util_statuscode.h"
#include "../version.h"

using namespace dxvk;

NVAPI_FUNCTION NvAPI_SYS_GetPhysicalGpuFromDisplayId(NvU32 displayId, NvPhysicalGpuHandle* hPhysicalGpu) {
    static constexpr auto n = FUNC;

    if (log::tracing())
        log::trace(n, displayId, log::fmt::ptr(hPhysicalGpu));

    if (!nvapiAdapterRegistry)
        return ApiNotInitialized(n);

    if (!hPhysicalGpu)
        return InvalidArgument(n);

    auto output = nvapiAdapterRegistry->FindOutput(displayId);
    if (!output)
        return InvalidArgument(n);

    *hPhysicalGpu = reinterpret_cast<NvPhysicalGpuHandle>(output->GetParent());

    return Ok(n);
}

NVAPI_FUNCTION NvAPI_SYS_GetDriverAndBranchVersion(NvU32* pDriverVersion, NvAPI_ShortString szBuildBranchString) {
    static constexpr auto n = FUNC;

    if (log::tracing())
        log::trace(n, log::fmt::ptr(pDriverVersion), log::fmt::ptr(szBuildBranchString));

    if (!nvapiAdapterRegistry)
        return ApiNotInitialized(n);

    if (!pDriverVersion || !szBuildBranchString)
        return InvalidArgument(n);

    *pDriverVersion = nvapiAdapterRegistry->GetFirstAdapter()->GetDriverVersion();
    str::tonvss(szBuildBranchString, str::format("r", NVAPI_SDK_VERSION, "_", DXVK_NVAPI_VERSION));

    return Ok(n);
}

NVAPI_FUNCTION NvAPI_SYS_GetDisplayDriverInfo(NV_DISPLAY_DRIVER_INFO* pDriverInfo) {
    static constexpr auto n = FUNC;

    if (log::tracing())
        log::trace(n, log::fmt::ptr(pDriverInfo));

    if (!nvapiAdapterRegistry)
        return ApiNotInitialized(n);

    if (!pDriverInfo)
        return InvalidArgument(n);

    if (pDriverInfo->version != NV_DISPLAY_DRIVER_INFO_VER1 && pDriverInfo->version != NV_DISPLAY_DRIVER_INFO_VER2)
        return IncompatibleStructVersion(n, pDriverInfo->version);

    switch (pDriverInfo->version) {
        case NV_DISPLAY_DRIVER_INFO_VER2:
            str::tonvss(pDriverInfo->szBuildBaseBranch, str::format("r", NVAPI_SDK_VERSION));
            [[fallthrough]];
        case NV_DISPLAY_DRIVER_INFO_VER1:
            pDriverInfo->driverVersion = nvapiAdapterRegistry->GetFirstAdapter()->GetDriverVersion();
            str::tonvss(pDriverInfo->szBuildBranch, str::format("r", NVAPI_SDK_VERSION, "_", DXVK_NVAPI_VERSION));
            pDriverInfo->bIsDCHDriver = 1;              // Assume DCH driver for Windows
            pDriverInfo->bIsNVIDIAStudioPackage = 0;    // Let's not support "Studio Package"
            pDriverInfo->bIsNVIDIAGameReadyPackage = 1; // GameReady Package should be "safe" even if other packages is used
            pDriverInfo->bIsNVIDIARTXProductionBranchPackage = 0;
            pDriverInfo->bIsNVIDIARTXNewFeatureBranchPackage = 0;
            break;
        default:
            return Error(n); // Unreachable, but just to be sure
    }

    return Ok(n);
}

NVAPI_FUNCTION NvAPI_SYS_GetPhysicalGPUs(NV_PHYSICAL_GPUS* pPhysicalGPUs) {
    static constexpr auto n = FUNC;

    if (log::tracing())
        log::trace(n, log::fmt::ptr(pPhysicalGPUs));

    if (!nvapiAdapterRegistry)
        return ApiNotInitialized(n);

    if (!pPhysicalGPUs)
        return InvalidArgument(n);

    if (pPhysicalGPUs->version != NV_PHYSICAL_GPUS_VER1)
        return IncompatibleStructVersion(n, pPhysicalGPUs->version);

    for (auto i = 0U; i < nvapiAdapterRegistry->GetAdapterCount(); i++) {
        pPhysicalGPUs->gpuHandleData[i].hPhysicalGpu = reinterpret_cast<NvPhysicalGpuHandle>(nvapiAdapterRegistry->GetAdapter(i));
        pPhysicalGPUs->gpuHandleData[i].adapterType = NV_ADAPTER_TYPE_WDDM;
    }

    pPhysicalGPUs->gpuHandleCount = nvapiAdapterRegistry->GetAdapterCount();

    return Ok(n);
}

NVAPI_FUNCTION NvAPI_SYS_GetLogicalGPUs(NV_LOGICAL_GPUS* pLogicalGPUs) {
    static constexpr auto n = FUNC;

    if (log::tracing())
        log::trace(n, log::fmt::ptr(pLogicalGPUs));

    if (!nvapiAdapterRegistry)
        return ApiNotInitialized(n);

    if (!pLogicalGPUs)
        return InvalidArgument(n);

    if (pLogicalGPUs->version != NV_LOGICAL_GPUS_VER1)
        return IncompatibleStructVersion(n, pLogicalGPUs->version);

    for (auto i = 0U; i < nvapiAdapterRegistry->GetAdapterCount(); i++) {
        pLogicalGPUs->gpuHandleData[i].hLogicalGpu = reinterpret_cast<NvLogicalGpuHandle>(nvapiAdapterRegistry->GetAdapter(i));
        pLogicalGPUs->gpuHandleData[i].adapterType = NV_ADAPTER_TYPE_WDDM;
    }

    pLogicalGPUs->gpuHandleCount = nvapiAdapterRegistry->GetAdapterCount();

    return Ok(n);
}
