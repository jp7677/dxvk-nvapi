#include "../inc/nvapi_interface.h"
#include "nvapi.cpp"

#define INSERT_AND_RETURN_WHEN_EQUALS(method) \
    if (std::string(it->func) == #method) { \
        auto methodptr = reinterpret_cast<void*>(method); \
        registry.insert(std::make_pair(id, methodptr)); \
        return methodptr; \
    }

extern "C" {
    using namespace dxvk;

    static std::unordered_map<NvU32, void*> registry;

    void* nvapi_QueryInterface(NvU32 id) {
        auto entry = registry.find(id);
        if (entry != registry.end())
            return entry->second;

        auto it = std::find_if(
            std::begin(nvapi_interface_table),
            std::end(nvapi_interface_table),
            [id](const auto& item) { return item.id == id; });

        if (it == std::end(nvapi_interface_table)) {
            std::cerr << "NvAPI_QueryInterface 0x" << std::hex << id << ": Called with unknown id" << std::endl;
            registry.insert(std::make_pair(id, nullptr));
            return nullptr;
        }

        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D11_SetDepthBoundsTest)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D11_IsNvShaderExtnOpCodeSupported)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D_GetObjectHandleForResource)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D_SetResourceHint)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D_GetCurrentSLIState)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_DISP_GetDisplayIdByDisplayName)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GetErrorMessage)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_Unload)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_Initialize)
        
        std::cerr << "NvAPI_QueryInterface " << it->func << ": Called for not implemented method" << std::endl;
        registry.insert(std::make_pair(id, nullptr));
        return nullptr;
    }
}

#undef INSERT_AND_RETURN_WHEN_EQUALS
