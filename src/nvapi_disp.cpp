#include "nvapi_private.h"
#include "nvapi_static.h"
#include "util/util_statuscode.h"
#include "util/util_string.h"

extern "C" {
    using namespace dxvk;

    NvAPI_Status __cdecl NvAPI_Disp_GetHdrCapabilities(NvU32 displayId, NV_HDR_CAPABILITIES *pHdrCapabilities) {
        constexpr auto n = __func__;

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (pHdrCapabilities == nullptr)
            return InvalidArgument(n);

        if (pHdrCapabilities->version != NV_HDR_CAPABILITIES_VER1 && pHdrCapabilities->version != NV_HDR_CAPABILITIES_VER2)
            return IncompatibleStructVersion(n);

        // Report that HDR is not available
        pHdrCapabilities->isST2084EotfSupported = false;
        pHdrCapabilities->isTraditionalHdrGammaSupported = false;
        pHdrCapabilities->isEdrSupported = false;
        pHdrCapabilities->driverExpandDefaultHdrParameters = false;
        pHdrCapabilities->isTraditionalSdrGammaSupported = false;
        // pHdrCapabilities->display_data

        if (pHdrCapabilities->version == NV_HDR_CAPABILITIES_VER2)
            pHdrCapabilities->isDolbyVisionSupported = false;
            // pHdrCapabilities->dv_static_metadata

        return Ok(str::format(n, " ", displayId));
    }

    NvAPI_Status __cdecl NvAPI_DISP_GetDisplayIdByDisplayName(const char *displayName, NvU32 *displayId) {
        constexpr auto n = __func__;

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (displayName == nullptr || displayId == nullptr)
            return InvalidArgument(n);

        auto id = nvapiAdapterRegistry->GetOutputId(std::string(displayName));
        if (id == -1)
            return InvalidArgument(str::format(n, " ", displayName));

        *displayId = id;

        return Ok(str::format(n, " ", displayName));
    }

    NvAPI_Status __cdecl NvAPI_DISP_GetGDIPrimaryDisplayId(NvU32 *displayId) {
        constexpr auto n = __func__;

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (displayId == nullptr)
            return InvalidArgument(n);

        auto id = nvapiAdapterRegistry->GetPrimaryOutputId();
        if (id == -1)
            return NvidiaDeviceNotFound(n);

        *displayId = id;

        return Ok(n);
    }
}
