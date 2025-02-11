#include "nvapi_private.h"
#include "nvapi_globals.h"
#include "util/util_statuscode.h"
#include "util/util_error.h"
#include "util/util_string.h"
#include "util/util_env.h"
#include "util/util_log.h"
#include "util/util_ngx_debug.h"
#include "../version.h"
#include "../config.h"

static auto initializationMutex = std::mutex{};

extern "C" {
    using namespace dxvk;

    NvAPI_Status __cdecl NvAPI_EnumLogicalGPUs(NvLogicalGpuHandle nvGPUHandle[NVAPI_MAX_LOGICAL_GPUS], NvU32* pGpuCount) {
        constexpr auto n = __func__;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(nvGPUHandle), log::fmt::ptr(pGpuCount));

        if (!nvapiAdapterRegistry)
            return ApiNotInitialized(n);

        if (!nvGPUHandle || !pGpuCount)
            return InvalidArgument(n);

        for (auto i = 0U; i < nvapiAdapterRegistry->GetAdapterCount(); i++)
            nvGPUHandle[i] = reinterpret_cast<NvLogicalGpuHandle>(nvapiAdapterRegistry->GetAdapter(i));

        *pGpuCount = nvapiAdapterRegistry->GetAdapterCount();

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_EnumPhysicalGPUs(NvPhysicalGpuHandle nvGPUHandle[NVAPI_MAX_PHYSICAL_GPUS], NvU32* pGpuCount) {
        constexpr auto n = __func__;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(nvGPUHandle), log::fmt::ptr(pGpuCount));

        if (!nvapiAdapterRegistry)
            return ApiNotInitialized(n);

        if (!nvGPUHandle || !pGpuCount)
            return InvalidArgument(n);

        for (auto i = 0U; i < nvapiAdapterRegistry->GetAdapterCount(); i++)
            nvGPUHandle[i] = reinterpret_cast<NvPhysicalGpuHandle>(nvapiAdapterRegistry->GetAdapter(i));

        *pGpuCount = nvapiAdapterRegistry->GetAdapterCount();

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_EnumTCCPhysicalGPUs(NvPhysicalGpuHandle nvGPUHandle[NVAPI_MAX_PHYSICAL_GPUS], NvU32* pGpuCount) {
        constexpr auto n = __func__;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(nvGPUHandle), log::fmt::ptr(pGpuCount));

        if (!nvapiAdapterRegistry)
            return ApiNotInitialized(n);

        if (!nvGPUHandle || !pGpuCount)
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

        if (log::tracing())
            log::trace(n, log::fmt::hnd(hPhysicalGpu), log::fmt::ptr(pGpuId));

        if (!nvapiAdapterRegistry)
            return ApiNotInitialized(n);

        if (!pGpuId)
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

        if (log::tracing())
            log::trace(n, gpuId, log::fmt::ptr(hPhysicalGpu));

        if (!nvapiAdapterRegistry)
            return ApiNotInitialized(n);

        if (!hPhysicalGpu)
            return InvalidArgument(n);

        NvapiAdapter* adapter = nullptr;
        for (auto i = 0U; i < nvapiAdapterRegistry->GetAdapterCount(); i++)
            if (nvapiAdapterRegistry->GetAdapter(i)->GetBoardId() == gpuId)
                adapter = nvapiAdapterRegistry->GetAdapter(i);

        if (!adapter)
            return InvalidArgument(n);

        *hPhysicalGpu = reinterpret_cast<NvPhysicalGpuHandle>(adapter);

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_GetDisplayDriverVersion(NvDisplayHandle hNvDisplay, NV_DISPLAY_DRIVER_VERSION* pVersion) {
        constexpr auto n = __func__;

        if (log::tracing())
            log::trace(n, log::fmt::hnd(hNvDisplay), log::fmt::ptr(pVersion));

        if (!nvapiAdapterRegistry)
            return ApiNotInitialized(n);

        if (!pVersion) // Ignore hNvDisplay
            return InvalidArgument(n);

        if (pVersion->version != NV_DISPLAY_DRIVER_VERSION_VER)
            return IncompatibleStructVersion(n, pVersion->version);

        // Ignore hNvDisplay and query the first adapter
        auto adapter = nvapiAdapterRegistry->GetFirstAdapter();
        pVersion->drvVersion = adapter->GetDriverVersion();
        pVersion->bldChangeListNum = 0;
        str::tonvss(pVersion->szBuildBranchString, str::format("r", NVAPI_SDK_VERSION, "_", DXVK_NVAPI_VERSION));
        str::tonvss(pVersion->szAdapterString, adapter->GetDeviceName());

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_GetLogicalGPUFromPhysicalGPU(NvPhysicalGpuHandle hPhysicalGPU, NvLogicalGpuHandle* pLogicalGPU) {
        constexpr auto n = __func__;

        if (log::tracing())
            log::trace(n, log::fmt::hnd(hPhysicalGPU), log::fmt::ptr(pLogicalGPU));

        if (!nvapiAdapterRegistry)
            return ApiNotInitialized(n);

        if (!pLogicalGPU)
            return InvalidArgument(n);

        auto adapter = reinterpret_cast<NvapiAdapter*>(hPhysicalGPU);
        if (!nvapiAdapterRegistry->IsAdapter(adapter))
            return ExpectedPhysicalGpuHandle(n);

        *pLogicalGPU = reinterpret_cast<NvLogicalGpuHandle>(adapter);

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_GetLogicalGPUFromDisplay(NvDisplayHandle hNvDisp, NvLogicalGpuHandle* pLogicalGPU) {
        constexpr auto n = __func__;

        if (log::tracing())
            log::trace(n, log::fmt::hnd(hNvDisp), log::fmt::ptr(pLogicalGPU));

        if (!nvapiAdapterRegistry)
            return ApiNotInitialized(n);

        if (!pLogicalGPU)
            return InvalidArgument(n);

        auto output = reinterpret_cast<NvapiOutput*>(hNvDisp);
        if (!nvapiAdapterRegistry->IsOutput(output))
            return ExpectedDisplayHandle(n);

        *pLogicalGPU = reinterpret_cast<NvLogicalGpuHandle>(output->GetParent());

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_GetPhysicalGPUsFromLogicalGPU(NvLogicalGpuHandle hLogicalGPU, NvPhysicalGpuHandle hPhysicalGPU[NVAPI_MAX_PHYSICAL_GPUS], NvU32* pGpuCount) {
        constexpr auto n = __func__;

        if (log::tracing())
            log::trace(n, log::fmt::hnd(hLogicalGPU), log::fmt::ptr(hPhysicalGPU), log::fmt::ptr(pGpuCount));

        if (!nvapiAdapterRegistry)
            return ApiNotInitialized(n);

        if (!hPhysicalGPU || !pGpuCount)
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

        if (log::tracing())
            log::trace(n, log::fmt::hnd(hNvDisp), log::fmt::ptr(nvGPUHandle), log::fmt::ptr(pGpuCount));

        if (!nvapiAdapterRegistry)
            return ApiNotInitialized(n);

        if (!nvGPUHandle || !pGpuCount)
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

        if (log::tracing())
            log::trace(n, thisEnum, log::fmt::ptr(pNvDispHandle));

        if (!nvapiAdapterRegistry)
            return ApiNotInitialized(n);

        if (!pNvDispHandle)
            return InvalidArgument(n);

        auto output = nvapiAdapterRegistry->GetOutput(thisEnum);
        if (!output)
            return EndEnumeration(str::format(n, " (", thisEnum, ")"));

        *pNvDispHandle = reinterpret_cast<NvDisplayHandle>(output);

        return Ok(str::format(n, " (", thisEnum, ")"));
    }

    NvAPI_Status __cdecl NvAPI_EnumNvidiaUnAttachedDisplayHandle(NvU32 thisEnum, NvUnAttachedDisplayHandle* pNvUnAttachedDispHandle) {
        constexpr auto n = __func__;

        if (log::tracing())
            log::trace(n, thisEnum, log::fmt::ptr(pNvUnAttachedDispHandle));

        // DXVK does not know about unattached displays
        return EndEnumeration(str::format(n, " (", thisEnum, ")"));
    }

    NvAPI_Status __cdecl NvAPI_GetAssociatedNvidiaDisplayName(NvDisplayHandle NvDispHandle, NvAPI_ShortString szDisplayName) {
        constexpr auto n = __func__;

        if (log::tracing())
            log::trace(n, log::fmt::hnd(NvDispHandle), log::fmt::ptr(szDisplayName));

        if (!nvapiAdapterRegistry)
            return ApiNotInitialized(n);

        if (!szDisplayName)
            return InvalidArgument(n);

        auto output = reinterpret_cast<NvapiOutput*>(NvDispHandle);
        if (!nvapiAdapterRegistry->IsOutput(output))
            return ExpectedDisplayHandle(n);

        str::tonvss(szDisplayName, output->GetDeviceName());

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_GetAssociatedNvidiaDisplayHandle(const char* szDisplayName, NvDisplayHandle* pNvDispHandle) {
        constexpr auto n = __func__;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(szDisplayName), log::fmt::ptr(pNvDispHandle));

        if (!nvapiAdapterRegistry)
            return ApiNotInitialized(n);

        if (!szDisplayName || !pNvDispHandle)
            return InvalidArgument(n);

        auto output = nvapiAdapterRegistry->FindOutput(szDisplayName);
        if (!output)
            return NvidiaDeviceNotFound(n);

        *pNvDispHandle = reinterpret_cast<NvDisplayHandle>(output);

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_GetInterfaceVersionString(NvAPI_ShortString szDesc) {
        constexpr auto n = __func__;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(szDesc));

        if (!szDesc)
            return InvalidArgument(n);

        str::tonvss(szDesc, "NVAPI Open Source Interface (DXVK-NVAPI)");

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_GetErrorMessage(NvAPI_Status nr, NvAPI_ShortString szDesc) {
        constexpr auto n = __func__;

        if (log::tracing())
            log::trace(n, nr, log::fmt::ptr(szDesc));

        if (!szDesc)
            return InvalidArgument(n);

        auto error = fromErrorNr(nr);
        str::tonvss(szDesc, error);

        return Ok(str::format(n, " (", nr, "/", error, ")"));
    }

    NvAPI_Status __cdecl NvAPI_Unload() {
        constexpr auto n = __func__;

        if (log::tracing())
            log::trace(n);

        std::scoped_lock lock(initializationMutex);

        if (initializationCount == 0)
            return ApiNotInitialized(n);

        if (--initializationCount == 0)
            nvapiAdapterRegistry.reset();

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_Initialize() {
        constexpr auto n = __func__;

        if (log::tracing())
            log::trace(n);

        std::scoped_lock lock(initializationMutex);

        if (++initializationCount > 1)
            return Ok(n);

        log::info(str::format(
            "DXVK-NVAPI ", DXVK_NVAPI_VERSION,
            " NVAPI",
            " ", DXVK_NVAPI_BUILD_COMPILER,
            " ", DXVK_NVAPI_BUILD_COMPILER_VERSION,
            " ", DXVK_NVAPI_BUILD_TARGET,
            " ", DXVK_NVAPI_BUILD_TYPE,
            " (", env::getExecutableName(), ")"));

        if (!resourceFactory)
            resourceFactory = std::make_unique<NvapiResourceFactory>();

        nvapiAdapterRegistry = std::make_unique<NvapiAdapterRegistry>(*resourceFactory);
        if (!nvapiAdapterRegistry->Initialize()) {
            nvapiAdapterRegistry.reset();
            --initializationCount;
            return NvidiaDeviceNotFound(n);
        }

#if _WIN64
        SetNgxDebugOptions(); // NGX is 64-bit only
#endif

        return Ok(n);
    }
}
