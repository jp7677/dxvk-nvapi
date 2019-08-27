#include "nvapi_private.h"

static NvAPI_Status dxvkSetDepthBounds(ID3D11VkExtDevice* device, ID3D11VkExtContext* context, bool enabled, float minDepth, float maxDepth) {
    if (0 > minDepth || minDepth > maxDepth || maxDepth > 1)
        return NVAPI_INVALID_ARGUMENT;

    if (!device->GetExtensionSupport(D3D11_VK_EXT_DEPTH_BOUNDS))
        return NVAPI_ERROR;
  
    context->SetDepthBoundsTest(enabled, minDepth, maxDepth);
    return NVAPI_OK;
}

extern "C" {

    NVAPI_INTERFACE NvAPI_D3D11_SetDepthBoundsTest(IUnknown* pDeviceOrContext, NvU32 bEnable, float fMinDepth, float fMaxDepth)
    {
        ID3D11VkExtDevice* dxvkDevice = nullptr;
        if (FAILED(pDeviceOrContext->QueryInterface(IID_PPV_ARGS(&dxvkDevice))))
            return NVAPI_ERROR;

        dxvkDevice->Release();

        ID3D11Device* d3d11Device = nullptr;
        if (FAILED(pDeviceOrContext->QueryInterface(IID_PPV_ARGS(&d3d11Device))))
            return NVAPI_ERROR;

        d3d11Device->Release();
    
        ID3D11DeviceContext* ctx = nullptr;
        d3d11Device->GetImmediateContext(&ctx);

        ID3D11VkExtContext* dxvkContext = nullptr;
        if (FAILED(ctx->QueryInterface(IID_PPV_ARGS(&dxvkContext))))
            return NVAPI_ERROR;

        dxvkContext->Release();

        return dxvkSetDepthBounds(dxvkDevice, dxvkContext, bEnable, fMinDepth, fMaxDepth);
    }

    NVAPI_INTERFACE NvAPI_D3D11_IsNvShaderExtnOpCodeSupported(IUnknown* pDeviceOrContext, NvU32 code, bool* supported)
    {
        std::cerr << "NvAPI_D3D11_IsNvShaderExtnOpCodeSupported " << code << ": Not implemented" << std::endl;
        return NVAPI_NO_IMPLEMENTATION;
    }

    NVAPI_INTERFACE NvAPI_D3D_GetObjectHandleForResource(IUnknown* pDevice, IUnknown* pResource, NVDX_ObjectHandle* pHandle)
    {
        std::cerr << "NvAPI_D3D_GetObjectHandleForResource: Not implemented" << std::endl;
        return NVAPI_NO_IMPLEMENTATION;
    }

    NVAPI_INTERFACE NvAPI_D3D_SetResourceHint(IUnknown *pDev, NVDX_ObjectHandle obj, NVAPI_D3D_SETRESOURCEHINT_CATEGORY dwHintCategory, NvU32 dwHintName, NvU32 *pdwHintValue)
    {
        std::cerr << "NvAPI_D3D_SetResourceHint: Not implemented" << std::endl;
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
        std::cerr << "NvAPI_Initialize: OK" << std::endl;
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

        std::cerr << "nvapi_QueryInterface: Called with not implemented offset 0x" << std::hex << offset << std::endl;
        return nullptr;
    }
}
