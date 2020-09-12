#include "nvapi_private.h"
#include "impl/nvapi_adapter_registry.h"
#include "../version.h"

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // __GNUC__

extern "C" {
    using namespace dxvk;

    NvAPI_Status __cdecl NvAPI_SYS_GetDriverAndBranchVersion(NvU32* pDriverVersion, NvAPI_ShortString szBuildBranchString) {
        *pDriverVersion = nvapiAdapterRegistry->GetAdapter()->GetDriverVersion();
        strcpy(szBuildBranchString, DXVK_NVAPI_VERSION);

        std::cerr << "NvAPI_SYS_GetDriverAndBranchVersion: OK" << std::endl;
        return NVAPI_OK;
    }
}
