#include "nvapi_private.h"
#include "impl/nvapi_adapter_registry.h"
#include "../version.h"

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // __GNUC__

extern "C" {
    using namespace dxvk;

    NvAPI_Status __cdecl NvAPI_Disp_GetHdrCapabilities(NvU32 displayId, NV_HDR_CAPABILITIES *pHdrCapabilities) {
        std::cerr << "NvAPI_Disp_GetHdrCapabilities " << displayId << ": No implementation" << std::endl;
        return NVAPI_NO_IMPLEMENTATION;
    }

    NvAPI_Status __cdecl NvAPI_DISP_GetDisplayIdByDisplayName(const char *displayName, NvU32* displayId) {
        auto id = nvapiAdapterRegistry->GetOutputId(std::string(displayName));
        if (id == -1) {
            std::cerr << "NvAPI_DISP_GetDisplayIdByDisplayName " << displayName << ": NVAPI_INVALID_ARGUMENT" << std::endl;
            return NVAPI_INVALID_ARGUMENT;
        }

        *displayId = id;

        std::cerr << "NvAPI_DISP_GetDisplayIdByDisplayName " << displayName << ": OK" << std::endl;
        return NVAPI_OK;
    }
}
