#include "nvapi_tests_private.h"

using namespace trompeloeil;

TEST_CASE("NGX methods succeed", "[.ngx]") {
    SECTION("GetNGXOverrideState returns OK") {
        NV_NGX_DLSS_OVERRIDE_GET_STATE_PARAMS params{};
        params.version = NV_NGX_DLSS_OVERRIDE_GET_STATE_PARAMS_VER1;
        REQUIRE(NvAPI_NGX_GetNGXOverrideState(&params) == NVAPI_OK);
        REQUIRE(params.feedbackMaskSR == NV_NGX_DLSS_OVERRIDE_FLAG_ERR_FAILED);
        REQUIRE(params.feedbackMaskRR == NV_NGX_DLSS_OVERRIDE_FLAG_ERR_FAILED);
        REQUIRE(params.feedbackMaskFG == NV_NGX_DLSS_OVERRIDE_FLAG_ERR_FAILED);
    }

    SECTION("SetNGXOverrideState returns OK") {
        NV_NGX_DLSS_OVERRIDE_SET_STATE_PARAMS params{};
        params.version = NV_NGX_DLSS_OVERRIDE_SET_STATE_PARAMS_VER1;
        REQUIRE(NvAPI_NGX_SetNGXOverrideState(&params) == NVAPI_OK);
    }

    SECTION("GetDriverFeatureSupport succeeds") {
        SECTION("GetDriverFeatureSupport returns OK") {
            NV_NGX_GET_DRIVER_FEATURE_SUPPORT_PARAMS params{};
            params.version = NV_NGX_GET_DRIVER_FEATURE_SUPPORT_PARAMS_VER1;
            params.featureCount = 1;
            params.featureSupportInfo[0].featureId = NV_NGX_DRIVER_FEATURE_ID_SET_FLIP_CONFIG_V2;
            REQUIRE(NvAPI_NGX_GetDriverFeatureSupport(&params) == NVAPI_OK);
            REQUIRE(params.featureSupportInfo[0].bSupported == NV_TRUE);
        }

        SECTION("GetDriverFeatureSupport with current struct version returns not incompatible-struct-version") {
            // This test should fail when a header update provides a newer not yet implemented struct version
            NV_NGX_GET_DRIVER_FEATURE_SUPPORT_PARAMS params{};
            params.version = NV_NGX_GET_DRIVER_FEATURE_SUPPORT_PARAMS_VER;
            REQUIRE(NvAPI_NGX_GetDriverFeatureSupport(&params) != NVAPI_INCOMPATIBLE_STRUCT_VERSION);
        }

        SECTION("GetDriverFeatureSupport with unknown struct version returns incompatible-struct-version") {
            NV_NGX_GET_DRIVER_FEATURE_SUPPORT_PARAMS params{};
            params.version = NV_NGX_GET_DRIVER_FEATURE_SUPPORT_PARAMS_VER1 + 1;
            REQUIRE(NvAPI_NGX_GetDriverFeatureSupport(&params) == NVAPI_INCOMPATIBLE_STRUCT_VERSION);
        }

        SECTION("GetDriverFeatureSupport with invalid feature count returns invalid-argument") {
            NV_NGX_GET_DRIVER_FEATURE_SUPPORT_PARAMS params{};
            params.version = NV_NGX_GET_DRIVER_FEATURE_SUPPORT_PARAMS_VER1;
            params.featureCount = NVAPI_MAX_NGX_FEATURES_PER_QUERY + 1;
            REQUIRE(NvAPI_NGX_GetDriverFeatureSupport(&params) == NVAPI_INVALID_ARGUMENT);
        }

        SECTION("GetDriverFeatureSupport with null argument returns invalid-pointer") {
            REQUIRE(NvAPI_NGX_GetDriverFeatureSupport(nullptr) == NVAPI_INVALID_POINTER);
        }
    }
}
