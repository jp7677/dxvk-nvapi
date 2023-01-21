#include "nvapi_private.h"
#include "nvapi_globals.h"
#include "util/util_statuscode.h"
#include "util/util_error.h"
#include "util/util_string.h"
#include "util/util_env.h"
#include "util/util_log.h"
#include "../version.h"

static auto initializationMutex = std::mutex{};

extern "C" {
    using namespace dxvk;

    NvAPI_Status __cdecl NvAPI_EnumLogicalGPUs(NvLogicalGpuHandle nvGPUHandle[NVAPI_MAX_LOGICAL_GPUS], NvU32* pGpuCount) {
        constexpr auto n = __func__;

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (nvGPUHandle == nullptr || pGpuCount == nullptr)
            return InvalidArgument(n);

        for (auto i = 0U; i < nvapiAdapterRegistry->GetAdapterCount(); i++)
            nvGPUHandle[i] = reinterpret_cast<NvLogicalGpuHandle>(nvapiAdapterRegistry->GetAdapter(i));

        *pGpuCount = nvapiAdapterRegistry->GetAdapterCount();

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_EnumPhysicalGPUs(NvPhysicalGpuHandle nvGPUHandle[NVAPI_MAX_PHYSICAL_GPUS], NvU32* pGpuCount) {
        constexpr auto n = __func__;

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (nvGPUHandle == nullptr || pGpuCount == nullptr)
            return InvalidArgument(n);

        for (auto i = 0U; i < nvapiAdapterRegistry->GetAdapterCount(); i++)
            nvGPUHandle[i] = reinterpret_cast<NvPhysicalGpuHandle>(nvapiAdapterRegistry->GetAdapter(i));

        *pGpuCount = nvapiAdapterRegistry->GetAdapterCount();

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_EnumTCCPhysicalGPUs(NvPhysicalGpuHandle nvGPUHandle[NVAPI_MAX_PHYSICAL_GPUS], NvU32* pGpuCount) {
        constexpr auto n = __func__;

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (nvGPUHandle == nullptr || pGpuCount == nullptr)
            return InvalidArgument(n);

        // There is no TCC mode on Linux, see https://forums.developer.nvidia.com/t/gpudirect-is-tcc-mode-a-requirement/79248
        // TCC can be queried on Windows using NVML by using https://docs.nvidia.com/deploy/nvml-api/group__nvmlDeviceQueries.html#group__nvmlDeviceQueries_1g6ad5e9e80313f958515b36acd1cb27b7
        // But we assume that no sane person would use this NVAPI implementation on Windows with GPUs in TCC mode.
        // Please open an issue if otherwise.
        *pGpuCount = 0;

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_GetGPUIDfromPhysicalGPU(NvPhysicalGpuHandle hPhysicalGpu, NvU32* pGpuId) {
        constexpr auto n = __func__;

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (pGpuId == nullptr)
            return InvalidArgument(n);

        auto adapter = reinterpret_cast<NvapiAdapter*>(hPhysicalGpu);
        if (!nvapiAdapterRegistry->IsAdapter(adapter))
            return ExpectedPhysicalGpuHandle(n);

        // The GPU ID seems to be the same as Board ID
        *pGpuId = adapter->GetBoardId();

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_GetPhysicalGPUFromGPUID(NvU32 gpuId, NvPhysicalGpuHandle* hPhysicalGpu) {
        constexpr auto n = __func__;

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (hPhysicalGpu == nullptr)
            return InvalidArgument(n);

        NvapiAdapter* adapter = nullptr;
        for (auto i = 0U; i < nvapiAdapterRegistry->GetAdapterCount(); i++)
            if (nvapiAdapterRegistry->GetAdapter(i)->GetBoardId() == gpuId)
                adapter = nvapiAdapterRegistry->GetAdapter(i);

        if (adapter == nullptr)
            return InvalidArgument(n);

        *hPhysicalGpu = reinterpret_cast<NvPhysicalGpuHandle>(adapter);

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_GetDisplayDriverVersion(NvDisplayHandle hNvDisplay, NV_DISPLAY_DRIVER_VERSION* pVersion) {
        constexpr auto n = __func__;

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (pVersion == nullptr) // Ignore hNvDisplay
            return InvalidArgument(n);

        if (pVersion->version != NV_DISPLAY_DRIVER_VERSION_VER)
            return IncompatibleStructVersion(n);

        // Ignore hNvDisplay and query the first adapter
        auto adapter = nvapiAdapterRegistry->GetFirstAdapter();
        pVersion->drvVersion = adapter->GetDriverVersion();
        pVersion->bldChangeListNum = 0;
        str::tonvss(pVersion->szBuildBranchString, str::format(NVAPI_VERSION, "_", DXVK_NVAPI_VERSION));
        str::tonvss(pVersion->szAdapterString, adapter->GetDeviceName());

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_GetPhysicalGPUsFromLogicalGPU(NvLogicalGpuHandle hLogicalGPU, NvPhysicalGpuHandle hPhysicalGPU[NVAPI_MAX_PHYSICAL_GPUS], NvU32* pGpuCount) {
        constexpr auto n = __func__;

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (hLogicalGPU == nullptr)
            return InvalidArgument(n);

        auto adapter = reinterpret_cast<NvapiAdapter*>(hLogicalGPU);
        if (!nvapiAdapterRegistry->IsAdapter(adapter))
            return ExpectedLogicalGpuHandle(n);

        hPhysicalGPU[0] = reinterpret_cast<NvPhysicalGpuHandle>(adapter);
        *pGpuCount = 1;

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_GetPhysicalGPUsFromDisplay(NvDisplayHandle hNvDisp, NvPhysicalGpuHandle nvGPUHandle[NVAPI_MAX_PHYSICAL_GPUS], NvU32* pGpuCount) {
        constexpr auto n = __func__;

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (hNvDisp == nullptr || nvGPUHandle == nullptr || pGpuCount == nullptr)
            return InvalidArgument(n);

        auto output = reinterpret_cast<NvapiOutput*>(hNvDisp);
        if (!nvapiAdapterRegistry->IsOutput(output))
            return ExpectedDisplayHandle(n);

        nvGPUHandle[0] = reinterpret_cast<NvPhysicalGpuHandle>(output->GetParent());
        *pGpuCount = 1;

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_EnumNvidiaDisplayHandle(NvU32 thisEnum, NvDisplayHandle* pNvDispHandle) {
        constexpr auto n = __func__;

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (pNvDispHandle == nullptr)
            return InvalidArgument(n);

        auto output = nvapiAdapterRegistry->GetOutput(thisEnum);
        if (output == nullptr)
            return EndEnumeration(str::format(n, " (", thisEnum, ")"));

        *pNvDispHandle = reinterpret_cast<NvDisplayHandle>(output);

        return Ok(str::format(n, " (", thisEnum, ")"));
    }

    NvAPI_Status __cdecl NvAPI_EnumNvidiaUnAttachedDisplayHandle(NvU32 thisEnum, NvUnAttachedDisplayHandle* pNvUnAttachedDispHandle) {
        // DXVK does not know about unattached displays
        return EndEnumeration(str::format(__func__, " (", thisEnum, ")"));
    }

    NvAPI_Status __cdecl NvAPI_GetAssociatedNvidiaDisplayName(NvDisplayHandle NvDispHandle, NvAPI_ShortString szDisplayName) {
        constexpr auto n = __func__;

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (NvDispHandle == nullptr)
            return InvalidArgument(n);

        auto output = reinterpret_cast<NvapiOutput*>(NvDispHandle);
        if (!nvapiAdapterRegistry->IsOutput(output))
            return ExpectedDisplayHandle(n);

        str::tonvss(szDisplayName, output->GetDeviceName());

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_GetInterfaceVersionString(NvAPI_ShortString szDesc) {
        constexpr auto n = __func__;

        if (szDesc == nullptr)
            return InvalidArgument(n);

        str::tonvss(szDesc, "DXVK_NVAPI");

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_GetErrorMessage(NvAPI_Status nr, NvAPI_ShortString szDesc) {
        constexpr auto n = __func__;

        if (szDesc == nullptr)
            return InvalidArgument(n);

        auto error = fromErrorNr(nr);
        str::tonvss(szDesc, error);

        return Ok(str::format(n, " (", nr, "/", error, ")"));
    }

    NvAPI_Status __cdecl NvAPI_Unload() {
        constexpr auto n = __func__;

        std::scoped_lock lock(initializationMutex);

        if (initializationCount == 0)
            return ApiNotInitialized(n);

        if (--initializationCount == 0)
            nvapiAdapterRegistry.reset();

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_Initialize() {
        constexpr auto n = __func__;

        std::scoped_lock lock(initializationMutex);

        if (++initializationCount > 1)
            return Ok(n);

        log::write(str::format("DXVK-NVAPI ", DXVK_NVAPI_VERSION, " (", env::getExecutableName(), ")"));

        if (resourceFactory == nullptr)
            resourceFactory = std::make_unique<ResourceFactory>();

        nvapiAdapterRegistry = std::make_unique<NvapiAdapterRegistry>(*resourceFactory);
        if (!nvapiAdapterRegistry->Initialize()) {
            nvapiAdapterRegistry.reset();
            --initializationCount;
            return NvidiaDeviceNotFound(n);
        }

        nvapiD3dInstance = std::make_unique<NvapiD3dInstance>(*resourceFactory);
        nvapiD3dInstance->Initialize();

        return Ok(n);
    }
}
