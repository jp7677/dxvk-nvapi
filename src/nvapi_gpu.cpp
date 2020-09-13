extern "C" {
    using namespace dxvk;

    NvAPI_Status __cdecl NvAPI_GPU_GetGPUType(NvPhysicalGpuHandle hPhysicalGpu, NV_GPU_TYPE *pGpuType) {
        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized("NvAPI_GPU_GetGPUType");

        if (hPhysicalGpu == 0 || pGpuType == nullptr)
            return InvalidArgument("NvAPI_GPU_GetGPUType");

        auto adapter = nvapiAdapterRegistry->GetAdapter(hPhysicalGpu);
        if (adapter ==  nullptr)
            return ExpectedPhysicalGpuHandle("NvAPI_GPU_GetGPUType");

        *pGpuType = (NV_GPU_TYPE) adapter->GetGpuType();

        return Ok("NvAPI_GPU_GetGPUType");
    }

    NvAPI_Status __cdecl NvAPI_GPU_GetPCIIdentifiers(NvPhysicalGpuHandle hPhysicalGpu, NvU32 *pDeviceId, NvU32 *pSubSystemId, NvU32 *pRevisionId, NvU32 *pExtDeviceId) {
        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized("NvAPI_GPU_GetPCIIdentifiers");

        if (hPhysicalGpu == 0 || pDeviceId == nullptr || pSubSystemId == nullptr || pRevisionId == nullptr || pExtDeviceId == nullptr)
            return InvalidArgument("NvAPI_GPU_GetPCIIdentifiers");

        auto adapter = nvapiAdapterRegistry->GetAdapter(hPhysicalGpu);
        if (adapter ==  nullptr)
            return ExpectedPhysicalGpuHandle("NvAPI_GPU_GetPCIIdentifiers");

        *pDeviceId = adapter->GetDeviceId();
        *pSubSystemId = 0;
        *pRevisionId = 0;
        *pExtDeviceId = adapter->GetDeviceId();

        return Ok("NvAPI_GPU_GetPCIIdentifiers");
    }

    NvAPI_Status __cdecl NvAPI_GPU_GetFullName(NvPhysicalGpuHandle hPhysicalGpu, NvAPI_ShortString szName) {
        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized("NvAPI_GPU_GetFullName");

        if (hPhysicalGpu == 0 || szName == nullptr)
            return InvalidArgument("NvAPI_GPU_GetFullName");

        auto adapter = nvapiAdapterRegistry->GetAdapter(hPhysicalGpu);
        if (adapter ==  nullptr)
            return ExpectedPhysicalGpuHandle("NvAPI_GPU_GetFullName");

        strcpy(szName, adapter->GetDeviceName().c_str());

        return Ok("NvAPI_GPU_GetFullName");
    }
}
