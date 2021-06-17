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

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#endif // __GNUC__

#define VKD3D_PROTON_GUID(iface) \
    template<> inline GUID const& __mingw_uuidof<iface> () { return iface::guid; }

enum D3D12_VK_EXTENSION : uint32_t {
    D3D12_VK_NVX_BINARY_IMPORT      = 0x1,
    D3D12_VK_NVX_IMAGE_VIEW_HANDLE  = 0x2
};

struct D3D12_CUBIN_DATA_HANDLE {
    VkCuFunctionNVX vkCuFunction;
    VkCuModuleNVX vkCuModule;
    UINT32 blockX;
    UINT32 blockY;
    UINT32 blockZ; 
};

typedef struct D3D12_UAV_INFO
{
    UINT32 version;
    UINT32 surfaceHandle;
    UINT64 gpuVAStart;
    UINT64 gpuVASize;  
} D3D12_UAV_INFO;

MIDL_INTERFACE("11ea7a1a-0f6a-49bf-b612-3e30f8e201dd")
ID3D12DeviceExt : public IUnknown
{
    static const GUID guid;
    virtual HRESULT STDMETHODCALLTYPE GetVulkanHandles(
        VkInstance*                   vk_instance,
        VkPhysicalDevice*             vk_physical_device,
        VkDevice*                     vk_device) = 0;

    virtual BOOL STDMETHODCALLTYPE GetExtensionSupport(
        D3D12_VK_EXTENSION            extension) = 0;

    virtual HRESULT STDMETHODCALLTYPE CreateCubinComputeShaderWithName(
        const void*                   cubin_data,
        UINT32                        cubin_size,
        UINT32                        block_x,
        UINT32                        block_y,
        UINT32                        block_z,
        const char*                   shader_name,
        D3D12_CUBIN_DATA_HANDLE**     handle) = 0;

    virtual HRESULT STDMETHODCALLTYPE DestroyCubinComputeShader(
        D3D12_CUBIN_DATA_HANDLE*      handle) = 0;

    virtual HRESULT STDMETHODCALLTYPE GetCudaTextureObject(
        D3D12_CPU_DESCRIPTOR_HANDLE   srv_handle,
        D3D12_CPU_DESCRIPTOR_HANDLE   sampler_handle,
        UINT32*                       cuda_texture_handle) = 0;

    virtual HRESULT STDMETHODCALLTYPE GetCudaSurfaceObject(
        D3D12_CPU_DESCRIPTOR_HANDLE   uav_handle,
        UINT32*                       cuda_surface_handle) = 0;

    virtual HRESULT STDMETHODCALLTYPE CaptureUAVInfo(
        D3D12_UAV_INFO*               uav_info) = 0;
};

MIDL_INTERFACE("77a86b09-2bea-4801-b89a-37648e104af1")
ID3D12GraphicsCommandListExt : public IUnknown
{
    static const GUID guid;
    virtual HRESULT STDMETHODCALLTYPE GetVulkanHandle(
        VkCommandBuffer*               pVkCommandBuffer) = 0;

    virtual HRESULT STDMETHODCALLTYPE LaunchCubinShader(
        D3D12_CUBIN_DATA_HANDLE*       handle,
        UINT32                         block_x,
        UINT32                         block_y,
        UINT32                         block_z,
        const void*                    params,
        UINT32                         param_size) = 0;
};

VKD3D_PROTON_GUID(ID3D12DeviceExt)
VKD3D_PROTON_GUID(ID3D12GraphicsCommandListExt)
