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

#include "util/util_log.h"
#include "util/util_string.h"

#include "../inc/nvofapi/nvOpticalFlowVulkan.h"

#include "vulkan/nvofapi_vulkan_instance.h"

namespace nvofapi {

    bool NvOFInstanceVk::Initialize() {
        // For VK we cannot load winevulkan directly or we may break handle
        // opacity.
        m_library = LoadLibraryA("vulkan-1.dll");
        if (!m_library) {
            return false;
        }

        m_vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr) reinterpret_cast<void*>(GetProcAddress(m_library, "vkGetInstanceProcAddr"));
        m_vkGetDeviceProcAddr = (PFN_vkGetDeviceProcAddr)m_vkGetInstanceProcAddr(m_vkInstance, "vkGetDeviceProcAddr");
        m_vkGetPhysicalDeviceQueueFamilyProperties = (PFN_vkGetPhysicalDeviceQueueFamilyProperties)m_vkGetInstanceProcAddr(m_vkInstance, "vkGetPhysicalDeviceQueueFamilyProperties");

        m_vkCreateImageView = (PFN_vkCreateImageView)m_vkGetDeviceProcAddr(m_vkDevice, "vkCreateImageView");
        m_vkDestroyImageView = (PFN_vkDestroyImageView)m_vkGetDeviceProcAddr(m_vkDevice, "vkDestroyImageView");

        m_vkGetDeviceQueue = (PFN_vkGetDeviceQueue)m_vkGetDeviceProcAddr(m_vkDevice, "vkGetDeviceQueue");
        m_vkCreateCommandPool = (PFN_vkCreateCommandPool)m_vkGetDeviceProcAddr(m_vkDevice, "vkCreateCommandPool");
        m_vkDestroyCommandPool = (PFN_vkDestroyCommandPool)m_vkGetDeviceProcAddr(m_vkDevice, "vkDestroyCommandPool");
        m_vkAllocateCommandBuffers = (PFN_vkAllocateCommandBuffers)m_vkGetDeviceProcAddr(m_vkDevice, "vkAllocateCommandBuffers");
        m_vkQueueSubmit2 = (PFN_vkQueueSubmit2)m_vkGetDeviceProcAddr(m_vkDevice, "vkQueueSubmit2");

        m_vkResetCommandBuffer = (PFN_vkResetCommandBuffer)m_vkGetDeviceProcAddr(m_vkDevice, "vkResetCommandBuffer");
        m_vkBeginCommandBuffer = (PFN_vkBeginCommandBuffer)m_vkGetDeviceProcAddr(m_vkDevice, "vkBeginCommandBuffer");
        m_vkEndCommandBuffer = (PFN_vkEndCommandBuffer)m_vkGetDeviceProcAddr(m_vkDevice, "vkEndCommandBuffer");

        // Get NvapiAdapter from the vkPhysicalDevice
        // Populate the optical flow related info here
        // fail to create if optical flow extension is unsupported
        m_vkCreateOpticalFlowSessionNV = (PFN_vkCreateOpticalFlowSessionNV)m_vkGetDeviceProcAddr(m_vkDevice, "vkCreateOpticalFlowSessionNV");
        m_vkDestroyOpticalFlowSessionNV = (PFN_vkDestroyOpticalFlowSessionNV)m_vkGetDeviceProcAddr(m_vkDevice, "vkDestroyOpticalFlowSessionNV");
        m_vkBindOpticalFlowSessionImageNV = (PFN_vkBindOpticalFlowSessionImageNV)m_vkGetDeviceProcAddr(m_vkDevice, "vkBindOpticalFlowSessionImageNV");
        m_vkCmdOpticalFlowExecuteNV = (PFN_vkCmdOpticalFlowExecuteNV)m_vkGetDeviceProcAddr(m_vkDevice, "vkCmdOpticalFlowExecuteNV");

        // Get the OFA queue
        VkCommandPoolCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        createInfo.queueFamilyIndex = GetVkOFAQueue();
        createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        m_vkGetDeviceQueue(m_vkDevice, createInfo.queueFamilyIndex, 0, &m_queue);

        if (m_vkCreateCommandPool(m_vkDevice, &createInfo, nullptr, &m_commandPool)
            != VK_SUCCESS) {
            return false;
        }

        // ALlocate command buffers
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = CMDS_IN_FLIGHT; // more than enough for anybody ;)
        if (m_vkAllocateCommandBuffers(m_vkDevice, &allocInfo, m_commandBuffers)
            != VK_SUCCESS) {
            return false;
        }

        return true;
    }

    void NvOFInstanceVk::Execute(const NV_OF_EXECUTE_INPUT_PARAMS_VK* inParams, NV_OF_EXECUTE_OUTPUT_PARAMS_VK* outParams) {
        dxvk::log::info(
            dxvk::str::format("OFExecuteVK params:",
                " inputFrame: ", inParams->inputFrame,
                " referenceFrame: ", inParams->referenceFrame,
                " externalHints: ", inParams->externalHints,
                " disableTemporalHints: ", inParams->disableTemporalHints,
                " hPrivData: ", inParams->hPrivData,
                " numRois: ", inParams->numRois,
                " roiData: ", inParams->roiData,
                " numWaitSync: ", inParams->numWaitSyncs,
                " pWaitSyncs: ", inParams->pWaitSyncs));

        auto waitSyncs = std::vector<VkSemaphoreSubmitInfo>(inParams->numWaitSyncs);
        for (uint32_t i = 0; i < inParams->numWaitSyncs; i++) {
            waitSyncs[i].sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
            waitSyncs[i].semaphore = inParams->pWaitSyncs[i].semaphore;
            waitSyncs[i].value = inParams->pWaitSyncs[i].value;
            waitSyncs[i].stageMask = VK_PIPELINE_STAGE_2_OPTICAL_FLOW_BIT_NV;
        }

        VkSemaphoreSubmitInfo signalSync{};
        signalSync.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        if (outParams->pSignalSync) {
            signalSync.semaphore = outParams->pSignalSync->semaphore;
            signalSync.value = outParams->pSignalSync->value;
            signalSync.stageMask = VK_PIPELINE_STAGE_2_OPTICAL_FLOW_BIT_NV;
        }

        VkCommandBufferSubmitInfo cmdbufInfo{};
        cmdbufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        cmdbufInfo.commandBuffer = m_commandBuffers[m_cmdBufIndex];
        cmdbufInfo.deviceMask = 1;

        VkCommandBufferBeginInfo begInfo{};
        begInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        m_vkResetCommandBuffer(m_commandBuffers[m_cmdBufIndex], 0);
        m_vkBeginCommandBuffer(m_commandBuffers[m_cmdBufIndex], &begInfo);

        RecordCmdBuf(inParams, outParams, m_commandBuffers[m_cmdBufIndex]);

        m_vkEndCommandBuffer(m_commandBuffers[m_cmdBufIndex]);

        VkSubmitInfo2 submit{};
        submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        submit.waitSemaphoreInfoCount = waitSyncs.size();
        submit.pWaitSemaphoreInfos = waitSyncs.data();
        submit.commandBufferInfoCount = 1;
        submit.pCommandBufferInfos = &cmdbufInfo;
        submit.signalSemaphoreInfoCount = (outParams->pSignalSync) ? 1 : 0;
        submit.pSignalSemaphoreInfos = &signalSync;

        m_vkQueueSubmit2(m_queue, 1, &submit, VK_NULL_HANDLE);

        m_cmdBufIndex++;
        if (m_cmdBufIndex >= CMDS_IN_FLIGHT)
            m_cmdBufIndex = 0;
    }
}
