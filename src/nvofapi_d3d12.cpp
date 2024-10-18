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

#include "d3d12/nvofapi_d3d12_instance.h"

#include "util/util_statuscode.h"
#include "util/util_env.h"
#include "util/util_log.h"
#include "util/util_string.h"
#include "../version.h"

extern "C" {
    NV_OF_STATUS OFSessionInit(NvOFHandle hOf, const NV_OF_INIT_PARAMS* initParams);
    NV_OF_STATUS OFSessionDestroy(NvOFHandle hOf);
    NV_OF_STATUS OFSessionGetLastError(NvOFHandle hOf, char lastError[], uint32_t* size);
    NV_OF_STATUS OFSessionGetCaps(NvOFHandle hOf, NV_OF_CAPS capsParam, uint32_t* capsVal, uint32_t* size);

    using namespace dxvk;
    using namespace dxvk::status::nvofapi;

    NV_OF_STATUS NVOFAPI CreateOpticalFlowD3D12(ID3D12Device* pD3D12Device, NvOFHandle* hOFInstance) {
        dxvk::log::info("CreateOpticalFlowD3D12 called");

        nvofapi::NvofapiD3d12Instance* nvOF = nullptr;
        try {
            nvOF = new nvofapi::NvofapiD3d12Instance(pD3D12Device);
        } catch (std::exception e) {
            dxvk::log::info(dxvk::str::format("CreateOpticalFlowD3D12 exception, %s", e.what()));
        }

        if (!nvOF) {
            dxvk::log::info(dxvk::str::format("CreateOpticalFlowD3D12 failed allocation"));
            return NV_OF_ERR_GENERIC;
        }

        if (!nvOF->Initialize()) {
            dxvk::log::info(dxvk::str::format("CreateOpticalFlowD3D12 failed creation"));
            delete nvOF;
            return NV_OF_ERR_GENERIC;
        }

        *hOFInstance = reinterpret_cast<NvOFHandle>(nvOF);
        dxvk::log::info(dxvk::str::format("CreateOpticalFlowD3D12 end"));
        return NV_OF_SUCCESS;
    }

    NV_OF_STATUS NVOFAPI GetSurfaceFormatCountD3D12(NvOFHandle hOf, const NV_OF_BUFFER_USAGE bufferUsage, const NV_OF_MODE ofMode, uint32_t* const pCount) {
        dxvk::log::info("GetSurfaceFormatCountD3D12 called");
        *pCount = 1;
        return NV_OF_SUCCESS;
    }

    NV_OF_STATUS NVOFAPI GetSurfaceFormatD3D12(NvOFHandle hOf, const NV_OF_BUFFER_USAGE bufferUsage, const NV_OF_MODE ofMode, DXGI_FORMAT* const pFormat) {
        dxvk::log::info("GetSurfaceFormatD3D12 called");
        if (bufferUsage == NV_OF_BUFFER_USAGE_INPUT)
            *pFormat = DXGI_FORMAT_R8_UNORM;
        else
            *pFormat = DXGI_FORMAT_R16G16_SINT;
        return NV_OF_SUCCESS;
    }

    NV_OF_STATUS NVOFAPI RegisterResourceD3D12(NvOFHandle hOf, NV_OF_REGISTER_RESOURCE_PARAMS_D3D12* registerParams) {
        dxvk::log::info("RegisterResourceD3D12 called");
        auto nvOF = reinterpret_cast<nvofapi::NvofapiD3d12Instance*>(hOf);

        return nvOF->RegisterBuffer(registerParams);
    }

    NV_OF_STATUS NVOFAPI UnregisterResourceD3D12(NV_OF_UNREGISTER_RESOURCE_PARAMS_D3D12* registerParams) {
        dxvk::log::info("UnregisterResourceD3D12 called");
        auto nvRes = reinterpret_cast<nvofapi::NvOFImageVk*>(registerParams->hOFGpuBuffer);
        delete nvRes;
        return NV_OF_SUCCESS;
    }

    NV_OF_STATUS NVOFAPI ExecuteD3D12(NvOFHandle hOf, const NV_OF_EXECUTE_INPUT_PARAMS_D3D12* executeInParams, NV_OF_EXECUTE_OUTPUT_PARAMS_D3D12* executeOutParams) {
        dxvk::log::info("ExecuteD3D12 called");
        dxvk::log::info(dxvk::str::format("handle: %p", hOf));
        auto nvOF = reinterpret_cast<nvofapi::NvofapiD3d12Instance*>(hOf);

        return nvOF->Execute(executeInParams, executeOutParams);
    }

    NV_OF_STATUS __stdcall NvOFAPICreateInstanceD3D12(uint32_t apiVer, NV_OF_D3D12_API_FUNCTION_LIST* functionList) {
        constexpr auto n = __func__;

        dxvk::log::info(dxvk::str::format("DXVK-NVOFAPI ", DXVK_NVAPI_VERSION, " (", dxvk::env::getExecutableName(), ") D3D12"));

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

        return NV_OF_SUCCESS;
    }
}
