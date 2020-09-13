extern "C" {
    using namespace dxvk;

    NvAPI_Status __cdecl NvAPI_SYS_GetDriverAndBranchVersion(NvU32* pDriverVersion, NvAPI_ShortString szBuildBranchString) {
        constexpr auto n = "NvAPI_SYS_GetDriverAndBranchVersion";
        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (pDriverVersion == nullptr || szBuildBranchString == nullptr)
            return InvalidArgument(n);

        *pDriverVersion = nvapiAdapterRegistry->GetAdapter()->GetDriverVersion();
        strcpy(szBuildBranchString, DXVK_NVAPI_VERSION);

        return Ok(n);
    }
}
