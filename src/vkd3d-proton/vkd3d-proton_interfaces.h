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
    D3D12_VK_NV_LOW_LATENCY_2 = 0x3,
    D3D12_VK_NV_OPTICAL_FLOW = 0x4
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

MIDL_INTERFACE("39da4e09-bd1c-4198-9fae-86bbe3be41fd")
ID3D12DXVKInteropDevice : public IUnknown {
    virtual HRESULT STDMETHODCALLTYPE GetDXGIAdapter(
        REFIID iid,
        void** object) = 0;

    virtual HRESULT STDMETHODCALLTYPE GetInstanceExtensions(
        UINT * extensionCount,
        const char** extensions) = 0;

    virtual HRESULT STDMETHODCALLTYPE GetDeviceExtensions(
        UINT * extensionCount,
        const char** extensions) = 0;

    virtual HRESULT STDMETHODCALLTYPE GetDeviceFeatures(
        const VkPhysicalDeviceFeatures2** features) = 0;

    virtual HRESULT STDMETHODCALLTYPE GetVulkanHandles(
        VkInstance * vk_instance,
        VkPhysicalDevice * vk_physical_device,
        VkDevice * vk_device) = 0;

    virtual HRESULT STDMETHODCALLTYPE GetVulkanQueueInfo(
        ID3D12CommandQueue * queue,
        VkQueue * vkQueue,
        UINT32 * vkFamily) = 0;

    virtual void STDMETHODCALLTYPE GetVulkanImageLayout(
        ID3D12Resource * resource,
        D3D12_RESOURCE_STATES state,
        VkImageLayout * layout) = 0;

    virtual HRESULT STDMETHODCALLTYPE GetVulkanResourceInfo(
        ID3D12Resource * resource,
        UINT64 * handle,
        UINT64 * offset) = 0;

    virtual HRESULT STDMETHODCALLTYPE LockCommandQueue(
        ID3D12CommandQueue * queue) = 0;

    virtual HRESULT STDMETHODCALLTYPE UnlockCommandQueue(
        ID3D12CommandQueue * queue) = 0;
};
MIDL_INTERFACE("902d8115-59eb-4406-9518-fe00f991ee65")
ID3D12DXVKInteropDevice1 : public ID3D12DXVKInteropDevice {

    virtual HRESULT STDMETHODCALLTYPE GetVulkanResourceInfo1(ID3D12Resource * resource, UINT64 * vk_handle, UINT64 * buffer_offset, VkFormat * format);
    virtual HRESULT STDMETHODCALLTYPE CreateInteropCommandQueue(const D3D12_COMMAND_QUEUE_DESC* pDesc, UINT32 vk_queue_family_index, ID3D12CommandQueue** ppQueue);
    virtual HRESULT STDMETHODCALLTYPE CreateInteropCommandAllocator(D3D12_COMMAND_LIST_TYPE type, UINT32 vk_queue_family_index, ID3D12CommandAllocator * *ppAllocator);
    virtual HRESULT STDMETHODCALLTYPE BeginVkCommandBufferInterop(ID3D12CommandList * pCmdList, VkCommandBuffer * pCommandBuffer);
    virtual HRESULT STDMETHODCALLTYPE EndVkCommandBufferInterop(ID3D12CommandList * pCmdList);
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
__CRT_UUID_DECL(ID3D12DXVKInteropDevice, 0x39da4e09, 0xbd1c, 0x4198, 0x9f, 0xae, 0x86, 0xbb, 0xe3, 0xbe, 0x41, 0xfd);
__CRT_UUID_DECL(ID3D12DXVKInteropDevice1, 0x902d8115, 0x59eb, 0x4406, 0x95, 0x18, 0xfe, 0x00, 0xf9, 0x91, 0xee, 0x65);
__CRT_UUID_DECL(ID3D12GraphicsCommandListExt, 0x77a86b09, 0x2bea, 0x4801, 0xb8, 0x9a, 0x37, 0x64, 0x8e, 0x10, 0x4a, 0xf1);
__CRT_UUID_DECL(ID3D12GraphicsCommandListExt1, 0xd53b0028, 0xafb4, 0x4b65, 0xa4, 0xf1, 0x7b, 0x0d, 0xaa, 0xa6, 0x5b, 0x4f);
__CRT_UUID_DECL(ID3D12CommandQueueExt, 0x40ed3f96, 0xe773, 0xe9bc, 0xfc, 0x0c, 0xe9, 0x55, 0x60, 0xc9, 0x9a, 0xd6);
#endif
