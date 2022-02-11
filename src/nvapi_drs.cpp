#include "../inc/NvApiDriverSettings.c"
#include "nvapi_private.h"
#include "util/util_statuscode.h"

extern "C" {
    using namespace dxvk;

    NvAPI_Status __cdecl NvAPI_DRS_CreateSession(NvDRSSessionHandle *phSession) {
        constexpr auto n = __func__;

        *phSession = reinterpret_cast<NvDRSSessionHandle>(nvapiAdapterRegistry->GetAdapter()); // Just another opaque pointer, we don't use it anywhere

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_DRS_LoadSettings(NvDRSSessionHandle hSession) {
        return Ok(__func__);
    }

    NvAPI_Status __cdecl NvAPI_DRS_FindProfileByName(NvDRSSessionHandle hSession, NvAPI_UnicodeString profileName, NvDRSProfileHandle* phProfile) {
        return ProfileNotFound(str::format(__func__, " (", str::fromnvs(profileName), ")"));
    }

    NvAPI_Status __cdecl NvAPI_DRS_FindApplicationByName(NvDRSSessionHandle hSession, NvAPI_UnicodeString appName, NvDRSProfileHandle *phProfile, __inout NVDRS_APPLICATION *pApplication) {
        constexpr auto n = __func__;

        return ExecutableNotFound(str::format(n, " (", str::fromnvs(appName), ")"));
    }

    NvAPI_Status __cdecl NvAPI_DRS_GetBaseProfile(NvDRSSessionHandle hSession, NvDRSProfileHandle *phProfile) {
        constexpr auto n = __func__;

        *phProfile = reinterpret_cast<NvDRSProfileHandle>(nvapiAdapterRegistry->GetAdapter()); // Just another opaque pointer, we don't use it anywhere

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_DRS_GetSetting(NvDRSSessionHandle hSession, NvDRSProfileHandle hProfile, NvU32 settingId, NVDRS_SETTING *pSetting) {
        constexpr auto n = __func__;

        auto name = str::format("0x", std::hex, settingId);
        auto itD = std::find_if(
            std::begin(mapSettingDWORD),
            std::end(mapSettingDWORD),
            [&settingId](const auto& item) { return item.settingId == settingId; });
        if (itD != std::end(mapSettingDWORD))
            name = str::fromws(itD->settingNameString);

        auto itW = std::find_if(
            std::begin(mapSettingWSTRING),
            std::end(mapSettingWSTRING),
            [&settingId](const auto& item) { return item.settingId == settingId; });
        if (itW != std::end(mapSettingWSTRING))
            name = str::fromws(itW->settingNameString);

        return SettingNotFound(str::format(n, " (", name, ")"));
    }

    NvAPI_Status __cdecl NvAPI_DRS_DestroySession(NvDRSSessionHandle hSession) {
        return Ok(__func__);
    }
}
