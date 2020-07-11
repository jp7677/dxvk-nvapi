#include "../inc/nvapi_interface.h"
#include "nvapi.cpp"

extern "C" {
    using namespace dxvk;

    typedef NvAPI_Status __cdecl (*NvapiMethod) (void); 

    static std::unordered_map<NvU32, NvapiMethod> registry;

    NVAPI_QUERYINTERFACE nvapi_QueryInterface(NvU32 id) {
        auto entry = registry.find(id);
        if (entry != registry.end()) {
            return reinterpret_cast<void*>(entry->second);
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

        return reinterpret_cast<void*>(registry.find(id)->second);
    }
}
