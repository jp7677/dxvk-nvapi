#include "nvapi_private.h"
#include "util/util_statuscode.h"

extern "C" {
    using namespace dxvk;

    NvAPI_Status __cdecl NvAPI_DRS_CreateSession(NvDRSSessionHandle *phSession) {
        *phSession = reinterpret_cast<NvDRSSessionHandle>(nvapiAdapterRegistry->GetAdapter()); // Just another opaque pointer
        return Ok(__func__);
    }

    NvAPI_Status __cdecl NvAPI_DRS_LoadSettings(NvDRSSessionHandle hSession) {
        return Ok(__func__);
    }

    NvAPI_Status __cdecl NvAPI_DRS_FindProfileByName(NvDRSSessionHandle hSession, NvAPI_UnicodeString profileName, NvDRSProfileHandle* phProfile) {
        return ExecutableNotFound(str::format(__func__, " (", str::fromnvs(profileName), ")"));
    }

    NvAPI_Status __cdecl NvAPI_DRS_FindApplicationByName(NvDRSSessionHandle hSession, NvAPI_UnicodeString appName, NvDRSProfileHandle *phProfile, __inout NVDRS_APPLICATION *pApplication) {
        return ExecutableNotFound(str::format(__func__, " (", str::fromnvs(appName), ")"));
    }

    NvAPI_Status __cdecl NvAPI_DRS_GetBaseProfile(NvDRSSessionHandle hSession, NvDRSProfileHandle *phProfile) {
        return Ok(__func__);
    }

    NvAPI_Status __cdecl NvAPI_DRS_GetSetting(NvDRSSessionHandle hSession, NvDRSProfileHandle hProfile, NvU32 settingId, NVDRS_SETTING *pSetting) {
        *pSetting = NVDRS_SETTING{};
        return Ok(str::format(__func__, " (", settingId, ")"));
    }

    NvAPI_Status __cdecl NvAPI_DRS_DestroySession(NvDRSSessionHandle hSession) {
        return Ok(__func__);
    }
}
