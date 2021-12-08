#include "nvapi_private.h"
#include "util/util_statuscode.h"

extern "C" {
    using namespace dxvk;

    NvAPI_Status __cdecl NvAPI_D3D_GetObjectHandleForResource(IUnknown *pDevice, IUnknown *pResource, NVDX_ObjectHandle *pHandle) {
        static bool alreadyLogged = false;
        // Fake-implement with a dumb passthrough, though no other NvAPI entry points
        // we're likely to implement should care about the actual handle value.
        *pHandle = (NVDX_ObjectHandle)pResource;
        return Ok(__func__, alreadyLogged);
    }

    NvAPI_Status __cdecl NvAPI_D3D_SetResourceHint(IUnknown *pDev, NVDX_ObjectHandle obj, NVAPI_D3D_SETRESOURCEHINT_CATEGORY dwHintCategory, NvU32 dwHintName, NvU32 *pdwHintValue) {
        static bool alreadyLogged = false;
        return NoImplementation(__func__, alreadyLogged);
    }

    NvAPI_Status __cdecl NvAPI_D3D_BeginResourceRendering(IUnknown *pDeviceOrContext, NVDX_ObjectHandle obj, NvU32 Flags) {
        static bool alreadyLogged = false;
        // Synchronisation hints for SLI...
        return Ok(__func__, alreadyLogged);
    }

    NvAPI_Status __cdecl NvAPI_D3D_EndResourceRendering(IUnknown *pDeviceOrContext, NVDX_ObjectHandle obj, NvU32 Flags) {
        static bool alreadyLogged = false;
        return Ok(__func__, alreadyLogged);
    }

    NvAPI_Status __cdecl NvAPI_D3D_GetCurrentSLIState(IUnknown *pDevice, NV_GET_CURRENT_SLI_STATE *pSliState) {
        constexpr auto n = __func__;
        static bool alreadyLoggedOk = false;

        if (pDevice == nullptr || pSliState == nullptr)
            return InvalidArgument(n);

        if (pSliState->version != NV_GET_CURRENT_SLI_STATE_VER1 && pSliState->version != NV_GET_CURRENT_SLI_STATE_VER2)
            return IncompatibleStructVersion(n);

        // Report that SLI is not available
        pSliState->maxNumAFRGroups = 1;
        pSliState->numAFRGroups = 1;
        pSliState->currentAFRIndex = 0;
        pSliState->nextFrameAFRIndex = 0;
        pSliState->previousFrameAFRIndex = 0;
        pSliState->bIsCurAFRGroupNew = false;

        if (pSliState->version == NV_GET_CURRENT_SLI_STATE_VER2)
            pSliState->numVRSLIGpus = 0;

        return Ok(n, alreadyLoggedOk);
    }

    NvAPI_Status __cdecl NvAPI_D3D_ImplicitSLIControl(IMPLICIT_SLI_CONTROL implicitSLIControl) {
        return Ok(__func__);
    }

    NvAPI_Status __cdecl NvAPI_D3D1x_GetGraphicsCapabilities(IUnknown *pDevice,
                                                    NvU32 structVersion,
                                                    NV_D3D1x_GRAPHICS_CAPS *pGraphicsCaps) {
        constexpr auto n = __func__;
        static bool alreadyLoggedOk = false;

        switch(structVersion) {
            case NV_D3D1x_GRAPHICS_CAPS_VER1:
                memset(pGraphicsCaps, 0, sizeof(NV_D3D1x_GRAPHICS_CAPS_V1));
                break;
            case NV_D3D1x_GRAPHICS_CAPS_VER2:
                memset(pGraphicsCaps, 0, sizeof(NV_D3D1x_GRAPHICS_CAPS_V2));
                break;
            default:
                return IncompatibleStructVersion(n);
        }

        switch(structVersion) {
            case NV_D3D1x_GRAPHICS_CAPS_VER2:
                // bFastUAVClearSupported is reported mostly for the sake of DLSS.
                // All NVIDIA Vulkan drivers support this.
                (*reinterpret_cast<NV_D3D1x_GRAPHICS_CAPS_V2*>(pGraphicsCaps)).bFastUAVClearSupported = 1;
                // dummy SM version number (unused by DLSS):
                (*reinterpret_cast<NV_D3D1x_GRAPHICS_CAPS_V2*>(pGraphicsCaps)).majorSMVersion = 0;
                (*reinterpret_cast<NV_D3D1x_GRAPHICS_CAPS_V2*>(pGraphicsCaps)).minorSMVersion = 0;
            
                [[fallthrough]];
            
            case NV_D3D1x_GRAPHICS_CAPS_VER1:
                (*reinterpret_cast<NV_D3D1x_GRAPHICS_CAPS_V1*>(pGraphicsCaps)).bExclusiveScissorRectsSupported = 0;
                (*reinterpret_cast<NV_D3D1x_GRAPHICS_CAPS_V1*>(pGraphicsCaps)).bVariablePixelRateShadingSupported = 0;
        }

        return Ok(n, alreadyLoggedOk);
    }
}
