extern "C" {
    using namespace dxvk;

    NvAPI_Status __cdecl NvAPI_D3D_GetObjectHandleForResource(IUnknown* pDevice, IUnknown* pResource, NVDX_ObjectHandle* pHandle) {
        static bool alreadyLogged = false;
        if (!alreadyLogged) {
            alreadyLogged = true;
            return NoImplementation("NvAPI_D3D_GetObjectHandleForResource");
        }

        return NoImplementation();
    }

    NvAPI_Status __cdecl NvAPI_D3D_SetResourceHint(IUnknown *pDev, NVDX_ObjectHandle obj, NVAPI_D3D_SETRESOURCEHINT_CATEGORY dwHintCategory, NvU32 dwHintName, NvU32 *pdwHintValue) {
        static bool alreadyLogged = false;
        if (!alreadyLogged) {
            alreadyLogged = true;
            return NoImplementation("NvAPI_D3D_SetResourceHint");
        }
        
        return NoImplementation();
    }

    NvAPI_Status __cdecl NvAPI_D3D_GetCurrentSLIState(IUnknown* pDevice, NV_GET_CURRENT_SLI_STATE* pSliState) {
        if (pDevice == nullptr || pSliState == nullptr)
            return InvalidArgument("NvAPI_D3D_GetCurrentSLIState");

        if (pSliState->version != NV_GET_CURRENT_SLI_STATE_VER1 && pSliState->version != NV_GET_CURRENT_SLI_STATE_VER2)
            return IncompatibleStructVersion("NvAPI_D3D_GetCurrentSLIState");

        // Report that SLI is not enabled
        pSliState->maxNumAFRGroups = 1;
        pSliState->numAFRGroups = 1;
        pSliState->currentAFRIndex = 0;
        pSliState->nextFrameAFRIndex = 0;
        pSliState->previousFrameAFRIndex = 0;
        pSliState->bIsCurAFRGroupNew = false;

        if (pSliState->version == NV_GET_CURRENT_SLI_STATE_VER2)
            pSliState->numVRSLIGpus = 0;

        return Ok("NvAPI_D3D_GetCurrentSLIState");
    }
}
