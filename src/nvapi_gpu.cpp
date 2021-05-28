#include "nvapi_private.h"
#include "nvapi_static.h"
#include "util/util_statuscode.h"

extern "C" {
    using namespace dxvk;

    NvAPI_Status __cdecl NvAPI_GPU_GetGPUType(NvPhysicalGpuHandle hPhysicalGpu, NV_GPU_TYPE *pGpuType) {
        constexpr auto n = "NvAPI_GPU_GetGPUType";

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (hPhysicalGpu == nullptr || pGpuType == nullptr)
            return InvalidArgument(n);

        auto adapter = reinterpret_cast<NvapiAdapter*>(hPhysicalGpu);
        if (!nvapiAdapterRegistry->IsAdapter(adapter))
            return ExpectedPhysicalGpuHandle(n);

        *pGpuType = (NV_GPU_TYPE) adapter->GetGpuType();

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_GPU_GetPCIIdentifiers(NvPhysicalGpuHandle hPhysicalGpu, NvU32 *pDeviceId, NvU32 *pSubSystemId, NvU32 *pRevisionId, NvU32 *pExtDeviceId) {
        constexpr auto n = "NvAPI_GPU_GetPCIIdentifiers";

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (hPhysicalGpu == nullptr || pDeviceId == nullptr || pSubSystemId == nullptr || pRevisionId == nullptr || pExtDeviceId == nullptr)
            return InvalidArgument(n);

        auto adapter = reinterpret_cast<NvapiAdapter*>(hPhysicalGpu);
        if (!nvapiAdapterRegistry->IsAdapter(adapter))
            return ExpectedPhysicalGpuHandle(n);

        *pDeviceId = adapter->GetDeviceId();
        *pSubSystemId = 0;
        *pRevisionId = 0;
        *pExtDeviceId = 0;

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_GPU_GetFullName(NvPhysicalGpuHandle hPhysicalGpu, NvAPI_ShortString szName) {
        constexpr auto n = "NvAPI_GPU_GetFullName";

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (hPhysicalGpu == nullptr || szName == nullptr)
            return InvalidArgument(n);

        auto adapter = reinterpret_cast<NvapiAdapter*>(hPhysicalGpu);
        if (!nvapiAdapterRegistry->IsAdapter(adapter))
            return ExpectedPhysicalGpuHandle(n);

        strcpy(szName, adapter->GetDeviceName().c_str());

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_GPU_GetBusId(NvPhysicalGpuHandle hPhysicalGpu, NvU32 *pBusId) {
        constexpr auto n = "NvAPI_GPU_GetBusId";

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (hPhysicalGpu == nullptr || pBusId == nullptr)
            return InvalidArgument(n);

        auto adapter = reinterpret_cast<NvapiAdapter*>(hPhysicalGpu);
        if (!nvapiAdapterRegistry->IsAdapter(adapter))
            return ExpectedPhysicalGpuHandle(n);

        *pBusId = adapter->GetBusId();

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_GPU_GetPhysicalFrameBufferSize(NvPhysicalGpuHandle hPhysicalGpu, NvU32 *pSize) {
        constexpr auto n = "NvAPI_GPU_GetPhysicalFrameBufferSize";

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (pSize == nullptr)
            return InvalidArgument(n);

        auto adapter = reinterpret_cast<NvapiAdapter*>(hPhysicalGpu);
        if (!nvapiAdapterRegistry->IsAdapter(adapter))
            return ExpectedPhysicalGpuHandle(n);

        *pSize = adapter->GetVRamSize();

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_GPU_GetAdapterIdFromPhysicalGpu(NvPhysicalGpuHandle hPhysicalGpu, void *pOSAdapterId) {
        constexpr auto n = "NvAPI_GPU_GetAdapterIdFromPhysicalGpu";

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (pOSAdapterId == nullptr)
            return InvalidArgument(n);

        auto adapter = reinterpret_cast<NvapiAdapter*>(hPhysicalGpu);
        if (!nvapiAdapterRegistry->IsAdapter(adapter))
            return ExpectedPhysicalGpuHandle(n);

        if (!adapter->GetLUID(static_cast<LUID*>(pOSAdapterId)))
            return Error(n);

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_GPU_GetArchInfo(NvPhysicalGpuHandle hPhysicalGpu, NV_GPU_ARCH_INFO *pGpuArchInfo) {
        constexpr auto n = "NvAPI_GPU_GetArchInfo";

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (pGpuArchInfo == nullptr)
            return InvalidArgument(n);

        auto adapter = reinterpret_cast<NvapiAdapter*>(hPhysicalGpu);
        if (!nvapiAdapterRegistry->IsAdapter(adapter))
            return ExpectedPhysicalGpuHandle(n);

        if (pGpuArchInfo->version != NV_GPU_ARCH_INFO_VER_1 && pGpuArchInfo->version != NV_GPU_ARCH_INFO_VER_2)
            return IncompatibleStructVersion(n);

        if (adapter->GetDriverId() != VK_DRIVER_ID_NVIDIA_PROPRIETARY)
            return NvidiaDeviceNotFound(n);

        pGpuArchInfo->architecture_id = adapter->GetArchitectureId();

        // Assume the implementation ID from the architecture ID. No simple way
        // to do a more fine-grained query at this time. Would need wine-nvml
        // usage.
        switch(pGpuArchInfo->architecture_id) {
            case NV_GPU_ARCHITECTURE_GK100:
                pGpuArchInfo->implementation_id = NV_GPU_ARCH_IMPLEMENTATION_GK104;
                break;
            case NV_GPU_ARCHITECTURE_GM200:
                pGpuArchInfo->implementation_id = NV_GPU_ARCH_IMPLEMENTATION_GM204;
                break;
            case NV_GPU_ARCHITECTURE_GP100:
                pGpuArchInfo->implementation_id = NV_GPU_ARCH_IMPLEMENTATION_GP102;
                break;
            case NV_GPU_ARCHITECTURE_GV100:
                pGpuArchInfo->implementation_id = NV_GPU_ARCH_IMPLEMENTATION_GV100;
                break;
            case NV_GPU_ARCHITECTURE_TU100:
                pGpuArchInfo->implementation_id = NV_GPU_ARCH_IMPLEMENTATION_TU102;
                break;
            case NV_GPU_ARCHITECTURE_GA100:
                pGpuArchInfo->implementation_id = NV_GPU_ARCH_IMPLEMENTATION_GA102;
                break;
            default:
                return Error(n);
        }

        // Assume first revision, no way to query currently.
        pGpuArchInfo->revision_id = NV_GPU_CHIP_REV_A01;

        return Ok(n);
    }
}
