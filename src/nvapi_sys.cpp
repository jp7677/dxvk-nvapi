#include "nvapi_private.h"
#include "nvapi_static.h"
#include "util/util_statuscode.h"
#include "../version.h"

extern "C" {
    using namespace dxvk;

    NvAPI_Status __cdecl NvAPI_SYS_GetPhysicalGpuFromDisplayId(NvU32 displayId, NvPhysicalGpuHandle *hPhysicalGpu) {
        constexpr auto n = "NvAPI_SYS_GetPhysicalGpuFromDisplayId";

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        auto output = nvapiAdapterRegistry->GetOutput(displayId);
        if (output == nullptr)
            return InvalidArgument(n);

        *hPhysicalGpu = (NvPhysicalGpuHandle) output->GetParent();

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_SYS_GetDriverAndBranchVersion(NvU32* pDriverVersion, NvAPI_ShortString szBuildBranchString) {
        constexpr auto n = "NvAPI_SYS_GetDriverAndBranchVersion";

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (pDriverVersion == nullptr || szBuildBranchString == nullptr)
            return InvalidArgument(n);

        *pDriverVersion = nvapiAdapterRegistry->GetAdapter()->GetDriverVersion();
        strcpy(szBuildBranchString, DXVK_NVAPI_VERSION);

        return Ok(n);
    }
}
