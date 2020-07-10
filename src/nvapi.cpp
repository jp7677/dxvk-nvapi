#include "nvapi_private.h"
#include "../inc/nvapi_interface.h"
#include "../inc/nvapi.h"

extern "C" {
    using namespace dxvk;

    NVAPI_METHOD NvAPI_D3D11_SetDepthBoundsTest(IUnknown* pDeviceOrContext, NvU32 bEnable, float fMinDepth, float fMaxDepth) {
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

    NVAPI_METHOD NvAPI_D3D11_IsNvShaderExtnOpCodeSupported(IUnknown* pDeviceOrContext, NvU32 code, bool* supported) {
        *supported = false;

        std::cerr << "NvAPI_D3D11_IsNvShaderExtnOpCodeSupported " << std::dec << code << ": OK" << std::endl;
        return NVAPI_OK;
    }

    NVAPI_METHOD NvAPI_D3D_GetObjectHandleForResource(IUnknown* pDevice, IUnknown* pResource, NVDX_ObjectHandle* pHandle) {
        static bool alreadyLogged = false;
        if (!alreadyLogged) {
            alreadyLogged = true;
            std::cerr << "NvAPI_D3D_GetObjectHandleForResource: Not implemented" << std::endl;
        }

        return NVAPI_NO_IMPLEMENTATION;
    }

    NVAPI_METHOD NvAPI_D3D_SetResourceHint(IUnknown *pDev, NVDX_ObjectHandle obj, NVAPI_D3D_SETRESOURCEHINT_CATEGORY dwHintCategory, NvU32 dwHintName, NvU32 *pdwHintValue) {
        static bool alreadyLogged = false;
        if (!alreadyLogged) {
            alreadyLogged = true;
            std::cerr << "NvAPI_D3D_SetResourceHint: Not implemented" << std::endl;
        }
        
        return NVAPI_NO_IMPLEMENTATION;
    }

    NVAPI_METHOD NvAPI_D3D_GetCurrentSLIState(IUnknown* pDevice, NV_GET_CURRENT_SLI_STATE* pSliState) {
        std::cerr << "NvAPI_D3D_GetCurrentSLIState: Not implemented" << std::endl;
        return NVAPI_NO_IMPLEMENTATION;
    }

    NVAPI_METHOD NvAPI_DISP_GetDisplayIdByDisplayName(const char* displayName, NvU32* displayId) {
        std::cerr << "NvAPI_DISP_GetDisplayIdByDisplayName: Not implemented" << std::endl;
        return NVAPI_NO_IMPLEMENTATION;
    }

    NVAPI_METHOD NvAPI_GetErrorMessage(NvAPI_Status nr, NvAPI_ShortString szDesc) {
        std::cerr << "NvAPI_GetErrorMessage " << std::dec << nr << ": OK" << std::endl;
        return NVAPI_OK;
    }

    NVAPI_METHOD NvAPI_Unload() {
        std::cerr << "NvAPI_Unload: OK" << std::endl;
        return NVAPI_OK;
    }

    NVAPI_METHOD NvAPI_Initialize() {
        std::cerr << "NvAPI_Initialize DXVK-NVAPI-" << DXVK_NVAPI_VERSION << ": OK" << std::endl;
        return NVAPI_OK;
    }

    NVAPI_QUERYINTERFACE nvapi_QueryInterface(NvU32 offset) {
        switch(offset) {
            case 0x7aaf7a04: return NVAPI_CAST(NvAPI_D3D11_SetDepthBoundsTest);
            case 0x5f68da40: return NVAPI_CAST(NvAPI_D3D11_IsNvShaderExtnOpCodeSupported);
            case 0xfceac864: return NVAPI_CAST(NvAPI_D3D_GetObjectHandleForResource);
            case 0x6c0ed98c: return NVAPI_CAST(NvAPI_D3D_SetResourceHint);
            case 0x4b708b54: return NVAPI_CAST(NvAPI_D3D_GetCurrentSLIState);
            case 0xae457190: return NVAPI_CAST(NvAPI_DISP_GetDisplayIdByDisplayName);
            case 0x6c2d048c: return NVAPI_CAST(NvAPI_GetErrorMessage);
            case 0xd22bdd7e: return NVAPI_CAST(NvAPI_Unload);
            case 0x0150e828: return NVAPI_CAST(NvAPI_Initialize);
            case 0x33c7358c: return nullptr; /* NvAPI_Diag_ReportCallStart, optional */
            case 0x593e8644: return nullptr; /* NvAPI_Diag_ReportCallReturn, optional */
            default: 
                std::cerr << "NvAPI_QueryInterface 0x" << std::hex << offset << ": Called with unknown offset" << std::endl;
                return nullptr;
        }
    }
}
