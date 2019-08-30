#include "nvapi_private.h"

extern "C" {
    using namespace dxvk;

    NVAPI_INTERFACE NvAPI_D3D11_SetDepthBoundsTest(IUnknown* pDeviceOrContext, NvU32 bEnable, float fMinDepth, float fMaxDepth)
    {
        static bool alreadyTested = false;
        if (!alreadyTested)
        {
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
        if (!alreadyLogged)
        {
            alreadyLogged = true;
            std::cerr << "NvAPI_D3D11_SetDepthBoundsTest: Succeeded" << std::endl;
        }

        return NVAPI_OK;
    }

    NVAPI_INTERFACE NvAPI_D3D11_IsNvShaderExtnOpCodeSupported(IUnknown* pDeviceOrContext, NvU32 code, bool* supported)
    {
        std::cerr << "NvAPI_D3D11_IsNvShaderExtnOpCodeSupported " << code << ": Not implemented" << std::endl;
        return NVAPI_NO_IMPLEMENTATION;
    }

    NVAPI_INTERFACE NvAPI_D3D_GetObjectHandleForResource(IUnknown* pDevice, IUnknown* pResource, NVDX_ObjectHandle* pHandle)
    {
        static bool alreadyLogged = false;
        if (!alreadyLogged)
        {
            alreadyLogged = true;
            std::cerr << "NvAPI_D3D_GetObjectHandleForResource: Not implemented" << std::endl;
        }

        return NVAPI_NO_IMPLEMENTATION;
    }

    NVAPI_INTERFACE NvAPI_D3D_SetResourceHint(IUnknown *pDev, NVDX_ObjectHandle obj, NVAPI_D3D_SETRESOURCEHINT_CATEGORY dwHintCategory, NvU32 dwHintName, NvU32 *pdwHintValue)
    {
        static bool alreadyLogged = false;
        if (!alreadyLogged)
        {
            alreadyLogged = true;
            std::cerr << "NvAPI_D3D_SetResourceHint: Not implemented" << std::endl;
        }
        
        return NVAPI_NO_IMPLEMENTATION;
    }

    NVAPI_INTERFACE NvAPI_D3D_GetCurrentSLIState(IUnknown* pDevice, NV_GET_CURRENT_SLI_STATE* pSliState)
    {
        std::cerr << "NvAPI_D3D_GetCurrentSLIState: Not implemented" << std::endl;
        return NVAPI_NO_IMPLEMENTATION;
    }

    NVAPI_INTERFACE NvAPI_DISP_GetDisplayIdByDisplayName(const char* displayName, NvU32* displayId)
    {
        std::cerr << "NvAPI_DISP_GetDisplayIdByDisplayName: Not implemented" << std::endl;
        return NVAPI_NO_IMPLEMENTATION;
    }

    NVAPI_INTERFACE NvAPI_Initialize(void)
    {
        std::cerr << "NvAPI_Initialize DXVK-NVAPI-" << DXVK_NVAPI_VERSION << ": OK" << std::endl;
        return NVAPI_OK;
    }

    NVAPI_INTERFACE NvAPI_GetErrorMessage (NvAPI_Status nr, NvAPI_ShortString szDesc)	
    {
        std::cerr << "NvAPI_GetErrorMessage " << nr << ": OK" << std::endl;
        return NVAPI_OK;
    }

    __declspec(dllexport) void* __stdcall nvapi_QueryInterface(unsigned int offset)
    {
        if (offset == 0x7aaf7a04)
            return reinterpret_cast<void*>(NvAPI_D3D11_SetDepthBoundsTest);
        if (offset == 0x5f68da40)
            return reinterpret_cast<void*>(NvAPI_D3D11_IsNvShaderExtnOpCodeSupported);
        if (offset == 0xfceac864) 
            return reinterpret_cast<void*>(NvAPI_D3D_GetObjectHandleForResource);
        if (offset == 0x6c0ed98c) 
            return reinterpret_cast<void*>(NvAPI_D3D_SetResourceHint);
        if (offset == 0x4b708b54)
            return reinterpret_cast<void*>(NvAPI_D3D_GetCurrentSLIState);
        if (offset == 0xae457190) 
            return reinterpret_cast<void*>(NvAPI_DISP_GetDisplayIdByDisplayName);
        if (offset == 0x0150e828)
            return reinterpret_cast<void*>(NvAPI_Initialize);
        if (offset == 0x6c2d048c)
            return reinterpret_cast<void*>(NvAPI_GetErrorMessage);
        if (offset == 0x33c7358c)
            return nullptr; /* NvAPI_Diag_ReportCallStart, optional */
        if (offset == 0x593e8644) 
            return nullptr; /* NvAPI_Diag_ReportCallReturn, optional */

        std::cerr << "NvAPI_QueryInterface 0x" << std::hex << offset << ": Called with unknown offset" << std::endl;
        return nullptr;
    }
}
