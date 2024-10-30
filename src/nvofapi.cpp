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

#include "nvofapi/nvofapi_entrypoints.h"

#include "util/util_env.h"
#include "util/util_log.h"
#include "util/util_string.h"
#include "../version.h"

extern "C" {
    // Common entrypoints
    NV_OF_STATUS NVOFAPI OFSessionInit(NvOFHandle hOf, const NV_OF_INIT_PARAMS* initParams) {
        dxvk::log::info("OFSessionInit called");
        nvofapi::NvOFInstance* nvOF = reinterpret_cast<nvofapi::NvOFInstance*>(hOf);

        if (!nvOF) {
            dxvk::log::info(dxvk::str::format("OFSessionInit failed!"));
            return NV_OF_ERR_GENERIC;
        }

        return nvOF->InitSession(initParams);
    }

    NV_OF_STATUS NVOFAPI OFSessionDestroy(NvOFHandle hOf) {
        dxvk::log::info("OFSessionDestroy called");
        nvofapi::NvOFInstance* nvOF = reinterpret_cast<nvofapi::NvOFInstance*>(hOf);
        if (!nvOF) {
            dxvk::log::info(dxvk::str::format("OFSessionDestroy failed!"));
            return NV_OF_ERR_GENERIC;
        }

        delete nvOF;
        return NV_OF_SUCCESS;
    }

    NV_OF_STATUS NVOFAPI OFSessionGetLastError(NvOFHandle hOf, char lastError[], uint32_t* size) {
        dxvk::log::info("OFSessionGetLastError called");
        dxvk::log::info(dxvk::str::format("OFSessionGetLastError failed!"));
        return NV_OF_ERR_GENERIC;
    }

    NV_OF_STATUS NVOFAPI OFSessionGetCaps(NvOFHandle hOf, NV_OF_CAPS capsParam, uint32_t* capsVal, uint32_t* size) {
        dxvk::log::info("OFSessionGetCaps called");
        nvofapi::NvOFInstance* nvOF = reinterpret_cast<nvofapi::NvOFInstance*>(hOf);

        return nvOF->getCaps(capsParam, capsVal, size);
    }

    NV_OF_STATUS __cdecl NvOFGetMaxSupportedApiVersion(uint32_t* version) {
        if (!version)
            return NV_OF_ERR_INVALID_PTR;

        *version = NV_OF_API_VERSION;
        return NV_OF_SUCCESS;
    }
}
