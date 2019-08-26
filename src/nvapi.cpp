#include "nvapi_private.h"

static NvAPI_Status dxvkSetDepthBounds(
        ID3D11VkExtDevice*            device,
        ID3D11VkExtContext*           context,
        bool                          enabled,
        float                         minDepth,
        float                         maxDepth) {

  if (0 > minDepth || minDepth > maxDepth || maxDepth > 1)
    return NVAPI_INVALID_ARGUMENT;

  if (!device->GetExtensionSupport(D3D11_VK_EXT_DEPTH_BOUNDS))
    return NVAPI_ERROR;
  
  context->SetDepthBoundsTest(enabled, minDepth, maxDepth);
  return NVAPI_OK;
}

extern "C" {

    NVAPI_INTERFACE NvAPI_Initialize(void)
    {
        std::cerr << "NvAPI_Initialize: Called" << std::endl;
        return NVAPI_OK;
    }

    NVAPI_INTERFACE NvAPI_GetErrorMessage (NvAPI_Status nr, NvAPI_ShortString szDesc)	
    {
        std::cerr << "NvAPI_GetErrorMessage: Called for " << nr << std::endl;
        return NVAPI_OK;
    }

    NVAPI_INTERFACE NvAPI_DISP_GetDisplayIdByDisplayName(const char* displayName, NvU32* displayId)
    {
        return NVAPI_NO_IMPLEMENTATION;
    }
    
    NVAPI_INTERFACE NvAPI_D3D_GetCurrentSLIState(IUnknown *pDevice, NV_GET_CURRENT_SLI_STATE *pSliState)
    {
        return NVAPI_NO_IMPLEMENTATION;
    }

    NVAPI_INTERFACE NvAPI_D3D11_IsNvShaderExtnOpCodeSupported(IUnknown *pDeviceOrContext, NvU32 code, bool* supported)
    {
        std::cerr << "NvAPI_D3D11_IsNvShaderExtnOpCodeSupported: Called for code " << code << std::endl;
        return NVAPI_NO_IMPLEMENTATION;
    }

    NVAPI_INTERFACE NvAPI_D3D11_SetDepthBoundsTest(IUnknown *pDeviceOrContext, NvU32 enable, float minDepth, float maxDepth)
    {
        ID3D11VkExtDevice* dxvkDevice = nullptr;
        if (FAILED(pDeviceOrContext->QueryInterface(IID_PPV_ARGS(&dxvkDevice)))) {
            std::cerr << "Failed to get DXVK extension device handle" << std::endl;
            return NVAPI_ERROR;
        }
        dxvkDevice->Release();

        ID3D11Device* d3d11Device = nullptr;
        if (FAILED(pDeviceOrContext->QueryInterface(IID_PPV_ARGS(&d3d11Device)))) {
            std::cerr << "Failed to get DXVK device handle" << std::endl;
            return NVAPI_ERROR;
        }
        d3d11Device->Release();
    
        ID3D11DeviceContext* ctx = nullptr;
        d3d11Device->GetImmediateContext(&ctx);

        ID3D11VkExtContext* dxvkContext = nullptr;
        if (FAILED(ctx->QueryInterface(IID_PPV_ARGS(&dxvkContext)))) {
            std::cerr << "Failed to get DXVK context handle" << std::endl;
            return NVAPI_ERROR;
        }
        dxvkContext->Release();

        return dxvkSetDepthBounds(dxvkDevice, dxvkContext, enable, minDepth, maxDepth);
    }

    __declspec(dllexport) void* __stdcall nvapi_QueryInterface(unsigned int offset)
    {
        if (offset == 0x0150e828)
            return reinterpret_cast<void*>(NvAPI_Initialize);
        if (offset == 0x6c2d048c)
            return reinterpret_cast<void*>(NvAPI_GetErrorMessage);
        if (offset == 0x33c7358c)
            return nullptr; /* NvAPI_Diag_ReportCallStart, not needed */
        if (offset == 0x593e8644) 
            return nullptr; /* NvAPI_Diag_ReportCallReturn, not needed */
        if (offset == 0x5f68da40)
            return reinterpret_cast<void*>(NvAPI_D3D11_IsNvShaderExtnOpCodeSupported);
        if (offset == 0xae457190) 
            return reinterpret_cast<void*>(NvAPI_DISP_GetDisplayIdByDisplayName);
        if (offset == 0x4b708b54)
            return reinterpret_cast<void*>(NvAPI_D3D_GetCurrentSLIState);
        if (offset == 0xfceac864) 
            return nullptr; /* TODO: NvAPI_D3D_GetObjectHandleForResource */
        if (offset == 0x6c0ed98c) 
            return nullptr; /* TODO: NvAPI_D3D_SetResourceHint */
        if (offset == 0x7aaf7a04)
            return reinterpret_cast<void*>(NvAPI_D3D11_SetDepthBoundsTest);

        std::cerr << "nvapi_QueryInterface: Called with not implemented offset 0x" << std::hex << offset << std::endl;
        return nullptr;
    }
}
