extern "C" {
    using namespace dxvk;

    NvAPI_Status __cdecl NvAPI_GPU_GetGPUType(NvPhysicalGpuHandle hPhysicalGpu, NV_GPU_TYPE *pGpuType) {
        constexpr auto n = "NvAPI_GPU_GetGPUType";

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (hPhysicalGpu == 0 || pGpuType == nullptr)
            return InvalidArgument(n);

        auto adapter = nvapiAdapterRegistry->GetAdapter(hPhysicalGpu);
        if (adapter ==  nullptr)
            return ExpectedPhysicalGpuHandle(n);

        *pGpuType = (NV_GPU_TYPE) adapter->GetGpuType();

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_GPU_GetPCIIdentifiers(NvPhysicalGpuHandle hPhysicalGpu, NvU32 *pDeviceId, NvU32 *pSubSystemId, NvU32 *pRevisionId, NvU32 *pExtDeviceId) {
        constexpr auto n = "NvAPI_GPU_GetPCIIdentifiers";

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (hPhysicalGpu == 0 || pDeviceId == nullptr || pSubSystemId == nullptr || pRevisionId == nullptr || pExtDeviceId == nullptr)
            return InvalidArgument(n);

        auto adapter = nvapiAdapterRegistry->GetAdapter(hPhysicalGpu);
        if (adapter ==  nullptr)
            return ExpectedPhysicalGpuHandle(n);

        *pDeviceId = adapter->GetDeviceId();
        *pSubSystemId = 0;
        *pRevisionId = 0;
        *pExtDeviceId = adapter->GetDeviceId();

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_GPU_GetFullName(NvPhysicalGpuHandle hPhysicalGpu, NvAPI_ShortString szName) {
        constexpr auto n = "NvAPI_GPU_GetFullName";

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (hPhysicalGpu == 0 || szName == nullptr)
            return InvalidArgument(n);

        auto adapter = nvapiAdapterRegistry->GetAdapter(hPhysicalGpu);
        if (adapter ==  nullptr)
            return ExpectedPhysicalGpuHandle(n);

        strcpy(szName, adapter->GetDeviceName().c_str());

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_GPU_GetBusId(NvPhysicalGpuHandle hPhysicalGpu, NvU32 *pBusId) {
        constexpr auto n = "NvAPI_GPU_GetBusId";

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (hPhysicalGpu == 0 || pBusId == nullptr)
            return InvalidArgument(n);

        auto adapter = nvapiAdapterRegistry->GetAdapter(hPhysicalGpu);
        if (adapter ==  nullptr)
            return ExpectedPhysicalGpuHandle(n);

        *pBusId = adapter->GetBusId();

        return Ok(n);
    }
}
