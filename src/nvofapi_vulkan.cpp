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
#include "vulkan/nvofapi_vulkan_instance.h"

#include "util/util_env.h"
#include "util/util_log.h"
#include "util/util_string.h"
#include "../version.h"

extern "C" {
    // VK entrypoints
    NV_OF_STATUS NVOFAPI CreateOpticalFlowVk(VkInstance vkInstance, VkPhysicalDevice vkPhysicalDevice, VkDevice vkDevice, NvOFHandle* hOFInstance) {
        dxvk::log::info("CreateOpticalFlowVk called");
        nvofapi::NvOFInstanceVk* nvOF = new nvofapi::NvOFInstanceVk(vkInstance, vkPhysicalDevice, vkDevice);

        if (!nvOF) {
            dxvk::log::info(dxvk::str::format("CreateOpticalFlowVK failed allocation"));
            return NV_OF_ERR_GENERIC;
        }

        if (!nvOF->Initialize()) {
            dxvk::log::info(dxvk::str::format("CreateOpticalFlowVK failed creation"));
            delete nvOF;
            return NV_OF_ERR_GENERIC;
        }

        *hOFInstance = reinterpret_cast<NvOFHandle>(nvOF);
        return NV_OF_SUCCESS;
    }

    NV_OF_STATUS NVOFAPI GetSurfaceFormatCountVk(NvOFHandle hOf, const NV_OF_BUFFER_USAGE bufferUsage, const NV_OF_MODE ofMode, uint32_t* const pCount) {
        dxvk::log::info("GetSurfaceFormatCountVk called");
        dxvk::log::info(dxvk::str::format("GetSurfaceFormatCountVk failed!"));
        return NV_OF_ERR_GENERIC;
    }

    NV_OF_STATUS NVOFAPI GetSurfaceFormatVk(NvOFHandle hOf, const NV_OF_BUFFER_USAGE bufferUsage, const NV_OF_MODE ofMode, VkFormat* const pFormat) {
        dxvk::log::info("GetSurfaceFormatVk called");
        dxvk::log::info(dxvk::str::format("GetSurfaceFormatVk failed!"));
        return NV_OF_ERR_GENERIC;
    }

    NV_OF_STATUS NVOFAPI RegisterResourceVk(NvOFHandle hOf, NV_OF_REGISTER_RESOURCE_PARAMS_VK* registerParams) {
        dxvk::log::info("RegisterResourceVk called");
        nvofapi::NvOFInstanceVk* nvOF = reinterpret_cast<nvofapi::NvOFInstanceVk*>(hOf);
        return nvOF->RegisterBuffer(registerParams);
    }

    NV_OF_STATUS NVOFAPI UnregisterResourceVk(NV_OF_UNREGISTER_RESOURCE_PARAMS_VK* registerParams) {
        dxvk::log::info("UnregisterResourceVk called");
        nvofapi::NvOFImage* nvRes = reinterpret_cast<nvofapi::NvOFImage*>(registerParams->hOFGpuBuffer);
        delete nvRes;
        return NV_OF_SUCCESS;
    }

    NV_OF_STATUS NVOFAPI ExecuteVk(NvOFHandle hOf, const NV_OF_EXECUTE_INPUT_PARAMS_VK* executeInParams, NV_OF_EXECUTE_OUTPUT_PARAMS_VK* executeOutParams) {
        nvofapi::NvOFInstanceVk* nvOF = reinterpret_cast<nvofapi::NvOFInstanceVk*>(hOf);

        return nvOF->Execute(executeInParams, executeOutParams);
    }

    NV_OF_STATUS __cdecl NvOFAPICreateInstanceVk(uint32_t apiVer, NV_OF_VK_API_FUNCTION_LIST* functionList) {
        uint32_t apiVerMajor = (apiVer & 0xfffffff0) >> 4;
        uint32_t apiVerMinor = (apiVer & 0xf);
        constexpr auto n = __func__;

        dxvk::log::info(dxvk::str::format("DXVK-NVOFAPI ", DXVK_NVAPI_VERSION, " (", dxvk::env::getExecutableName(), ") VK"));
        dxvk::log::info(dxvk::str::format("OFAPI Client Version: ", apiVerMajor, ".", apiVerMinor));

        if (apiVerMajor != 5)
            return NV_OF_ERR_INVALID_VERSION;

        functionList->nvCreateOpticalFlowVk = CreateOpticalFlowVk;
        functionList->nvOFInit = OFSessionInit;
        functionList->nvOFGetSurfaceFormatCountVk = GetSurfaceFormatCountVk;
        functionList->nvOFGetSurfaceFormatVk = GetSurfaceFormatVk;
        functionList->nvOFRegisterResourceVk = RegisterResourceVk;
        functionList->nvOFUnregisterResourceVk = UnregisterResourceVk;
        functionList->nvOFExecuteVk = ExecuteVk;
        functionList->nvOFDestroy = OFSessionDestroy;
        functionList->nvOFGetLastError = OFSessionGetLastError;
        functionList->nvOFGetCaps = OFSessionGetCaps;

        return NV_OF_SUCCESS;
    }
}
