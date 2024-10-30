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
#include "d3d12/nvofapi_d3d12_instance.h"

#include "util/util_env.h"
#include "util/util_log.h"
#include "util/util_string.h"
#include "../version.h"

extern "C" {

    // D3D12 entrypoints
    NV_OF_STATUS NVOFAPI CreateOpticalFlowD3D12(ID3D12Device* pD3D12Device, NvOFHandle* hOFInstance) {
        dxvk::log::info("CreateOpticalFlowD3D12 called");

        nvofapi::NvOFInstanceD3D12* nvOF = nullptr;
        try {
            nvOF = new nvofapi::NvOFInstanceD3D12(pD3D12Device);
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
        nvofapi::NvOFInstanceD3D12* nvOF = reinterpret_cast<nvofapi::NvOFInstanceD3D12*>(hOf);

        return nvOF->RegisterBuffer(registerParams);
    }

    NV_OF_STATUS NVOFAPI UnregisterResourceD3D12(NV_OF_UNREGISTER_RESOURCE_PARAMS_D3D12* registerParams) {
        dxvk::log::info("UnregisterResourceD3D12 called");
        nvofapi::NvOFImage* nvRes = reinterpret_cast<nvofapi::NvOFImage*>(registerParams->hOFGpuBuffer);
        delete nvRes;
        return NV_OF_SUCCESS;
    }

    NV_OF_STATUS NVOFAPI ExecuteD3D12(NvOFHandle hOf, const NV_OF_EXECUTE_INPUT_PARAMS_D3D12* executeInParams, NV_OF_EXECUTE_OUTPUT_PARAMS_D3D12* executeOutParams) {
        dxvk::log::info("ExecuteD3D12 called");
        dxvk::log::info(dxvk::str::format("handle: %p", hOf));
        nvofapi::NvOFInstanceD3D12* nvOF = reinterpret_cast<nvofapi::NvOFInstanceD3D12*>(hOf);

        return nvOF->Execute(executeInParams, executeOutParams);
    }

    // ETBLs
    NV_OF_STATUS __stdcall NvOFAPICreateInstanceD3D12(uint32_t apiVer, NV_OF_D3D12_API_FUNCTION_LIST* functionList) {
        uint32_t apiVerMajor = (apiVer & 0xfffffff0) >> 4;
        uint32_t apiVerMinor = (apiVer & 0xf);
        constexpr auto n = __func__;

        dxvk::log::info(dxvk::str::format("DXVK-NVOFAPI ", DXVK_NVAPI_VERSION, " (", dxvk::env::getExecutableName(), ") D3D12"));
        dxvk::log::info(dxvk::str::format("OFAPI Client Version: ", apiVerMajor, ".", apiVerMinor));

        if (apiVerMajor != 5)
            return NV_OF_ERR_INVALID_VERSION;

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
