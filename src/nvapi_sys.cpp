extern "C" {
    using namespace dxvk;

    NvAPI_Status __cdecl NvAPI_SYS_GetDriverAndBranchVersion(NvU32* pDriverVersion, NvAPI_ShortString szBuildBranchString) {
        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized("NvAPI_SYS_GetDriverAndBranchVersion");

        if (pDriverVersion == nullptr || szBuildBranchString == nullptr)
            return InvalidArgument("NvAPI_SYS_GetDriverAndBranchVersion");

        *pDriverVersion = nvapiAdapterRegistry->GetAdapter()->GetDriverVersion();
        strcpy(szBuildBranchString, DXVK_NVAPI_VERSION);

        return Ok("NvAPI_SYS_GetDriverAndBranchVersion");
    }
}
