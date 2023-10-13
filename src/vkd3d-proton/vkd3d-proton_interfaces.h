/*
 * Copyright (c) 2021 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License, version 2.1, as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "../nvapi_private.h"

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#endif // defined(__GNUC__) || defined(__clang__)

enum D3D12_VK_EXTENSION : uint32_t {
    D3D12_VK_NVX_BINARY_IMPORT = 0x1,
    D3D12_VK_NVX_IMAGE_VIEW_HANDLE = 0x2,
    D3D12_VK_NV_LOW_LATENCY_2 = 0x3
};

enum D3D12_OUT_OF_BAND_CQ_TYPE : uint32_t {
    D3D_OUT_OF_BAND_RENDER = 0x0,
    D3D_OUT_OF_BAND_PRESENT = 0x1
};

struct D3D12_CUBIN_DATA_HANDLE {
    VkCuFunctionNVX vkCuFunction;
    VkCuModuleNVX vkCuModule;
    UINT32 blockX;
    UINT32 blockY;
    UINT32 blockZ;
};

typedef struct D3D12_UAV_INFO {
    UINT32 version;
    UINT32 surfaceHandle;
    UINT64 gpuVAStart;
    UINT64 gpuVASize;
} D3D12_UAV_INFO;

MIDL_INTERFACE("11ea7a1a-0f6a-49bf-b612-3e30f8e201dd")
ID3D12DeviceExt : public IUnknown {
    virtual HRESULT STDMETHODCALLTYPE GetVulkanHandles(
        VkInstance * vk_instance,
        VkPhysicalDevice * vk_physical_device,
        VkDevice * vk_device) = 0;

    virtual BOOL STDMETHODCALLTYPE GetExtensionSupport(
        D3D12_VK_EXTENSION extension) = 0;

    virtual HRESULT STDMETHODCALLTYPE CreateCubinComputeShaderWithName(
        const void* cubin_data,
        UINT32 cubin_size,
        UINT32 block_x,
        UINT32 block_y,
        UINT32 block_z,
        const char* shader_name,
        D3D12_CUBIN_DATA_HANDLE** handle) = 0;

    virtual HRESULT STDMETHODCALLTYPE DestroyCubinComputeShader(
        D3D12_CUBIN_DATA_HANDLE * handle) = 0;

    virtual HRESULT STDMETHODCALLTYPE GetCudaTextureObject(
        D3D12_CPU_DESCRIPTOR_HANDLE srv_handle,
        D3D12_CPU_DESCRIPTOR_HANDLE sampler_handle,
        UINT32 * cuda_texture_handle) = 0;

    virtual HRESULT STDMETHODCALLTYPE GetCudaSurfaceObject(
        D3D12_CPU_DESCRIPTOR_HANDLE uav_handle,
        UINT32 * cuda_surface_handle) = 0;

    virtual HRESULT STDMETHODCALLTYPE CaptureUAVInfo(
        D3D12_UAV_INFO * uav_info) = 0;
};

MIDL_INTERFACE("77a86b09-2bea-4801-b89a-37648e104af1")
ID3D12GraphicsCommandListExt : public IUnknown {
    virtual HRESULT STDMETHODCALLTYPE GetVulkanHandle(
        VkCommandBuffer * pVkCommandBuffer) = 0;

    virtual HRESULT STDMETHODCALLTYPE LaunchCubinShader(
        D3D12_CUBIN_DATA_HANDLE * handle,
        UINT32 block_x,
        UINT32 block_y,
        UINT32 block_z,
        const void* params,
        UINT32 param_size) = 0;
};

MIDL_INTERFACE("d53b0028-afb4-4b65-a4f1-7b0daaa65b4f")
ID3D12GraphicsCommandListExt1 : public ID3D12GraphicsCommandListExt {
    virtual HRESULT STDMETHODCALLTYPE LaunchCubinShaderEx(
        D3D12_CUBIN_DATA_HANDLE * handle,
        UINT32 block_x,
        UINT32 block_y,
        UINT32 block_z,
        UINT32 smem_size,
        const void* params,
        UINT32 param_size,
        const void* raw_params,
        UINT32 raw_params_count) = 0;
};

MIDL_INTERFACE("40ed3f96-e773-e9bc-fc0c-e95560c99ad6")
ID3D12CommandQueueExt : public IUnknown {
    virtual HRESULT STDMETHODCALLTYPE NotifyOutOfBandCommandQueue(
        D3D12_OUT_OF_BAND_CQ_TYPE type) = 0;
};

#ifndef _MSC_VER
__CRT_UUID_DECL(ID3D12DeviceExt, 0x11ea7a1a, 0x0f6a, 0x49bf, 0xb6, 0x12, 0x3e, 0x30, 0xf8, 0xe2, 0x01, 0xdd);
__CRT_UUID_DECL(ID3D12GraphicsCommandListExt, 0x77a86b09, 0x2bea, 0x4801, 0xb8, 0x9a, 0x37, 0x64, 0x8e, 0x10, 0x4a, 0xf1);
__CRT_UUID_DECL(ID3D12GraphicsCommandListExt1, 0xd53b0028, 0xafb4, 0x4b65, 0xa4, 0xf1, 0x7b, 0x0d, 0xaa, 0xa6, 0x5b, 0x4f);
__CRT_UUID_DECL(ID3D12CommandQueueExt, 0x40ed3f96, 0xe773, 0xe9bc, 0xfc, 0x0c, 0xe9, 0x55, 0x60, 0xc9, 0x9a, 0xd6);
#endif
