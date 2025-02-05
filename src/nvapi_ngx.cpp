#include "nvapi_private.h"
#include "util/util_statuscode.h"

extern "C" {
    using namespace dxvk;

    NvAPI_Status __cdecl NvAPI_NGX_GetNGXOverrideState(NV_NGX_DLSS_OVERRIDE_GET_STATE_PARAMS* pGetOverrideStateParams) {
        constexpr auto n = __func__;

        if (log::tracing())
            log::trace(n, log::fmt::ngx_dlss_override_get_state_params(pGetOverrideStateParams));

        if (pGetOverrideStateParams->version != NV_NGX_DLSS_OVERRIDE_GET_STATE_PARAMS_VER1)
            return IncompatibleStructVersion(n, pGetOverrideStateParams->version);

        pGetOverrideStateParams->feedbackMaskSR = NV_NGX_DLSS_OVERRIDE_FLAG_ERR_FAILED;
        pGetOverrideStateParams->feedbackMaskRR = NV_NGX_DLSS_OVERRIDE_FLAG_ERR_FAILED;
        pGetOverrideStateParams->feedbackMaskFG = NV_NGX_DLSS_OVERRIDE_FLAG_ERR_FAILED;

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_NGX_SetNGXOverrideState(NV_NGX_DLSS_OVERRIDE_SET_STATE_PARAMS* pSetOverrideStateParams) {
        constexpr auto n = __func__;

        if (log::tracing())
            log::trace(n, log::fmt::ngx_dlss_override_set_state_params(pSetOverrideStateParams));

        if (pSetOverrideStateParams->version != NV_NGX_DLSS_OVERRIDE_SET_STATE_PARAMS_VER1)
            return IncompatibleStructVersion(n, pSetOverrideStateParams->version);

        return Ok(n);
    }
}
