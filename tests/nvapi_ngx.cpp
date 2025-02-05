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
}
