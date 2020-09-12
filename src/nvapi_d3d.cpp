extern "C" {
    using namespace dxvk;

    NvAPI_Status __cdecl NvAPI_D3D_GetObjectHandleForResource(IUnknown* pDevice, IUnknown* pResource, NVDX_ObjectHandle* pHandle) {
        static bool alreadyLogged = false;
        if (!alreadyLogged) {
            alreadyLogged = true;
            return NoImplementation("NvAPI_D3D_GetObjectHandleForResource");
        }

        return NVAPI_NO_IMPLEMENTATION;
    }

    NvAPI_Status __cdecl NvAPI_D3D_SetResourceHint(IUnknown *pDev, NVDX_ObjectHandle obj, NVAPI_D3D_SETRESOURCEHINT_CATEGORY dwHintCategory, NvU32 dwHintName, NvU32 *pdwHintValue) {
        static bool alreadyLogged = false;
        if (!alreadyLogged) {
            alreadyLogged = true;
            return NoImplementation("NvAPI_D3D_SetResourceHint");
        }
        
        return NVAPI_NO_IMPLEMENTATION;
    }

    NvAPI_Status __cdecl NvAPI_D3D_GetCurrentSLIState(IUnknown* pDevice, NV_GET_CURRENT_SLI_STATE* pSliState) {
        return NoImplementation("NvAPI_D3D_GetCurrentSLIState");
    }
}
