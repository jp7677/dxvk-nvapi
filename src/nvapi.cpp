#include "nvapi_private.h"
#include "nvapi_static.h"
#include "util/util_statuscode.h"
#include "util/util_error.h"
#include "util/util_string.h"
#include "util/util_env.h"
#include "util/util_log.h"
#include "../version.h"

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // __GNUC__

extern "C" {
    using namespace dxvk;

    NvAPI_Status __cdecl NvAPI_EnumLogicalGPUs(NvLogicalGpuHandle nvGPUHandle[NVAPI_MAX_LOGICAL_GPUS], NvU32 *pGpuCount) {
        constexpr auto n = "NvAPI_EnumLogicalGPUs";

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (nvGPUHandle == nullptr || pGpuCount == nullptr)
            return InvalidArgument(n);

        for (auto i = 0U; i < nvapiAdapterRegistry->GetAdapterCount(); i++)
            nvGPUHandle[i] = reinterpret_cast<NvLogicalGpuHandle>(nvapiAdapterRegistry->GetAdapter(i));

        *pGpuCount = nvapiAdapterRegistry->GetAdapterCount();

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_EnumPhysicalGPUs(NvPhysicalGpuHandle nvGPUHandle[NVAPI_MAX_PHYSICAL_GPUS], NvU32 *pGpuCount) {
        constexpr auto n = "NvAPI_EnumPhysicalGPUs";

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (nvGPUHandle == nullptr || pGpuCount == nullptr)
            return InvalidArgument(n);

        for (auto i = 0U; i < nvapiAdapterRegistry->GetAdapterCount(); i++)
            nvGPUHandle[i] = reinterpret_cast<NvPhysicalGpuHandle>(nvapiAdapterRegistry->GetAdapter(i));

        *pGpuCount = nvapiAdapterRegistry->GetAdapterCount();

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_GetDisplayDriverVersion(NvDisplayHandle hNvDisplay, NV_DISPLAY_DRIVER_VERSION *pVersion) {
        constexpr auto n = "NvAPI_GetDisplayDriverVersion";

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (pVersion == nullptr) // Ignore hNvDisplay
            return InvalidArgument(n);

        if (pVersion->version != NV_DISPLAY_DRIVER_VERSION_VER)
            return IncompatibleStructVersion(n);

        // Ignore hNvDisplay and query the first adapter
        pVersion->drvVersion = nvapiAdapterRegistry->GetAdapter()->GetDriverVersion();
        pVersion->bldChangeListNum = 0;
        strcpy(pVersion->szBuildBranchString, DXVK_NVAPI_VERSION);
        strcpy(pVersion->szAdapterString, nvapiAdapterRegistry->GetAdapter()->GetDeviceName().c_str());

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_GetPhysicalGPUsFromDisplay(NvDisplayHandle hNvDisp, NvPhysicalGpuHandle nvGPUHandle[NVAPI_MAX_PHYSICAL_GPUS], NvU32 *pGpuCount) {
        constexpr auto n = "NvAPI_GetPhysicalGPUsFromDisplay";

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

    NvAPI_Status __cdecl NvAPI_EnumNvidiaDisplayHandle(NvU32 thisEnum, NvDisplayHandle *pNvDispHandle) {
        constexpr auto n = "NvAPI_EnumNvidiaDisplayHandle";

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (pNvDispHandle == nullptr)
            return InvalidArgument(n);

        auto output = nvapiAdapterRegistry->GetOutput(thisEnum);
        if (output == nullptr)
            return EndEnumeration(str::format(n, " ", thisEnum));

        *pNvDispHandle = reinterpret_cast<NvDisplayHandle>(output);

        return Ok(str::format(n, " ", thisEnum));
    }

    NvAPI_Status __cdecl NvAPI_EnumNvidiaUnAttachedDisplayHandle(NvU32 thisEnum, NvUnAttachedDisplayHandle *pNvUnAttachedDispHandle) {
        // DXVK does not know about unattached displays
        return EndEnumeration(str::format("NvAPI_EnumNvidiaUnAttachedDisplayHandle ", thisEnum));
    }

    NvAPI_Status __cdecl NvAPI_GetInterfaceVersionString(NvAPI_ShortString szDesc) {
        constexpr auto n = "NvAPI_GetInterfaceVersionString";

        if (szDesc == nullptr)
            return InvalidArgument(n);

        strcpy(szDesc, "R470");

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_GetErrorMessage(NvAPI_Status nr, NvAPI_ShortString szDesc) {
        constexpr auto n = "NvAPI_GetErrorMessage";

        if (szDesc == nullptr)
            return InvalidArgument(n);

        auto error = fromErrorNr(nr);
        strcpy(szDesc, error.c_str());

        return Ok(str::format(n, " ", nr, " (", error, ")"));
    }

    static auto initializationMutex = std::mutex{};
    static auto initializationCount = 0ULL;

    NvAPI_Status __cdecl NvAPI_Unload() {
        constexpr auto n = "NvAPI_Unload";

        std::scoped_lock lock(initializationMutex);

        if (initializationCount == 0)
            return ApiNotInitialized(n);

        if (--initializationCount == 0)
            nvapiAdapterRegistry.reset();

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_Initialize() {
        constexpr auto n = "NvAPI_Initialize";

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

        return Ok(n);
    }
}

namespace dxvk {
    void SetResourceFactory(std::unique_ptr<ResourceFactory> factory) {
        std::scoped_lock lock(initializationMutex);

        resourceFactory = std::move(factory);
        nvapiAdapterRegistry.reset();
        initializationCount = 0ULL;
    }
}
