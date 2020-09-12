extern "C" {
    using namespace dxvk;

    NvAPI_Status __cdecl NvAPI_SYS_GetDriverAndBranchVersion(NvU32* pDriverVersion, NvAPI_ShortString szBuildBranchString) {
        *pDriverVersion = nvapiAdapterRegistry->GetAdapter()->GetDriverVersion();
        strcpy(szBuildBranchString, DXVK_NVAPI_VERSION);

        return Ok("NvAPI_SYS_GetDriverAndBranchVersion");
    }
}
