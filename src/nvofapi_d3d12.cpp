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

#include "nvofapi_entrypoints.h"
#include "nvofapi_globals.h"
#include "nvofapi/nvofapi_image.h"
#include "nvofapi/nvofapi_d3d12_instance.h"
#include "util/util_env.h"
#include "util/util_log.h"
#include "util/util_string.h"
#include "util/util_statuscode.h"
#include "../version.h"
#include "../config.h"

static auto initializationMutex = std::mutex{};

extern "C" {

    using namespace dxvk;

    // D3D12 entrypoints
    NV_OF_STATUS NVOFAPI CreateOpticalFlowD3D12(ID3D12Device* pD3D12Device, NvOFHandle* hOFInstance) {
        constexpr auto n = __func__;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(pD3D12Device), log::fmt::ptr(hOFInstance));

        std::scoped_lock lock(initializationMutex);
        if (resourceFactory == nullptr)
            resourceFactory = std::make_unique<ResourceFactory>();

        NvOFInstanceD3D12* nvOF = nullptr;
        try {
            nvOF = new NvOFInstanceD3D12(*resourceFactory, pD3D12Device);
        } catch (std::exception const& e) {
            log::info(str::format("CreateOpticalFlowD3D12 exception, ", e.what()));
            return ErrorGeneric(n);
        }

        if (!nvOF->Initialize()) {
            delete nvOF;
            return ErrorGeneric(n);
        }

        *hOFInstance = reinterpret_cast<NvOFHandle>(nvOF);
        return Success(n);
    }

    NV_OF_STATUS NVOFAPI GetSurfaceFormatCountD3D12(NvOFHandle hOf, const NV_OF_BUFFER_USAGE bufferUsage, const NV_OF_MODE ofMode, uint32_t* const pCount) {
        constexpr auto n = __func__;

        if (log::tracing())
            log::trace(n, log::fmt::hnd(hOf), bufferUsage, ofMode, log::fmt::ptr(pCount));

        *pCount = 1;
        return Success(n);
    }

    NV_OF_STATUS NVOFAPI GetSurfaceFormatD3D12(NvOFHandle hOf, const NV_OF_BUFFER_USAGE bufferUsage, const NV_OF_MODE ofMode, DXGI_FORMAT* const pFormat) {
        constexpr auto n = __func__;

        if (log::tracing())
            log::trace(n, log::fmt::hnd(hOf), bufferUsage, ofMode, log::fmt::ptr(pFormat));

        if (bufferUsage == NV_OF_BUFFER_USAGE_INPUT)
            *pFormat = DXGI_FORMAT_R8_UNORM;
        else
            *pFormat = DXGI_FORMAT_R16G16_SINT;
        return Success(n);
    }

    NV_OF_STATUS NVOFAPI RegisterResourceD3D12(NvOFHandle hOf, NV_OF_REGISTER_RESOURCE_PARAMS_D3D12* registerParams) {
        constexpr auto n = __func__;

        if (log::tracing())
            log::trace(n, log::fmt::hnd(hOf), log::fmt::ptr(registerParams));

        auto nvOF = reinterpret_cast<NvOFInstanceD3D12*>(hOf);

        nvOF->RegisterBuffer(registerParams);
        return Success(n);
    }

    NV_OF_STATUS NVOFAPI UnregisterResourceD3D12(NV_OF_UNREGISTER_RESOURCE_PARAMS_D3D12* registerParams) {
        constexpr auto n = __func__;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(registerParams));

        auto nvRes = reinterpret_cast<NvOFImage*>(registerParams->hOFGpuBuffer);
        delete nvRes;
        return Success(n);
    }

    NV_OF_STATUS NVOFAPI ExecuteD3D12(NvOFHandle hOf, const NV_OF_EXECUTE_INPUT_PARAMS_D3D12* executeInParams, NV_OF_EXECUTE_OUTPUT_PARAMS_D3D12* executeOutParams) {
        constexpr auto n = __func__;
        thread_local bool alreadyLoggedOk = false;

        if (log::tracing())
            log::trace(n, log::fmt::hnd(hOf), log::fmt::ptr(executeInParams), log::fmt::ptr(executeOutParams));

        auto nvOF = reinterpret_cast<NvOFInstanceD3D12*>(hOf);

        nvOF->Execute(executeInParams, executeOutParams);
        return Success(n, alreadyLoggedOk);
    }

    // ETBLs
    NV_OF_STATUS NVOFAPI NvOFAPICreateInstanceD3D12(uint32_t apiVer, NV_OF_D3D12_API_FUNCTION_LIST* functionList) {
        uint32_t apiVerMajor = (apiVer & 0xfffffff0) >> 4;
        uint32_t apiVerMinor = (apiVer & 0xf);
        constexpr auto n = __func__;

        if (log::tracing())
            log::trace(n, apiVer, log::fmt::ptr(functionList));

        log::info(str::format(
            "DXVK-NVAPI ", DXVK_NVAPI_VERSION,
            " NVOFAPI/D3D12",
            " ", DXVK_NVAPI_BUILD_COMPILER,
            " ", DXVK_NVAPI_BUILD_COMPILER_VERSION,
            " ", DXVK_NVAPI_BUILD_TARGET,
            " ", DXVK_NVAPI_BUILD_TYPE,
            " (", env::getExecutableName(), ")"));
        log::info(str::format("OFAPI Client Version: ", apiVerMajor, ".", apiVerMinor));

        if (apiVerMajor != 5)
            return InvalidVersion(n);

        functionList->nvCreateOpticalFlowD3D12 = CreateOpticalFlowD3D12;
        functionList->nvOFInit = OFSessionInit;
        functionList->nvOFGetSurfaceFormatCountD3D12 = GetSurfaceFormatCountD3D12;
        functionList->nvOFGetSurfaceFormatD3D12 = GetSurfaceFormatD3D12;
        functionList->nvOFRegisterResourceD3D12 = RegisterResourceD3D12;
        functionList->nvOFUnregisterResourceD3D12 = UnregisterResourceD3D12;
        functionList->nvOFExecuteD3D12 = ExecuteD3D12;
        functionList->nvOFDestroy = OFSessionDestroy;
        functionList->nvOFGetLastError = OFSessionGetLastError;
        functionList->nvOFGetCaps = OFSessionGetCaps;

        return Success(n);
    }
}
