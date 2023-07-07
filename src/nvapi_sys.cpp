#include "nvapi_private.h"
#include "nvapi_globals.h"
#include "util/util_statuscode.h"
#include "../version.h"

extern "C" {
    using namespace dxvk;

    NvAPI_Status __cdecl NvAPI_SYS_GetPhysicalGpuFromDisplayId(NvU32 displayId, NvPhysicalGpuHandle* hPhysicalGpu) {
        constexpr auto n = __func__;

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        auto output = nvapiAdapterRegistry->FindOutput(displayId);
        if (output == nullptr)
            return InvalidArgument(n);

        *hPhysicalGpu = reinterpret_cast<NvPhysicalGpuHandle>(output->GetParent());

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_SYS_GetDriverAndBranchVersion(NvU32* pDriverVersion, NvAPI_ShortString szBuildBranchString) {
        constexpr auto n = __func__;

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (pDriverVersion == nullptr || szBuildBranchString == nullptr)
            return InvalidArgument(n);

        *pDriverVersion = nvapiAdapterRegistry->GetFirstAdapter()->GetDriverVersion();
        str::tonvss(szBuildBranchString, str::format(NVAPI_VERSION, "_", DXVK_NVAPI_VERSION));

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_SYS_GetDisplayDriverInfo(NV_DISPLAY_DRIVER_INFO *pDriverInfo) {
        constexpr auto n = __func__;

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (pDriverInfo == nullptr)
            return InvalidArgument(n);

        switch (pDriverInfo->version) {
            case NV_DISPLAY_DRIVER_INFO_VER1: {
                pDriverInfo->driverVersion = nvapiAdapterRegistry->GetFirstAdapter()->GetDriverVersion();
                str::tonvss(pDriverInfo->szBuildBranch, str::format(NVAPI_VERSION, "_", DXVK_NVAPI_VERSION));
                pDriverInfo->bIsDCHDriver = 1; // Assume DHC driver for Windows
                pDriverInfo->bIsNVIDIAStudioPackage = 0; // Lets not support "Studio Package"
                pDriverInfo->bIsNVIDIAGameReadyPackage = 1; // GameReady Package should be "safe" even if other packages is used
                pDriverInfo->bIsNVIDIARTXProductionBranchPackage = 0;
                pDriverInfo->bIsNVIDIARTXNewFeatureBranchPackage = 0;
                break;
            }
            case NV_DISPLAY_DRIVER_INFO_VER2: {
                pDriverInfo->driverVersion = nvapiAdapterRegistry->GetFirstAdapter()->GetDriverVersion();
                str::tonvss(pDriverInfo->szBuildBranch, str::format(NVAPI_VERSION, "_", DXVK_NVAPI_VERSION));
                pDriverInfo->bIsDCHDriver = 1; // Assume DHC driver for Windows
                pDriverInfo->bIsNVIDIAStudioPackage = 0; // Lets not support "Studio Package"
                pDriverInfo->bIsNVIDIAGameReadyPackage = 1; // GameReady Package should be "safe" even if other packages is used
                pDriverInfo->bIsNVIDIARTXProductionBranchPackage = 0;
                pDriverInfo->bIsNVIDIARTXNewFeatureBranchPackage = 0;
                str::tonvss(pDriverInfo->szBuildBaseBranch, str::format(NVAPI_VERSION, "_", DXVK_NVAPI_VERSION));
                break;
            }
            default:
                return Error(n); // Unreachable, but just to be sure
        }

        return Ok(n);
    }
}
