#include "nvapi_tests_private.h"
#include "NvApiDriverSettings.h"

using namespace trompeloeil;

TEST_CASE("DRS methods succeed", "[.drs]") {
    SECTION("CreateSession returns OK") {
        NvDRSSessionHandle handle;
        REQUIRE(NvAPI_DRS_CreateSession(&handle) == NVAPI_OK);
    }

    SECTION("LoadSettings returns OK") {
        NvDRSSessionHandle handle{};
        REQUIRE(NvAPI_DRS_LoadSettings(handle) == NVAPI_OK);
    }

    SECTION("FindProfileByName with null profile returns invalid-argument") {
        NvDRSSessionHandle handle{};
        NvAPI_UnicodeString name;
        memcpy(name, L"Profile", 16);
        REQUIRE(NvAPI_DRS_FindProfileByName(handle, name, nullptr) == NVAPI_INVALID_ARGUMENT);
    }

    SECTION("FindProfileByName returns OK") {
        NvDRSSessionHandle handle{};
        NvDRSProfileHandle profile;
        NvAPI_UnicodeString name;
        memcpy(name, L"Profile", 16);
        REQUIRE(NvAPI_DRS_FindProfileByName(handle, name, &profile) == NVAPI_OK);
    }

    SECTION("FindApplicationByName with null profile returns invalid-argument") {
        NvDRSSessionHandle handle{};
        NvAPI_UnicodeString name;
        memcpy(name, L"Application", 24);
        NVDRS_APPLICATION application{};
        application.version = NVDRS_APPLICATION_VER;
        REQUIRE(NvAPI_DRS_FindApplicationByName(handle, name, nullptr, &application) == NVAPI_INVALID_ARGUMENT);
    }

    SECTION("FindApplicationByName with null application returns invalid-argument") {
        NvDRSSessionHandle handle{};
        NvDRSProfileHandle profile;
        NvAPI_UnicodeString name;
        memcpy(name, L"Application", 24);
        REQUIRE(NvAPI_DRS_FindApplicationByName(handle, name, &profile, nullptr) == NVAPI_INVALID_ARGUMENT);
    }

    SECTION("FindApplicationByName with unknown struct version returns incompatible-struct-version") {
        NvDRSSessionHandle handle{};
        NvDRSProfileHandle profile;
        NvAPI_UnicodeString name;
        memcpy(name, L"Application", 24);
        NVDRS_APPLICATION application;
        application.version = NVDRS_APPLICATION_VER + 1;
        REQUIRE(NvAPI_DRS_FindApplicationByName(handle, name, &profile, &application) == NVAPI_INCOMPATIBLE_STRUCT_VERSION);
    }

    SECTION("FindApplicationByName with current struct version returns not incompatible-struct-version") {
        NvDRSSessionHandle handle{};
        NvDRSProfileHandle profile;
        NvAPI_UnicodeString name;
        memcpy(name, L"Application", 24);
        NVDRS_APPLICATION application;
        application.version = NVDRS_APPLICATION_VER;
        REQUIRE(NvAPI_DRS_FindApplicationByName(handle, name, &profile, &application) != NVAPI_INCOMPATIBLE_STRUCT_VERSION);
    }

    SECTION("FindApplicationByName returns OK and fills NVDRS_APPLICATION structure") {
        NvDRSSessionHandle handle{};
        NvDRSProfileHandle profile;
        NvAPI_UnicodeString name;
        memcpy(name, L"Application", 24);
        NvAPI_UnicodeString empty{};
        NVDRS_APPLICATION application;
        application.version = NVDRS_APPLICATION_VER_V4;
        REQUIRE(NvAPI_DRS_FindApplicationByName(handle, name, &profile, &application) == NVAPI_OK);
        CHECK_FALSE(application.isPredefined);
        CHECK_FALSE(memcmp(application.appName, name, sizeof(NvAPI_UnicodeString)));
        CHECK_FALSE(memcmp(application.userFriendlyName, name, sizeof(NvAPI_UnicodeString)));
        CHECK_FALSE(memcmp(application.launcher, empty, sizeof(NvAPI_UnicodeString)));
        CHECK_FALSE(memcmp(application.fileInFolder, empty, sizeof(NvAPI_UnicodeString)));
        CHECK_FALSE(application.isMetro);
        CHECK_FALSE(application.isCommandLine);
        CHECK_FALSE(memcmp(application.commandLine, empty, sizeof(NvAPI_UnicodeString)));
    }

    SECTION("GetBaseProfile returns OK") {
        NvDRSSessionHandle handle{};
        NvDRSProfileHandle profile;
        REQUIRE(NvAPI_DRS_GetBaseProfile(handle, &profile) == NVAPI_OK);
    }

    SECTION("GetCurrentGlobalProfile returns OK") {
        NvDRSSessionHandle handle{};
        NvDRSProfileHandle profile;
        REQUIRE(NvAPI_DRS_GetCurrentGlobalProfile(handle, &profile) == NVAPI_OK);
    }

    SECTION("CreateProfile returns not-supported") {
        NvDRSSessionHandle handle{};
        NVDRS_PROFILE profileInfo{};
        NvDRSProfileHandle profile;
        REQUIRE(NvAPI_DRS_CreateProfile(handle, &profileInfo, &profile) == NVAPI_NOT_SUPPORTED);
    }

    SECTION("GetSetting returns setting-not-found") {
        struct Data {
            int32_t settingId;
        };
        auto args = GENERATE(
            Data{FXAA_ALLOW_ID},
            Data{CUDA_EXCLUDED_GPUS_ID},
            Data{0x12345678});

        NvDRSSessionHandle handle{};
        NvDRSProfileHandle profile{};
        NVDRS_SETTING setting;
        REQUIRE(NvAPI_DRS_GetSetting(handle, profile, args.settingId, &setting) == NVAPI_SETTING_NOT_FOUND);
    }

    SECTION("DestroySession returns OK") {
        NvDRSSessionHandle handle{};
        REQUIRE(NvAPI_DRS_DestroySession(handle) == NVAPI_OK);
    }
}
