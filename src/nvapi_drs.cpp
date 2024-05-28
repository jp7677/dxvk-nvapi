#include "nvapi_private.h"
#include "../inc/NvApiDriverSettings.c"
#include "util/util_statuscode.h"

static auto drs = 1U;
static auto nvapiDrsSession = reinterpret_cast<NvDRSSessionHandle>(&drs);
static auto nvapiDrsProfile = reinterpret_cast<NvDRSProfileHandle>(&drs);

extern "C" {
    using namespace dxvk;

    NvAPI_Status __cdecl NvAPI_DRS_CreateSession(NvDRSSessionHandle* phSession) {
        constexpr auto n = __func__;

        if (phSession == nullptr)
            return InvalidArgument(n);

        *phSession = nvapiDrsSession;

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_DRS_LoadSettings(NvDRSSessionHandle hSession) {
        return Ok(__func__);
    }

    NvAPI_Status __cdecl NvAPI_DRS_FindProfileByName(NvDRSSessionHandle hSession, NvAPI_UnicodeString profileName, NvDRSProfileHandle* phProfile) {
        return ProfileNotFound(str::format(__func__, " (", str::fromnvus(profileName), ")"));
    }

    NvAPI_Status __cdecl NvAPI_DRS_FindApplicationByName(NvDRSSessionHandle hSession, NvAPI_UnicodeString appName, NvDRSProfileHandle* phProfile, NVDRS_APPLICATION* pApplication) {
        return ExecutableNotFound(str::format(__func__, " (", str::fromnvus(appName), ")"));
    }

    NvAPI_Status __cdecl NvAPI_DRS_GetBaseProfile(NvDRSSessionHandle hSession, NvDRSProfileHandle* phProfile) {
        constexpr auto n = __func__;

        if (phProfile == nullptr)
            return InvalidArgument(n);

        *phProfile = nvapiDrsProfile;

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_DRS_GetCurrentGlobalProfile(NvDRSSessionHandle hSession, NvDRSProfileHandle* phProfile) {
        constexpr auto n = __func__;

        if (phProfile == nullptr)
            return InvalidArgument(n);

        *phProfile = nvapiDrsProfile;

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_DRS_GetSetting(NvDRSSessionHandle hSession, NvDRSProfileHandle hProfile, NvU32 settingId, NVDRS_SETTING* pSetting) {
        constexpr auto n = __func__;

        auto id = str::format("0x", std::hex, settingId);
        auto name = std::string("Unknown");

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

        return SettingNotFound(str::format(n, " (", id, "/", name, ")"));
    }

    NvAPI_Status __cdecl NvAPI_DRS_DestroySession(NvDRSSessionHandle hSession) {
        return Ok(__func__);
    }
}
