#include "../inc/nvapi_interface.h"
#include "nvapi.cpp"

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif // __GNUC__

#define INSERT_WHEN_EQUALS(method) \
    if (std::string(it->func) == #method) { \
        registry.insert(std::make_pair(id, (NvAPI_Method) method)); \
    }

extern "C" {
    using namespace dxvk;
    using NvAPI_Method = NvAPI_Status (*) ();

    static std::unordered_map<NvU32, NvAPI_Method> registry;

    void* nvapi_QueryInterface(NvU32 id) {
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
            registry.insert(std::make_pair(id, (NvAPI_Method) nullptr));
        }
        else INSERT_WHEN_EQUALS(NvAPI_D3D11_SetDepthBoundsTest)
        else INSERT_WHEN_EQUALS(NvAPI_D3D11_IsNvShaderExtnOpCodeSupported)
        else INSERT_WHEN_EQUALS(NvAPI_D3D_GetObjectHandleForResource)
        else INSERT_WHEN_EQUALS(NvAPI_D3D_SetResourceHint)
        else INSERT_WHEN_EQUALS(NvAPI_D3D_GetCurrentSLIState)
        else INSERT_WHEN_EQUALS(NvAPI_DISP_GetDisplayIdByDisplayName)
        else INSERT_WHEN_EQUALS(NvAPI_GetErrorMessage)
        else INSERT_WHEN_EQUALS(NvAPI_Unload)
        else INSERT_WHEN_EQUALS(NvAPI_Initialize)
        else {
            std::cerr << "NvAPI_QueryInterface " << it->func << ": Called for not implemented method" << std::endl;
            registry.insert(std::make_pair(id, (NvAPI_Method) nullptr));
        }

        return reinterpret_cast<void*>(registry.find(id)->second);
    }
}

#undef INSERT_WHEN_EQUALS
