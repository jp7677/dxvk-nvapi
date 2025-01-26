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

#include "../nvofapi_private.h"
#include "../shared/resource_factory.h"
#include "nvofapi_instance.h"

namespace dxvk {
    class NvOFInstanceVk final : public NvOFInstance {

      public:
        NvOFInstanceVk(ResourceFactory& resourceFactory, VkInstance vkInstance, VkPhysicalDevice vkPhysicalDevice, VkDevice vkDevice)
            : NvOFInstance(resourceFactory, vkInstance, vkPhysicalDevice, vkDevice) {}

        ~NvOFInstanceVk() override {
            // free cmdbuffers
            if (m_vkFreeCommandBuffers)
                m_vkFreeCommandBuffers(m_vkDevice, m_commandPool, CMDS_IN_FLIGHT, m_commandBuffers.data());
            if (m_vkDestroyCommandPool)
                m_vkDestroyCommandPool(m_vkDevice, m_commandPool, nullptr);
        }

        bool Initialize();

        bool Execute(const NV_OF_EXECUTE_INPUT_PARAMS_VK* inParams, NV_OF_EXECUTE_OUTPUT_PARAMS_VK* outParams);

      private:
        VkQueue m_queue{};
        VkCommandPool m_commandPool{};
        PFN_vkCreateCommandPool m_vkCreateCommandPool{};
        PFN_vkDestroyCommandPool m_vkDestroyCommandPool{};
        std::array<VkCommandBuffer, CMDS_IN_FLIGHT> m_commandBuffers{};

        PFN_vkAllocateCommandBuffers m_vkAllocateCommandBuffers{};
        PFN_vkFreeCommandBuffers m_vkFreeCommandBuffers{};

        uint32_t m_cmdBufIndex{0};
        PFN_vkResetCommandBuffer m_vkResetCommandBuffer{};
        PFN_vkBeginCommandBuffer m_vkBeginCommandBuffer{};
        PFN_vkEndCommandBuffer m_vkEndCommandBuffer{};

        PFN_vkGetDeviceQueue m_vkGetDeviceQueue{};
        PFN_vkQueueSubmit2 m_vkQueueSubmit2{};
    };
}
