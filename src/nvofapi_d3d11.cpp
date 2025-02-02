#include "nvofapi_private.h"
#include "util/util_log.h"
#include "util/util_statuscode.h"

extern "C" {

    using namespace dxvk;

    NV_OF_STATUS NVOFAPI NvOFAPICreateInstanceD3D11(uint32_t apiVer, NV_OF_D3D11_API_FUNCTION_LIST* functionList) {
        constexpr auto n = __func__;

        if (log::tracing())
            log::trace(n, apiVer, log::fmt::ptr(functionList));

        return OFNotAvailable(n);
    }
}
