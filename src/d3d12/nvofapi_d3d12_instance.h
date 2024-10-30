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

#include "nvofapi/nvofapi_instance.h"

namespace nvofapi {
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
