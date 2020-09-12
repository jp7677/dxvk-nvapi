#include "nvapi_private.h"
#include "impl/nvapi_adapter_registry.h"
#include "../version.h"

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // __GNUC__

extern "C" {
    using namespace dxvk;

    static NvapiAdapterRegistry* nvapiAdapterRegistry;

    NvAPI_Status __cdecl NvAPI_EnumLogicalGPUs(NvLogicalGpuHandle nvGPUHandle[NVAPI_MAX_LOGICAL_GPUS], NvU32 *pGpuCount) {
        for (auto i = 0U; i < nvapiAdapterRegistry->GetAdapterCount(); i++)
            nvGPUHandle[i] = (NvLogicalGpuHandle) nvapiAdapterRegistry->GetAdapter(i);

        *pGpuCount = nvapiAdapterRegistry->GetAdapterCount();

        std::cerr << "NvAPI_EnumLogicalGPUs: OK" << std::endl;
        return NVAPI_OK;
    }

    NvAPI_Status __cdecl NvAPI_EnumPhysicalGPUs(NvPhysicalGpuHandle nvGPUHandle[NVAPI_MAX_PHYSICAL_GPUS], NvU32 *pGpuCount) {
        for (auto i = 0U; i < nvapiAdapterRegistry->GetAdapterCount(); i++)
            nvGPUHandle[i] = (NvPhysicalGpuHandle) nvapiAdapterRegistry->GetAdapter(i);

        *pGpuCount = nvapiAdapterRegistry->GetAdapterCount();

        std::cerr << "NvAPI_EnumPhysicalGPUs: OK" << std::endl;
        return NVAPI_OK;
    }

    NvAPI_Status __cdecl NvAPI_GetDisplayDriverVersion(NvDisplayHandle hNvDisplay, NV_DISPLAY_DRIVER_VERSION *pVersion) {
        if (pVersion->version != NV_DISPLAY_DRIVER_VERSION_VER) {
            std::cerr << "NvAPI_GetDisplayDriverVersion: NVAPI_INCOMPATIBLE_STRUCT_VERSION" << std::endl;
            return NVAPI_INCOMPATIBLE_STRUCT_VERSION;
        }

        // Ignore hNvDisplay and query the first adapter
        pVersion->drvVersion = nvapiAdapterRegistry->GetAdapter()->GetDriverVersion();
        pVersion->bldChangeListNum = 0;
        strcpy(pVersion->szBuildBranchString, DXVK_NVAPI_VERSION);
        strcpy(pVersion->szAdapterString, nvapiAdapterRegistry->GetAdapter()->GetDeviceName().c_str());

        std::cerr << "NvAPI_GetDisplayDriverVersion: OK" << std::endl;
        return NVAPI_OK;
    }

    NvAPI_Status __cdecl NvAPI_GetPhysicalGPUsFromDisplay(NvDisplayHandle hNvDisp, NvPhysicalGpuHandle nvGPUHandle[NVAPI_MAX_PHYSICAL_GPUS], NvU32 *pGpuCount) {
        auto output = nvapiAdapterRegistry->GetOutput(hNvDisp);
        if (output == nullptr) {
            std::cerr << "NvAPI_GetPhysicalGPUsFromDisplay: EXPECTED_DISPLAY_HANDLE" << std::endl;
            return NVAPI_EXPECTED_DISPLAY_HANDLE;
        }

        nvGPUHandle[0] = (NvPhysicalGpuHandle) output->GetParent();
        *pGpuCount = 1;

        std::cerr << "NvAPI_GetPhysicalGPUsFromDisplay: OK" << std::endl;
        return NVAPI_OK;
    }

    NvAPI_Status __cdecl NvAPI_EnumNvidiaDisplayHandle(NvU32 thisEnum, NvDisplayHandle *pNvDispHandle) {
        auto output = nvapiAdapterRegistry->GetOutput(thisEnum);
        if (output == nullptr) {
            std::cerr << "NvAPI_EnumNvidiaDisplayHandle " << thisEnum << ": INVALID_DISPLAY_ID" << std::endl;
            return NVAPI_INVALID_DISPLAY_ID;
        }

        *pNvDispHandle = (NvDisplayHandle) output;

        std::cerr << "NvAPI_EnumNvidiaDisplayHandle " << thisEnum << ": OK" << std::endl;
        return NVAPI_OK;
    }

    NvAPI_Status __cdecl NvAPI_GetInterfaceVersionString(NvAPI_ShortString szDesc) {
        strcpy(szDesc, "R440");

        std::cerr << "NvAPI_GetInterfaceVersionString: OK" << std::endl;
        return NVAPI_OK;
    }

    NvAPI_Status __cdecl NvAPI_GetErrorMessage(NvAPI_Status nr, NvAPI_ShortString szDesc) {
        std::cerr << "NvAPI_GetErrorMessage " << std::dec << nr << ": OK" << std::endl;
        return NVAPI_OK;
    }

    NvAPI_Status __cdecl NvAPI_Unload() {
        delete(nvapiAdapterRegistry);

        std::cerr << "NvAPI_Unload: OK" << std::endl;
        return NVAPI_OK;
    }

    NvAPI_Status __cdecl NvAPI_Initialize() {
        std::cerr << "DXVK-NVAPI-" << DXVK_NVAPI_VERSION << std::endl;
        nvapiAdapterRegistry = new NvapiAdapterRegistry();
        auto success = nvapiAdapterRegistry->Initialize();
        if (!success) {
            std::cerr << "NvAPI Device: No NVIDIA GPU has been found" << std::endl;
            std::cerr << "NvAPI_Initialize DXVK-NVAPI-" << DXVK_NVAPI_VERSION << ": ERROR" << std::endl;
            return NVAPI_NVIDIA_DEVICE_NOT_FOUND;
        }

        std::cerr << "NvAPI_Initialize: OK" << std::endl;
        return NVAPI_OK;
    }
}
