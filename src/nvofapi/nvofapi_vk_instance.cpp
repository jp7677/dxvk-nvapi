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

#include "nvofapi_vk_instance.h"
#include "../util/util_log.h"

namespace dxvk {

    bool NvOFInstanceVk::Initialize() {
        // For Vulkan we cannot load `winevulkan.dll` directly, or we may break handle opacity
        m_vk = m_resourceFactory.CreateVulkan("vulkan-1.dll");

        if (!m_vk || !m_vk->IsAvailable())
            m_vk = m_resourceFactory.CreateVulkan("winevulkan.dll");

        if (!m_vk || !m_vk->IsAvailable()) {
            log::info("Initializing NVOFAPI failed: could not find vulkan-1.dll (nor winevulkan.dll) in the current process");
            return false;
        }

#define VK_GET_INSTANCE_PROC_ADDR(proc) m_##proc = reinterpret_cast<PFN_##proc>(m_vk->GetInstanceProcAddr(m_vkInstance, #proc))

        VK_GET_INSTANCE_PROC_ADDR(vkGetPhysicalDeviceQueueFamilyProperties);

#define VK_GET_DEVICE_PROC_ADDR(proc) m_##proc = reinterpret_cast<PFN_##proc>(m_vk->GetDeviceProcAddr(m_vkDevice, #proc))

        VK_GET_DEVICE_PROC_ADDR(vkCreateImageView);
        VK_GET_DEVICE_PROC_ADDR(vkDestroyImageView);

        VK_GET_DEVICE_PROC_ADDR(vkGetDeviceQueue);
        VK_GET_DEVICE_PROC_ADDR(vkCreateCommandPool);
        VK_GET_DEVICE_PROC_ADDR(vkDestroyCommandPool);
        VK_GET_DEVICE_PROC_ADDR(vkAllocateCommandBuffers);
        VK_GET_DEVICE_PROC_ADDR(vkQueueSubmit2);

        VK_GET_DEVICE_PROC_ADDR(vkResetCommandBuffer);
        VK_GET_DEVICE_PROC_ADDR(vkBeginCommandBuffer);
        VK_GET_DEVICE_PROC_ADDR(vkEndCommandBuffer);

        // Get NvapiAdapter from the vkPhysicalDevice
        // Populate the optical flow related info here
        // fail to create if optical flow extension is unsupported
        VK_GET_DEVICE_PROC_ADDR(vkCreateOpticalFlowSessionNV);
        VK_GET_DEVICE_PROC_ADDR(vkDestroyOpticalFlowSessionNV);
        VK_GET_DEVICE_PROC_ADDR(vkBindOpticalFlowSessionImageNV);
        VK_GET_DEVICE_PROC_ADDR(vkCmdOpticalFlowExecuteNV);

        // Confirm that OPTICAL_FLOW extension is available
        if (!m_vkCreateOpticalFlowSessionNV || !m_vkDestroyOpticalFlowSessionNV || !m_vkBindOpticalFlowSessionImageNV || !m_vkCmdOpticalFlowExecuteNV) {
            log::info("Missing VK_NV_optical_flow extension!");
            return false;
        }

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
        if (m_vkAllocateCommandBuffers(m_vkDevice, &allocInfo, m_commandBuffers.data())
            != VK_SUCCESS) {
            return false;
        }

        return true;
    }

    bool NvOFInstanceVk::Execute(const NV_OF_EXECUTE_INPUT_PARAMS_VK* inParams, NV_OF_EXECUTE_OUTPUT_PARAMS_VK* outParams) {
        VkResult status = VK_SUCCESS;

        log::info(
            str::format("OFExecuteVK params:",
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
        submit.signalSemaphoreInfoCount = outParams->pSignalSync ? 1 : 0;
        submit.pSignalSemaphoreInfos = &signalSync;

        status = m_vkQueueSubmit2(m_queue, 1, &submit, VK_NULL_HANDLE);

        m_cmdBufIndex++;
        if (m_cmdBufIndex >= CMDS_IN_FLIGHT)
            m_cmdBufIndex = 0;

        return status == VK_SUCCESS;
    }
}
