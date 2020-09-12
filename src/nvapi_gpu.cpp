extern "C" {
    using namespace dxvk;

    NvAPI_Status __cdecl NvAPI_GPU_GetGPUType(NvPhysicalGpuHandle hPhysicalGpu, NV_GPU_TYPE *pGpuType) {
        auto adapter = nvapiAdapterRegistry->GetAdapter(hPhysicalGpu);
        if (adapter ==  nullptr)
            return ExpectedPhysicalGpuHandle("NvAPI_GPU_GetGPUType");

        *pGpuType = (NV_GPU_TYPE) adapter->GetGpuType();

        return Ok("NvAPI_GPU_GetGPUType");
    }

    NvAPI_Status __cdecl NvAPI_GPU_GetPCIIdentifiers(NvPhysicalGpuHandle hPhysicalGpu, NvU32 *pDeviceId, NvU32 *pSubSystemId, NvU32 *pRevisionId, NvU32 *pExtDeviceId) {
        auto adapter = nvapiAdapterRegistry->GetAdapter(hPhysicalGpu);
        if (adapter ==  nullptr)
            return ExpectedPhysicalGpuHandle("NvAPI_GPU_GetGPUType");

        *pDeviceId = adapter->GetDeviceId();
        *pSubSystemId = 0;
        *pRevisionId = 0;
        *pExtDeviceId = adapter->GetDeviceId();

        return Ok("NvAPI_GPU_GetPCIIdentifiers");
    }

    NvAPI_Status __cdecl NvAPI_GPU_GetFullName(NvPhysicalGpuHandle hPhysicalGpu, NvAPI_ShortString szName) {
        auto adapter = nvapiAdapterRegistry->GetAdapter(hPhysicalGpu);
        if (adapter ==  nullptr)
            return ExpectedPhysicalGpuHandle("NvAPI_GPU_GetGPUType");

        strcpy(szName, adapter->GetDeviceName().c_str());

        return Ok("NvAPI_GPU_GetFullName");
    }
}
