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

#pragma once

#include "vkd3d-proton/vkd3d-proton_interfaces.h"
#include "../inc/nvofapi/nvOpticalFlowD3D12.h"
#include "../inc/nvofapi/nvOpticalFlowVulkan.h"

namespace dxvk {
    constexpr uint32_t CMDS_IN_FLIGHT = 8;

    class NvOFInstance {
      public:
        NvOFInstance(VkInstance vkInstance,
            VkPhysicalDevice vkPhysicalDevice,
            VkDevice vkDevice) : m_vkInstance(vkInstance),
                                 m_vkPhysicalDevice(vkPhysicalDevice), m_vkDevice(vkDevice) {
        }

        NvOFInstance() {};

        virtual ~NvOFInstance() {
            m_vkDestroyOpticalFlowSessionNV(m_vkDevice, m_vkOfaSession, nullptr);
            FreeLibrary(m_library);
        }

        VkDevice GetVkDevice() { return m_vkDevice; }
        VkOpticalFlowSessionNV GetOfaSession() { return m_vkOfaSession; }

        NV_OF_STATUS getCaps(NV_OF_CAPS param, uint32_t* capsVal, uint32_t* size);

        NV_OF_STATUS InitSession(const NV_OF_INIT_PARAMS* initParams);

        void RegisterBuffer(const NV_OF_REGISTER_RESOURCE_PARAMS_VK* registerParams);

        NV_OF_STATUS BindImageToSession(NvOFGPUBufferHandle hBuffer, VkOpticalFlowSessionBindingPointNV bindingPoint);

        void RecordCmdBuf(const NV_OF_EXECUTE_INPUT_PARAMS_VK* inParams, NV_OF_EXECUTE_OUTPUT_PARAMS_VK* outParams, VkCommandBuffer cmdBuf);

      protected:
        VkInstance m_vkInstance;
        VkPhysicalDevice m_vkPhysicalDevice;
        VkDevice m_vkDevice;
        VkOpticalFlowSessionNV m_vkOfaSession{};
        PFN_vkGetInstanceProcAddr m_vkGetInstanceProcAddr{};
        PFN_vkGetDeviceProcAddr m_vkGetDeviceProcAddr{};
        PFN_vkCreateOpticalFlowSessionNV m_vkCreateOpticalFlowSessionNV{};
        PFN_vkDestroyOpticalFlowSessionNV m_vkDestroyOpticalFlowSessionNV{};
        PFN_vkCreateImageView m_vkCreateImageView{};
        PFN_vkDestroyImageView m_vkDestroyImageView{};
        PFN_vkBindOpticalFlowSessionImageNV m_vkBindOpticalFlowSessionImageNV{};
        PFN_vkCmdOpticalFlowExecuteNV m_vkCmdOpticalFlowExecuteNV{};

        PFN_vkGetPhysicalDeviceQueueFamilyProperties m_vkGetPhysicalDeviceQueueFamilyProperties;
        HMODULE m_library;

        uint32_t GetVkOFAQueue();
    };
}
