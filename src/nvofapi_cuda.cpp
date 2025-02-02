#include "nvofapi_private.h"
#include "util/util_log.h"
#include "util/util_statuscode.h"

extern "C" {

    using namespace dxvk;

    NV_OF_STATUS NVOFAPI NvOFAPICreateInstanceCuda(uint32_t apiVer, void* functionList) {
        constexpr auto n = __func__;

        if (log::tracing())
            log::trace(n, apiVer, log::fmt::ptr(functionList));

        return OFNotAvailable(n);
    }
}
