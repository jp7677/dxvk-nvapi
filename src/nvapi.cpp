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

    typedef NvAPI_Status __cdecl (*NvapiMethod) (void); 
    static std::unordered_map<NvU32, NvapiMethod> registry;

    NVAPI_QUERYINTERFACE nvapi_QueryInterface(NvU32 id) {
        auto entry = registry.find(id);
        if (entry != registry.end()) {
            return NVAPI_CAST(entry->second);
        }

        auto it = std::find_if(
            std::begin(nvapi_interface_table),
            std::end(nvapi_interface_table),
            [id](const auto& item) { return item.id == id; });

        if (it == std::end(nvapi_interface_table)) {
            std::cerr << "NvAPI_QueryInterface 0x" << std::hex << id << ": Called with unknown id" << std::endl;
            registry.insert(std::make_pair(id, (NvapiMethod) nullptr));
        } else if (std::string(it->func) == "NvAPI_D3D11_SetDepthBoundsTest")
            registry.insert(std::make_pair(id, (NvapiMethod) NvAPI_D3D11_SetDepthBoundsTest));
        else if (std::string(it->func) == "NvAPI_D3D11_IsNvShaderExtnOpCodeSupported")
            registry.insert(std::make_pair(id, (NvapiMethod) NvAPI_D3D11_IsNvShaderExtnOpCodeSupported));
        else if (std::string(it->func) == "NvAPI_D3D_GetObjectHandleForResource")
            registry.insert(std::make_pair(id, (NvapiMethod) NvAPI_D3D_GetObjectHandleForResource));
        else if (std::string(it->func) == "NvAPI_D3D_SetResourceHint")
            registry.insert(std::make_pair(id, (NvapiMethod) NvAPI_D3D_SetResourceHint));
        else if (std::string(it->func) == "NvAPI_D3D_GetCurrentSLIState")
            registry.insert(std::make_pair(id, (NvapiMethod) NvAPI_D3D_GetCurrentSLIState));
        else if (std::string(it->func) == "NvAPI_DISP_GetDisplayIdByDisplayName")
            registry.insert(std::make_pair(id, (NvapiMethod) NvAPI_DISP_GetDisplayIdByDisplayName));
        else if (std::string(it->func) == "NvAPI_GetErrorMessage")
            registry.insert(std::make_pair(id, (NvapiMethod) NvAPI_GetErrorMessage));
        else if (std::string(it->func) == "NvAPI_Unload")
            registry.insert(std::make_pair(id, NvAPI_Unload));
        else if (std::string(it->func) == "NvAPI_Initialize")
            registry.insert(std::make_pair(id, NvAPI_Initialize));
        else {
            std::cerr << "NvAPI_QueryInterface " << it->func << ": Called with not implemented method" << std::endl;
            registry.insert(std::make_pair(id, (NvapiMethod) nullptr));
        }

        return NVAPI_CAST(registry.find(id)->second);
    }
}
