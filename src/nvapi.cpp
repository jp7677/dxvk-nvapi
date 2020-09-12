#include "nvapi_private.h"
#include "impl/nvapi_adapter_registry.h"
#include "util/util_statuscode.h"
#include "util/util_string.h"
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

        return Ok("NvAPI_EnumLogicalGPUs");
    }

    NvAPI_Status __cdecl NvAPI_EnumPhysicalGPUs(NvPhysicalGpuHandle nvGPUHandle[NVAPI_MAX_PHYSICAL_GPUS], NvU32 *pGpuCount) {
        for (auto i = 0U; i < nvapiAdapterRegistry->GetAdapterCount(); i++)
            nvGPUHandle[i] = (NvPhysicalGpuHandle) nvapiAdapterRegistry->GetAdapter(i);

        *pGpuCount = nvapiAdapterRegistry->GetAdapterCount();

        return Ok("NvAPI_EnumPhysicalGPUs");
    }

    NvAPI_Status __cdecl NvAPI_GetDisplayDriverVersion(NvDisplayHandle hNvDisplay, NV_DISPLAY_DRIVER_VERSION *pVersion) {
        if (pVersion->version != NV_DISPLAY_DRIVER_VERSION_VER)
            return IncompatibleStructVersion("NvAPI_GetDisplayDriverVersion");

        // Ignore hNvDisplay and query the first adapter
        pVersion->drvVersion = nvapiAdapterRegistry->GetAdapter()->GetDriverVersion();
        pVersion->bldChangeListNum = 0;
        strcpy(pVersion->szBuildBranchString, DXVK_NVAPI_VERSION);
        strcpy(pVersion->szAdapterString, nvapiAdapterRegistry->GetAdapter()->GetDeviceName().c_str());

        return Ok("NvAPI_GetDisplayDriverVersion");
    }

    NvAPI_Status __cdecl NvAPI_GetPhysicalGPUsFromDisplay(NvDisplayHandle hNvDisp, NvPhysicalGpuHandle nvGPUHandle[NVAPI_MAX_PHYSICAL_GPUS], NvU32 *pGpuCount) {
        auto output = nvapiAdapterRegistry->GetOutput(hNvDisp);
        if (output == nullptr)
            return ExpectedDisplayHandle("NvAPI_GetPhysicalGPUsFromDisplay");

        nvGPUHandle[0] = (NvPhysicalGpuHandle) output->GetParent();
        *pGpuCount = 1;

        return Ok("NvAPI_GetPhysicalGPUsFromDisplay");
    }

    NvAPI_Status __cdecl NvAPI_EnumNvidiaDisplayHandle(NvU32 thisEnum, NvDisplayHandle *pNvDispHandle) {
        auto output = nvapiAdapterRegistry->GetOutput(thisEnum);
        if (output == nullptr)
            return InvalidDisplayId(str::format("NvAPI_EnumNvidiaDisplayHandle ", thisEnum));

        *pNvDispHandle = (NvDisplayHandle) output;

        return Ok(str::format("NvAPI_EnumNvidiaDisplayHandle ", thisEnum));
    }

    NvAPI_Status __cdecl NvAPI_GetInterfaceVersionString(NvAPI_ShortString szDesc) {
        strcpy(szDesc, "R440");

        return Ok("NvAPI_GetInterfaceVersionString");
    }

    NvAPI_Status __cdecl NvAPI_GetErrorMessage(NvAPI_Status nr, NvAPI_ShortString szDesc) {
        return Ok(str::format("NvAPI_GetErrorMessage", nr));
    }

    NvAPI_Status __cdecl NvAPI_Unload() {
        delete(nvapiAdapterRegistry);

        return Ok("NvAPI_Unload");
    }

    NvAPI_Status __cdecl NvAPI_Initialize() {
        std::cerr << "DXVK-NVAPI-" << DXVK_NVAPI_VERSION << std::endl;
        nvapiAdapterRegistry = new NvapiAdapterRegistry();
        auto success = nvapiAdapterRegistry->Initialize();
        if (!success)
            return NvidiaDeviceNotFound(str::format("NvAPI_Initialize DXVK-NVAPI-", DXVK_NVAPI_VERSION));

        return Ok("NvAPI_Initialize");
    }
}
