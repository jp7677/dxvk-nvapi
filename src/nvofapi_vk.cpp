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
#include "nvofapi/nvofapi_vk_instance.h"
#include "util/util_env.h"
#include "util/util_log.h"
#include "util/util_string.h"
#include "util/util_statuscode.h"
#include "../version.h"
#include "../config.h"

static auto initializationMutex = std::mutex{};

extern "C" {

    using namespace dxvk;

    // VK entrypoints
    NV_OF_STATUS NVOFAPI CreateOpticalFlowVk(VkInstance vkInstance, VkPhysicalDevice vkPhysicalDevice, VkDevice vkDevice, NvOFHandle* hOFInstance) {
        constexpr auto n = __func__;

        if (log::tracing())
            log::trace(n, log::fmt::hnd(vkInstance), log::fmt::hnd(vkPhysicalDevice), log::fmt::hnd(vkDevice), log::fmt::ptr(hOFInstance));

        std::scoped_lock lock(initializationMutex);
        if (resourceFactory == nullptr)
            resourceFactory = std::make_unique<ResourceFactory>();

        auto nvOF = new NvOFInstanceVk(*resourceFactory, vkInstance, vkPhysicalDevice, vkDevice);

        if (!nvOF->Initialize()) {
            delete nvOF;
            return ErrorGeneric(n);
        }

        *hOFInstance = reinterpret_cast<NvOFHandle>(nvOF);
        return Success(n);
    }

    NV_OF_STATUS NVOFAPI GetSurfaceFormatCountVk(NvOFHandle hOf, const NV_OF_BUFFER_USAGE bufferUsage, const NV_OF_MODE ofMode, uint32_t* const pCount) {
        constexpr auto n = __func__;

        if (log::tracing())
            log::trace(n, log::fmt::hnd(hOf), bufferUsage, ofMode, log::fmt::ptr(pCount));

        return ErrorGeneric(n);
    }

    NV_OF_STATUS NVOFAPI GetSurfaceFormatVk(NvOFHandle hOf, const NV_OF_BUFFER_USAGE bufferUsage, const NV_OF_MODE ofMode, VkFormat* const pFormat) {
        constexpr auto n = __func__;

        if (log::tracing())
            log::trace(n, log::fmt::hnd(hOf), bufferUsage, ofMode, log::fmt::ptr(pFormat));

        return ErrorGeneric(n);
    }

    NV_OF_STATUS NVOFAPI RegisterResourceVk(NvOFHandle hOf, NV_OF_REGISTER_RESOURCE_PARAMS_VK* registerParams) {
        constexpr auto n = __func__;

        if (log::tracing())
            log::trace(n, log::fmt::hnd(hOf), log::fmt::ptr(registerParams));

        auto nvOF = reinterpret_cast<NvOFInstanceVk*>(hOf);
        nvOF->RegisterBuffer(registerParams);
        return Success(n);
    }

    NV_OF_STATUS NVOFAPI UnregisterResourceVk(NV_OF_UNREGISTER_RESOURCE_PARAMS_VK* registerParams) {
        constexpr auto n = __func__;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(registerParams));

        auto nvRes = reinterpret_cast<NvOFImage*>(registerParams->hOFGpuBuffer);
        delete nvRes;
        return ErrorGeneric(n);
    }

    NV_OF_STATUS NVOFAPI ExecuteVk(NvOFHandle hOf, const NV_OF_EXECUTE_INPUT_PARAMS_VK* executeInParams, NV_OF_EXECUTE_OUTPUT_PARAMS_VK* executeOutParams) {
        constexpr auto n = __func__;
        thread_local bool alreadyLoggedOk = false;

        if (log::tracing())
            log::trace(n, log::fmt::hnd(hOf), log::fmt::ptr(executeInParams), log::fmt::ptr(executeOutParams));

        auto nvOF = reinterpret_cast<NvOFInstanceVk*>(hOf);

        if (nvOF->Execute(executeInParams, executeOutParams))
            return Success(n, alreadyLoggedOk);

        return ErrorGeneric(n);
    }

    NV_OF_STATUS __cdecl NvOFAPICreateInstanceVk(uint32_t apiVer, NV_OF_VK_API_FUNCTION_LIST* functionList) {
        uint32_t apiVerMajor = (apiVer & 0xfffffff0) >> 4;
        uint32_t apiVerMinor = (apiVer & 0xf);
        constexpr auto n = __func__;

        if (log::tracing())
            log::trace(n, apiVer, log::fmt::ptr(functionList));

        log::info(str::format(
            "DXVK-NVAPI ", DXVK_NVAPI_VERSION,
            " NVOFAPI/VK",
            " ", DXVK_NVAPI_BUILD_COMPILER,
            " ", DXVK_NVAPI_BUILD_COMPILER_VERSION,
            " ", DXVK_NVAPI_BUILD_TARGET,
            " ", DXVK_NVAPI_BUILD_TYPE,
            " (", env::getExecutableName(), ")"));

        log::info(str::format("OFAPI Client Version: ", apiVerMajor, ".", apiVerMinor));

        if (apiVerMajor != 5)
            return InvalidVersion(n);

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

        return Success(n);
    }
}
