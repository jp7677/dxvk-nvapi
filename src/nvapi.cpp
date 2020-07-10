#include "nvapi_private.h"
#include "../inc/nvapi_interface.h"

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

    NVAPI_QUERYINTERFACE nvapi_QueryInterface(NvU32 id) {
        if (id == 0x33c7358c) /* NvAPI_Diag_ReportCallStart, optional */
            return nullptr;
        if (id == 0x593e8644) /* NvAPI_Diag_ReportCallReturn, optional */
            return nullptr; 

        auto it = std::find_if(std::begin(nvapi_interface_table), std::end(nvapi_interface_table),
            [id](const auto& item) {
                return item.id == id;
            });

        if (it == std::end(nvapi_interface_table)) {
            std::cerr << "NvAPI_QueryInterface 0x" << std::hex << id << ": Called with unknown id" << std::endl;
            return nullptr;
        }

        if (std::string(it->func) == "NvAPI_Unload")
            return NVAPI_CAST(NvAPI_Unload);
        if (std::string(it->func) == "NvAPI_Initialize")
            return NVAPI_CAST(NvAPI_Initialize);
        if (std::string(it->func) == "NvAPI_D3D11_SetDepthBoundsTest")
            return NVAPI_CAST(NvAPI_D3D11_SetDepthBoundsTest);
        if (std::string(it->func) == "NvAPI_D3D11_IsNvShaderExtnOpCodeSupported")
            return NVAPI_CAST(NvAPI_D3D11_IsNvShaderExtnOpCodeSupported);
        if (std::string(it->func) == "NvAPI_D3D_GetObjectHandleForResource")
            return NVAPI_CAST(NvAPI_D3D_GetObjectHandleForResource);
        if (std::string(it->func) == "NvAPI_D3D_SetResourceHint")
            return NVAPI_CAST(NvAPI_D3D_SetResourceHint);
        if (std::string(it->func) == "NvAPI_D3D_GetCurrentSLIState")
            return NVAPI_CAST(NvAPI_D3D_GetCurrentSLIState);
        if (std::string(it->func) == "NvAPI_DISP_GetDisplayIdByDisplayName")
            return NVAPI_CAST(NvAPI_DISP_GetDisplayIdByDisplayName);
        if (std::string(it->func) == "NvAPI_GetErrorMessage")
            return NVAPI_CAST(NvAPI_GetErrorMessage);

        std::cerr << "NvAPI_QueryInterface " << it->func << ": Called with not implemented method" << std::endl;
        return nullptr;
    }
}
