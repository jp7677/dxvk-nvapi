#include "nvapi_private.h"
#include "impl/nvapi_adapter_registry.h"
#include "../version.h"

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // __GNUC__

extern "C" {
    using namespace dxvk;

    NvAPI_Status __cdecl NvAPI_GPU_GetGPUType(NvPhysicalGpuHandle hPhysicalGpu, NV_GPU_TYPE *pGpuType) {
        auto adapter = nvapiAdapterRegistry->GetAdapter(hPhysicalGpu);
        if (adapter ==  nullptr) {
            std::cerr << "NvAPI_GPU_GetGPUType: EXPECTED_PHYSICAL_GPU_HANDLE" << std::endl;
            return NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE;
        }

        *pGpuType = (NV_GPU_TYPE) adapter->GetGpuType();

        std::cerr << "NvAPI_GPU_GetGPUType: OK" << std::endl;
        return NVAPI_OK;
    }

    NvAPI_Status __cdecl NvAPI_GPU_GetPCIIdentifiers(NvPhysicalGpuHandle hPhysicalGpu, NvU32 *pDeviceId, NvU32 *pSubSystemId, NvU32 *pRevisionId, NvU32 *pExtDeviceId) {
        auto adapter = nvapiAdapterRegistry->GetAdapter(hPhysicalGpu);
        if (adapter ==  nullptr) {
            std::cerr << "NvAPI_GPU_GetPCIIdentifiers: EXPECTED_PHYSICAL_GPU_HANDLE" << std::endl;
            return NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE;
        }

        *pDeviceId = adapter->GetDeviceId();
        *pSubSystemId = 0;
        *pRevisionId = 0;
        *pExtDeviceId = adapter->GetDeviceId();

        std::cerr << "NvAPI_GPU_GetPCIIdentifiers: OK" << std::endl;
        return NVAPI_OK;
    }

    NvAPI_Status __cdecl NvAPI_GPU_GetFullName(NvPhysicalGpuHandle hPhysicalGpu, NvAPI_ShortString szName) {
        auto adapter = nvapiAdapterRegistry->GetAdapter(hPhysicalGpu);
        if (adapter ==  nullptr) {
            std::cerr << "NvAPI_GPU_GetFullName: EXPECTED_PHYSICAL_GPU_HANDLE" << std::endl;
            return NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE;
        }

        strcpy(szName, adapter->GetDeviceName().c_str());

        std::cerr << "NvAPI_GPU_GetFullName: OK" << std::endl;
        return NVAPI_OK;
    }
}
