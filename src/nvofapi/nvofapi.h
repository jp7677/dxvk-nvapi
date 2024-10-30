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

namespace nvofapi {
    constexpr uint32_t CMDS_IN_FLIGHT = 8;
    class NvOFInstance;
    class NvOFImageVk;

    class NvOFInstance {

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

      public:
        VkDevice GetVkDevice() { return m_vkDevice; }
        VkOpticalFlowSessionNV GetOfaSession() { return m_vkOfaSession; }

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

        NV_OF_STATUS getCaps(NV_OF_CAPS param, uint32_t* capsVal, uint32_t* size);

        NV_OF_STATUS InitSession(const NV_OF_INIT_PARAMS* initParams);

        NV_OF_STATUS RegisterBuffer(const NV_OF_REGISTER_RESOURCE_PARAMS_VK* registerParams);

        NV_OF_STATUS BindImageToSession(NvOFImageVk* image, VkOpticalFlowSessionBindingPointNV bindingPoint);

        NV_OF_STATUS RecordCmdBuf(const NV_OF_EXECUTE_INPUT_PARAMS_VK* inParams, NV_OF_EXECUTE_OUTPUT_PARAMS_VK* outParams, VkCommandBuffer cmdBuf);
    };

    class NvOFImageVk {
      private:
        NvOFInstance* m_ofInstance;
        VkImage m_image;
        VkImageView m_imageView;
        VkFormat m_format;
        PFN_vkDestroyImageView m_vkDestroyImageView{};

      public:
        VkImageView ImageView() { return m_imageView; }
        NvOFImageVk(NvOFInstance* ofInstance, VkImage image, VkFormat format) : m_ofInstance(ofInstance), m_image(image), m_format(format) {
        }
        ~NvOFImageVk() {
            m_vkDestroyImageView(m_ofInstance->GetVkDevice(), m_imageView, nullptr);
        }

        bool Initialize(PFN_vkCreateImageView CreateImageView,
            PFN_vkDestroyImageView DestroyImageView);
    };

    class NvOFInstanceVk : public NvOFInstance {
      public:
        NvOFInstanceVk(VkInstance vkInstance, VkPhysicalDevice vkPhysicalDevice, VkDevice vkDevice) : NvOFInstance(vkInstance, vkPhysicalDevice, vkDevice) {
        }
        virtual ~NvOFInstanceVk() {
            // free cmdbuffers
            m_vkFreeCommandBuffers(m_vkDevice, m_commandPool, CMDS_IN_FLIGHT, m_commandBuffers);
            m_vkDestroyCommandPool(m_vkDevice, m_commandPool, nullptr);
        }
        bool Initialize();

        NV_OF_STATUS Execute(const NV_OF_EXECUTE_INPUT_PARAMS_VK* inParams, NV_OF_EXECUTE_OUTPUT_PARAMS_VK* outParams);

      private:
        VkQueue m_queue;
        VkCommandPool m_commandPool;
        PFN_vkCreateCommandPool m_vkCreateCommandPool;
        PFN_vkDestroyCommandPool m_vkDestroyCommandPool;
        VkCommandBuffer m_commandBuffers[CMDS_IN_FLIGHT];

        PFN_vkAllocateCommandBuffers m_vkAllocateCommandBuffers;
        PFN_vkFreeCommandBuffers m_vkFreeCommandBuffers;

        uint32_t m_cmdBufIndex = 0;
        PFN_vkResetCommandBuffer m_vkResetCommandBuffer;
        PFN_vkBeginCommandBuffer m_vkBeginCommandBuffer;
        PFN_vkEndCommandBuffer m_vkEndCommandBuffer;

        PFN_vkGetDeviceQueue m_vkGetDeviceQueue;
        PFN_vkQueueSubmit2 m_vkQueueSubmit2{};
    };

    class NvOFInstanceD3D12 : public NvOFInstance {
      private:
        ID3D12DXVKInteropDevice1* m_device{};
        ID3D12DeviceExt* m_deviceExt{};
        ID3D12Device4* m_d3ddevice{};
        ID3D12CommandQueue* m_commandQueue{};
        ID3D12GraphicsCommandList* m_cmdList[CMDS_IN_FLIGHT]{};
        uint32_t m_cmdListIndex = 0;
        ID3D12CommandAllocator* m_cmdAllocator{};

        uint32_t m_vkQueueFamilyIndex = 0;

      public:
        bool Initialize();
        NvOFInstanceD3D12(ID3D12Device* pD3D12Device);
        virtual ~NvOFInstanceD3D12() {
            for (int i = 0; i < CMDS_IN_FLIGHT; i++) {
                if (m_cmdList[i])
                    m_cmdList[i]->Release();
            }
            if (m_cmdAllocator)
                m_cmdAllocator->Release();

            if (m_commandQueue)
                m_commandQueue->Release();

            if (m_deviceExt)
                m_deviceExt->Release();

            if (m_d3ddevice)
                m_d3ddevice->Release();

            if (m_device)
                m_device->Release();
        }

        NV_OF_STATUS RegisterBuffer(const NV_OF_REGISTER_RESOURCE_PARAMS_D3D12* registerParams);

        NV_OF_STATUS Execute(const NV_OF_EXECUTE_INPUT_PARAMS_D3D12* inParams, NV_OF_EXECUTE_OUTPUT_PARAMS_D3D12* outParams);
    };
}
