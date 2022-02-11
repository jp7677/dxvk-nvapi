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

    SECTION("FindProfileByName returns profile-not-found") {
        NvDRSSessionHandle handle{};
        NvDRSProfileHandle profile;
        NvAPI_UnicodeString name{};
        memcpy(name, L"Profile", 16);
        REQUIRE(NvAPI_DRS_FindProfileByName(handle, name, &profile) == NVAPI_PROFILE_NOT_FOUND);
    }

    SECTION("FindApplicationByName returns executable-not-found") {
        NvDRSSessionHandle handle{};
        NvDRSProfileHandle profile;
        NvAPI_UnicodeString name;
        memcpy(name, L"Application", 24);
        NVDRS_APPLICATION application;
        REQUIRE(NvAPI_DRS_FindApplicationByName(handle, name, &profile, &application) == NVAPI_EXECUTABLE_NOT_FOUND);
    }

    SECTION("GetBaseProfile returns OK") {
        NvDRSSessionHandle handle{};
        NvDRSProfileHandle profile;
        REQUIRE(NvAPI_DRS_GetBaseProfile(handle, &profile) == NVAPI_OK);
    }

    SECTION("GetSetting returns setting-not-found") {
        struct Data {int32_t settingId;};
        auto args = GENERATE(
            Data{FXAA_ALLOW_ID},
            Data{CUDA_EXCLUDED_GPUS_ID},
            Data{0x1234});

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
