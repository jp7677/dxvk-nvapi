#include "nvapi_private.h"
#include "NvApiDriverSettings.c"
#include "util/util_env.h"
#include "util/util_statuscode.h"
#include "util/util_string.h"

static auto drs = 1U;
static auto nvapiDrsSession = reinterpret_cast<NvDRSSessionHandle>(&drs);
static auto nvapiDrsProfile = reinterpret_cast<NvDRSProfileHandle>(&drs);

extern "C" {
    using namespace dxvk;

    NvAPI_Status __cdecl NvAPI_DRS_CreateSession(NvDRSSessionHandle* phSession) {
        constexpr auto n = __func__;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(phSession));

        if (!phSession)
            return InvalidArgument(n);

        *phSession = nvapiDrsSession;

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_DRS_LoadSettings(NvDRSSessionHandle hSession) {
        constexpr auto n = __func__;

        if (log::tracing())
            log::trace(n, log::fmt::hnd(hSession));

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_DRS_FindProfileByName(NvDRSSessionHandle hSession, NvAPI_UnicodeString profileName, NvDRSProfileHandle* phProfile) {
        constexpr auto n = __func__;

        if (log::tracing())
            log::trace(n, log::fmt::hnd(hSession), log::fmt::ptr(profileName), log::fmt::ptr(phProfile));

        if (!phProfile)
            return InvalidArgument(n);

        *phProfile = nvapiDrsProfile;

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_DRS_FindApplicationByName(NvDRSSessionHandle hSession, NvAPI_UnicodeString appName, NvDRSProfileHandle* phProfile, NVDRS_APPLICATION* pApplication) {
        constexpr auto n = __func__;

        if (log::tracing())
            log::trace(n, log::fmt::hnd(hSession), log::fmt::ptr(appName), log::fmt::ptr(phProfile), log::fmt::ptr(pApplication));

        if (!phProfile || !pApplication)
            return InvalidArgument(n);

        switch (pApplication->version) {
            case NVDRS_APPLICATION_VER_V4:
                std::memset(pApplication->commandLine, 0, sizeof(pApplication->commandLine));
                [[fallthrough]];
            case NVDRS_APPLICATION_VER_V3:
                pApplication->isCommandLine = 0;
                pApplication->isMetro = 0;
                [[fallthrough]];
            case NVDRS_APPLICATION_VER_V2:
                std::memset(pApplication->fileInFolder, 0, sizeof(pApplication->fileInFolder));
                [[fallthrough]];
            case NVDRS_APPLICATION_VER_V1:
                std::memset(pApplication->launcher, 0, sizeof(pApplication->launcher));
                std::memcpy(pApplication->userFriendlyName, appName, sizeof(NvAPI_UnicodeString));
                std::memcpy(pApplication->appName, appName, sizeof(NvAPI_UnicodeString));
                pApplication->isPredefined = 0;
                break;
            default:
                return IncompatibleStructVersion(n, pApplication->version);
        }

        *phProfile = nvapiDrsProfile;

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_DRS_GetBaseProfile(NvDRSSessionHandle hSession, NvDRSProfileHandle* phProfile) {
        constexpr auto n = __func__;

        if (log::tracing())
            log::trace(n, log::fmt::hnd(hSession), log::fmt::ptr(phProfile));

        if (!phProfile)
            return InvalidArgument(n);

        *phProfile = nvapiDrsProfile;

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_DRS_GetCurrentGlobalProfile(NvDRSSessionHandle hSession, NvDRSProfileHandle* phProfile) {
        constexpr auto n = __func__;

        if (log::tracing())
            log::trace(n, log::fmt::hnd(hSession), log::fmt::ptr(phProfile));

        if (!phProfile)
            return InvalidArgument(n);

        *phProfile = nvapiDrsProfile;

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_DRS_CreateProfile(NvDRSSessionHandle hSession, NVDRS_PROFILE* pProfileInfo, NvDRSProfileHandle* phProfile) {
        constexpr auto n = __func__;

        if (log::tracing())
            log::trace(n, log::fmt::hnd(hSession), log::fmt::ptr(pProfileInfo), log::fmt::ptr(phProfile));

        return NotSupported(n);
    }

    NvAPI_Status __cdecl NvAPI_DRS_GetSetting(NvDRSSessionHandle hSession, NvDRSProfileHandle hProfile, NvU32 settingId, NVDRS_SETTING* pSetting) {
        constexpr auto n = __func__;
        static const auto nvapiDrsSettingsString = dxvk::env::getEnvVariable("DXVK_NVAPI_DRS_SETTINGS");
        static const auto nvapiDrsDwords = dxvk::str::parsedwords(nvapiDrsSettingsString);

        if (log::tracing())
            log::trace(n, log::fmt::hnd(hSession), log::fmt::hnd(hProfile), settingId, log::fmt::ptr(pSetting));

        if (!pSetting)
            return InvalidArgument(n);

        if (pSetting->version != NVDRS_SETTING_VER1)
            return IncompatibleStructVersion(n, pSetting->version);

        auto id = str::format("0x", std::hex, settingId);
        auto name = std::string("Unknown");

        auto itD = std::find_if(
            std::begin(mapSettingDWORD),
            std::end(mapSettingDWORD),
            [&settingId](const auto& item) { return item.settingId == settingId; });
        if (itD != std::end(mapSettingDWORD))
            name = str::fromws(itD->settingNameString);

        if (auto it = nvapiDrsDwords.find(settingId); it != nvapiDrsDwords.end()) {
            auto value = it->second;
            pSetting->settingId = settingId;
            pSetting->settingType = NVDRS_DWORD_TYPE;
            pSetting->settingLocation = NVDRS_CURRENT_PROFILE_LOCATION;
            pSetting->isCurrentPredefined = 0;
            pSetting->isPredefinedValid = 1;
            pSetting->u32CurrentValue = value;

            if (itD != std::end(mapSettingDWORD)) {
                std::memcpy(pSetting->settingName, itD->settingNameString, sizeof(pSetting->settingName));
                pSetting->u32PredefinedValue = itD->defaultValue;
            } else {
                std::memset(pSetting->settingName, 0, sizeof(pSetting->settingName));
                pSetting->u32PredefinedValue = 0;
            }

            return Ok(str::format(n, " (", id, "/", name, " = 0x", std::hex, value, ")"));
        }

        auto itW = std::find_if(
            std::begin(mapSettingWSTRING),
            std::end(mapSettingWSTRING),
            [&settingId](const auto& item) { return item.settingId == settingId; });
        if (itW != std::end(mapSettingWSTRING))
            name = str::fromws(itW->settingNameString);

        return SettingNotFound(str::format(n, " (", id, "/", name, ")"));
    }

    NvAPI_Status __cdecl NvAPI_DRS_DestroySession(NvDRSSessionHandle hSession) {
        constexpr auto n = __func__;

        if (log::tracing())
            log::trace(n, log::fmt::hnd(hSession));

        return Ok(n);
    }
}
