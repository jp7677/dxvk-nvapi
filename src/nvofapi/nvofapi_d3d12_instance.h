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
#include "../util/com_pointer.h"

namespace dxvk {
    class NvOFInstanceD3D12 final : public NvOFInstance {

      public:
        NvOFInstanceD3D12(ResourceFactory& resourceFactory, ID3D12Device* pD3D12Device);
        ~NvOFInstanceD3D12() override = default;

        bool Initialize();
        void Execute(const NV_OF_EXECUTE_INPUT_PARAMS_D3D12* inParams, NV_OF_EXECUTE_OUTPUT_PARAMS_D3D12* outParams);
        void RegisterBuffer(const NV_OF_REGISTER_RESOURCE_PARAMS_D3D12* registerParams);

      private:
        Com<ID3D12DXVKInteropDevice1> m_device{};
        Com<ID3D12Device4> m_d3ddevice{};
        Com<ID3D12DeviceExt> m_deviceExt{};
        Com<ID3D12CommandQueue> m_commandQueue{};
        Com<ID3D12CommandAllocator> m_cmdAllocator{};
        std::array<Com<ID3D12GraphicsCommandList>, CMDS_IN_FLIGHT> m_cmdLists{};
        uint32_t m_cmdListIndex{0};

        uint32_t m_vkQueueFamilyIndex{0};
    };
}
