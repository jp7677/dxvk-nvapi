/*
 * SPDX-FileCopyrightText: Copyright (c) 2024 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "nvofapi/nvofapi_instance.h"
#include "nvofapi_entrypoints.h"
#include "util/util_log.h"
#include "util/util_statuscode.h"

using namespace dxvk;

extern "C" {
    // Common entrypoints
    NV_OF_STATUS NVOFAPI OFSessionInit(NvOFHandle hOf, const NV_OF_INIT_PARAMS* initParams) {
        constexpr auto n = __func__;

        if (log::tracing())
            log::trace(n, log::fmt::hnd(hOf), log::fmt::ptr(initParams));

        auto nvOF = reinterpret_cast<NvOFInstance*>(hOf);

        if (!nvOF) {
            return ErrorGeneric(n);
        }

        return nvOF->InitSession(initParams);
    }

    NV_OF_STATUS NVOFAPI OFSessionDestroy(NvOFHandle hOf) {
        constexpr auto n = __func__;

        if (log::tracing())
            log::trace(n, log::fmt::hnd(hOf));

        auto nvOF = reinterpret_cast<NvOFInstance*>(hOf);
        if (!nvOF) {
            return ErrorGeneric(n);
        }

        delete nvOF;
        return Success(n);
    }

    NV_OF_STATUS NVOFAPI OFSessionGetLastError(NvOFHandle hOf, char lastError[], uint32_t* size) {
        constexpr auto n = __func__;

        if (log::tracing())
            log::trace(n, log::fmt::hnd(hOf), log::fmt::ptr(lastError), log::fmt::ptr(size));

        return ErrorGeneric(n);
    }

    NV_OF_STATUS NVOFAPI OFSessionGetCaps(NvOFHandle hOf, NV_OF_CAPS capsParam, uint32_t* capsVal, uint32_t* size) {
        constexpr auto n = __func__;

        if (log::tracing())
            log::trace(n, log::fmt::hnd(hOf), capsParam, log::fmt::ptr(capsVal), log::fmt::ptr(size));

        auto nvOF = reinterpret_cast<NvOFInstance*>(hOf);

        return nvOF->GetCaps(capsParam, capsVal, size);
    }

    NV_OF_STATUS NVOFAPI NvOFGetMaxSupportedApiVersion(uint32_t* version) {
        constexpr auto n = __func__;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(version));

        if (!version)
            return InvalidPtr(n);

        *version = NV_OF_API_VERSION;
        return Success(n);
    }
}
