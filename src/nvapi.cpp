#include "nvapi_private.h"
#include "nvapi_adapter.h"

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // __GNUC__

extern "C" {
    using namespace dxvk;

    NvapiAdapter* nvapiAdapter;

    NvAPI_Status __cdecl NvAPI_D3D11_SetDepthBoundsTest(IUnknown* pDeviceOrContext, NvU32 bEnable, float fMinDepth, float fMaxDepth) {
        static bool alreadyTested = false;
        if (!alreadyTested) {
            alreadyTested = true;
            Com<ID3D11VkExtDevice> dxvkDevice;
            if (FAILED(pDeviceOrContext->QueryInterface(IID_PPV_ARGS(&dxvkDevice))))
                return NVAPI_ERROR;

            if (!dxvkDevice->GetExtensionSupport(D3D11_VK_EXT_DEPTH_BOUNDS))
                return NVAPI_ERROR;
        }

        Com<ID3D11Device> d3d11Device;
        if (FAILED(pDeviceOrContext->QueryInterface(IID_PPV_ARGS(&d3d11Device))))
            return NVAPI_ERROR;

        Com<ID3D11DeviceContext> d3d11DeviceContext;
        d3d11Device->GetImmediateContext(&d3d11DeviceContext);

        Com<ID3D11VkExtContext> dxvkDeviceContext;
        if (FAILED(d3d11DeviceContext->QueryInterface(IID_PPV_ARGS(&dxvkDeviceContext))))
            return NVAPI_ERROR;

        dxvkDeviceContext->SetDepthBoundsTest(bEnable, fMinDepth, fMaxDepth);

        static bool alreadyLogged = false;
        if (!alreadyLogged) {
            alreadyLogged = true;
            std::cerr << "NvAPI_D3D11_SetDepthBoundsTest: Succeeded" << std::endl;
        }

        return NVAPI_OK;
    }

    NvAPI_Status __cdecl NvAPI_D3D11_IsNvShaderExtnOpCodeSupported(IUnknown* pDeviceOrContext, NvU32 code, bool* supported) {
        *supported = false;

        std::cerr << "NvAPI_D3D11_IsNvShaderExtnOpCodeSupported " << std::hex << code << ": OK" << std::endl;
        return NVAPI_OK;
    }

    NvAPI_Status __cdecl NvAPI_D3D_GetObjectHandleForResource(IUnknown* pDevice, IUnknown* pResource, NVDX_ObjectHandle* pHandle) {
        static bool alreadyLogged = false;
        if (!alreadyLogged) {
            alreadyLogged = true;
            std::cerr << "NvAPI_D3D_GetObjectHandleForResource: No implementation" << std::endl;
        }

        return NVAPI_NO_IMPLEMENTATION;
    }

    NvAPI_Status __cdecl NvAPI_D3D_SetResourceHint(IUnknown *pDev, NVDX_ObjectHandle obj, NVAPI_D3D_SETRESOURCEHINT_CATEGORY dwHintCategory, NvU32 dwHintName, NvU32 *pdwHintValue) {
        static bool alreadyLogged = false;
        if (!alreadyLogged) {
            alreadyLogged = true;
            std::cerr << "NvAPI_D3D_SetResourceHint: No implementation" << std::endl;
        }
        
        return NVAPI_NO_IMPLEMENTATION;
    }

    NvAPI_Status __cdecl NvAPI_D3D_GetCurrentSLIState(IUnknown* pDevice, NV_GET_CURRENT_SLI_STATE* pSliState) {
        std::cerr << "NvAPI_D3D_GetCurrentSLIState: No implementation" << std::endl;
        return NVAPI_NO_IMPLEMENTATION;
    }

    NvAPI_Status __cdecl NvAPI_DISP_GetDisplayIdByDisplayName(const char* displayName, NvU32* displayId) {
        std::cerr << "NvAPI_DISP_GetDisplayIdByDisplayName: No implementation" << std::endl;
        return NVAPI_NO_IMPLEMENTATION;
    }

    NvAPI_Status __cdecl NvAPI_GPU_GetGPUType(NvPhysicalGpuHandle hPhysicalGpu, NV_GPU_TYPE *pGpuType) {
        *pGpuType = (NV_GPU_TYPE)nvapiAdapter->GetGpuType();

        std::cerr << "NvAPI_GPU_GetGPUType: OK" << std::endl;
        return NVAPI_OK;
    }

    NvAPI_Status __cdecl NvAPI_GPU_GetPCIIdentifiers(NvPhysicalGpuHandle hPhysicalGpu, NvU32 *pDeviceId, NvU32 *pSubSystemId, NvU32 *pRevisionId, NvU32 *pExtDeviceId) {
        *pDeviceId = nvapiAdapter->GetDeviceId();
        *pSubSystemId = 0;
        *pRevisionId = 0;
        *pExtDeviceId = nvapiAdapter->GetDeviceId();

        std::cerr << "NvAPI_GPU_GetPCIIdentifiers: OK" << std::endl;
        return NVAPI_OK;
    }

    NvAPI_Status __cdecl NvAPI_GPU_GetFullName(NvPhysicalGpuHandle hPhysicalGpu, NvAPI_ShortString szName) {
        strcpy(szName, nvapiAdapter->GetDeviceName().c_str());

        std::cerr << "NvAPI_GPU_GetFullName: OK" << std::endl;
        return NVAPI_OK;
    }

    NvAPI_Status __cdecl NvAPI_SYS_GetDriverAndBranchVersion(NvU32* pDriverVersion, NvAPI_ShortString szBuildBranchString) {
        *pDriverVersion = nvapiAdapter->GetDriverVersion();
        strcpy(szBuildBranchString, DXVK_NVAPI_VERSION);

        std::cerr << "NvAPI_SYS_GetDriverAndBranchVersion: OK" << std::endl;
        return NVAPI_OK;
    }

    NvAPI_Status __cdecl NvAPI_EnumLogicalGPUs(NvLogicalGpuHandle nvGPUHandle[NVAPI_MAX_LOGICAL_GPUS], NvU32 *pGpuCount) {
        nvGPUHandle[0] = 0x0;
        *pGpuCount = 1;

        std::cerr << "NvAPI_EnumLogicalGPUs: OK" << std::endl;
        return NVAPI_OK;
    }

    NvAPI_Status __cdecl NvAPI_EnumPhysicalGPUs(NvPhysicalGpuHandle nvGPUHandle[NVAPI_MAX_PHYSICAL_GPUS], NvU32 *pGpuCount) {
        nvGPUHandle[0] = 0x0;
        *pGpuCount = 1;

        std::cerr << "NvAPI_EnumPhysicalGPUs: OK" << std::endl;
        return NVAPI_OK;
    }

    NvAPI_Status __cdecl NvAPI_GetDisplayDriverVersion(NvDisplayHandle hNvDisplay, NV_DISPLAY_DRIVER_VERSION *pVersion) {
        std::cerr << "NvAPI_GetDisplayDriverVersion: No implementation" << std::endl;
        return NVAPI_NO_IMPLEMENTATION;
    }

    NvAPI_Status __cdecl NvAPI_GetInterfaceVersionString(NvAPI_ShortString szDesc) {
        strcpy(szDesc, "R440");

        std::cerr << "NvAPI_GetInterfaceVersionString: OK" << std::endl;
        return NVAPI_OK;
    }

    NvAPI_Status __cdecl NvAPI_GetErrorMessage(NvAPI_Status nr, NvAPI_ShortString szDesc) {
        std::cerr << "NvAPI_GetErrorMessage " << std::dec << nr << ": OK" << std::endl;
        return NVAPI_OK;
    }

    NvAPI_Status __cdecl NvAPI_Unload() {
        delete(nvapiAdapter);

        std::cerr << "NvAPI_Unload: OK" << std::endl;
        return NVAPI_OK;
    }

    NvAPI_Status __cdecl NvAPI_Initialize() {
        nvapiAdapter = new NvapiAdapter();
        auto success = nvapiAdapter->Initialize();
        if (!success) {
            std::cerr << "NvAPI_Initialize DXVK-NVAPI-" << DXVK_NVAPI_VERSION << ": ERROR" << std::endl;
            return NVAPI_ERROR;
        }

        std::cerr << "NvAPI_Initialize DXVK-NVAPI-" << DXVK_NVAPI_VERSION << ": OK" << std::endl;
        return NVAPI_OK;
    }
}
