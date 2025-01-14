#pragma once

#include "../nvapi_private.h"

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#endif // defined(__GNUC__) || defined(__clang__)

enum D3D11_VK_EXTENSION : uint32_t {
    D3D11_VK_EXT_MULTI_DRAW_INDIRECT = 0,
    D3D11_VK_EXT_MULTI_DRAW_INDIRECT_COUNT = 1,
    D3D11_VK_EXT_DEPTH_BOUNDS = 2,
    D3D11_VK_EXT_BARRIER_CONTROL = 3,
    D3D11_VK_NVX_BINARY_IMPORT = 4,
    D3D11_VK_NVX_IMAGE_VIEW_HANDLE = 5,
};

enum D3D11_VK_BARRIER_CONTROL : uint32_t {
    D3D11_VK_BARRIER_CONTROL_IGNORE_WRITE_AFTER_WRITE = 1 << 0,
};

/**
 * \brief DXGI factory interface for Vulkan interop
 */
MIDL_INTERFACE("4c5e1b0d-b0c8-4131-bfd8-9b2476f7f408")
IDXGIVkInteropFactory : public IUnknown {
    /**
     * \brief Queries Vulkan instance used by DXVK
     *
     * \param [out] pInstance The Vulkan instance
     * \param [out] ppfnVkGetInstanceProcAddr Vulkan entry point
     */
    virtual void STDMETHODCALLTYPE GetVulkanInstance(
        VkInstance * pInstance,
        PFN_vkGetInstanceProcAddr * ppfnVkGetInstanceProcAddr) = 0;
};

/**
￼ * \brief DXGI adapter interface for Vulkan interop
￼ *
￼ * Provides access to the physical device and
￼ * instance handles for the given DXGI adapter.
￼ */
MIDL_INTERFACE("3a6d8f2c-b0e8-4ab4-b4dc-4fd24891bfa5")
IDXGIVkInteropAdapter : public IUnknown {
    /**
     * \brief Queries Vulkan handles used by DXVK
     *
     * \param [out] pInstance The Vulkan instance
     * \param [out] pPhysDev The physical device
     */
    virtual void STDMETHODCALLTYPE GetVulkanHandles(
        VkInstance * pInstance,
        VkPhysicalDevice * pPhysDev) = 0;
};

MIDL_INTERFACE("8a6e3c42-f74c-45b7-8265-a231b677ca17")
ID3D11VkExtDevice : public IUnknown {
    /**
     * \brief Checks whether an extension is supported
     *
     * \param [in] Extension The extension to check
     * \returns \c TRUE if the extension is supported
     */
    virtual BOOL STDMETHODCALLTYPE GetExtensionSupport(
        D3D11_VK_EXTENSION Extension) = 0;
};

MIDL_INTERFACE("cfcf64ef-9586-46d0-bca4-97cf2ca61b06")
ID3D11VkExtDevice1 : public ID3D11VkExtDevice {
    virtual bool STDMETHODCALLTYPE GetResourceHandleGPUVirtualAddressAndSizeNVX(
        void* hObject,
        uint64_t* gpuVAStart,
        uint64_t* gpuVASize) = 0;

    virtual bool STDMETHODCALLTYPE CreateUnorderedAccessViewAndGetDriverHandleNVX(
        ID3D11Resource * pResource,
        const D3D11_UNORDERED_ACCESS_VIEW_DESC* pDesc,
        ID3D11UnorderedAccessView** ppUAV,
        uint32_t* pDriverHandle) = 0;

    virtual bool STDMETHODCALLTYPE CreateShaderResourceViewAndGetDriverHandleNVX(
        ID3D11Resource * pResource,
        const D3D11_SHADER_RESOURCE_VIEW_DESC* pDesc,
        ID3D11ShaderResourceView** ppSRV,
        uint32_t* pDriverHandle) = 0;

    virtual bool STDMETHODCALLTYPE CreateSamplerStateAndGetDriverHandleNVX(
        const D3D11_SAMPLER_DESC* pSamplerDesc,
        ID3D11SamplerState** ppSamplerState,
        uint32_t* pDriverHandle) = 0;

    virtual bool STDMETHODCALLTYPE CreateCubinComputeShaderWithNameNVX(
        const void* pCubin,
        uint32_t size,
        uint32_t blockX,
        uint32_t blockY,
        uint32_t blockZ,
        const char* pShaderName,
        IUnknown** phShader) = 0;

    virtual bool STDMETHODCALLTYPE GetCudaTextureObjectNVX(
        uint32_t srvDriverHandle,
        uint32_t samplerDriverHandle,
        uint32_t* pCudaTextureHandle) = 0;
};

MIDL_INTERFACE("fd0bca13-5cb6-4c3a-987e-4750de2ca791")
ID3D11VkExtContext : public IUnknown {
    virtual void STDMETHODCALLTYPE MultiDrawIndirect(
        UINT DrawCount,
        ID3D11Buffer * pBufferForArgs,
        UINT ByteOffsetForArgs,
        UINT ByteStrideForArgs) = 0;

    virtual void STDMETHODCALLTYPE MultiDrawIndexedIndirect(
        UINT DrawCount,
        ID3D11Buffer * pBufferForArgs,
        UINT ByteOffsetForArgs,
        UINT ByteStrideForArgs) = 0;

    virtual void STDMETHODCALLTYPE MultiDrawIndirectCount(
        UINT MaxDrawCount,
        ID3D11Buffer * pBufferForCount,
        UINT ByteOffsetForCount,
        ID3D11Buffer * pBufferForArgs,
        UINT ByteOffsetForArgs,
        UINT ByteStrideForArgs) = 0;

    virtual void STDMETHODCALLTYPE MultiDrawIndexedIndirectCount(
        UINT MaxDrawCount,
        ID3D11Buffer * pBufferForCount,
        UINT ByteOffsetForCount,
        ID3D11Buffer * pBufferForArgs,
        UINT ByteOffsetForArgs,
        UINT ByteStrideForArgs) = 0;

    virtual void STDMETHODCALLTYPE SetDepthBoundsTest(
        BOOL Enable,
        FLOAT MinDepthBounds,
        FLOAT MaxDepthBounds) = 0;

    virtual void STDMETHODCALLTYPE SetBarrierControl(
        UINT ControlFlags) = 0;
};

MIDL_INTERFACE("874b09b2-ae0b-41d8-8476-5f3b7a0e879d")
ID3D11VkExtContext1 : public ID3D11VkExtContext {
    virtual bool STDMETHODCALLTYPE LaunchCubinShaderNVX(
        IUnknown * hShader,
        uint32_t gridX,
        uint32_t gridY,
        uint32_t gridZ,
        const void* pParams,
        uint32_t paramSize,
        void* const* pReadResources,
        uint32_t numReadResources,
        void* const* pWriteResources,
        uint32_t numWriteResources) = 0;
};

MIDL_INTERFACE("2a289dbd-2d0a-4a51-89f7-f2adce465cd6")
IDXGIVkInteropFactory1 : public IDXGIVkInteropFactory {
    virtual HRESULT STDMETHODCALLTYPE GetGlobalHDRState(
        DXGI_COLOR_SPACE_TYPE * pOutColorSpace,
        DXGI_HDR_METADATA_HDR10 * ppOutMetadata) = 0;

    virtual HRESULT STDMETHODCALLTYPE SetGlobalHDRState(
        DXGI_COLOR_SPACE_TYPE ColorSpace,
        const DXGI_HDR_METADATA_HDR10* pMetadata) = 0;
};

#ifndef _MSC_VER
__CRT_UUID_DECL(IDXGIVkInteropFactory, 0x4c5e1b0d, 0xb0c8, 0x4131, 0xbf, 0xd8, 0x9b, 0x24, 0x76, 0xf7, 0xf4, 0x08);
__CRT_UUID_DECL(IDXGIVkInteropFactory1, 0x2a289dbd, 0x2d0a, 0x4a51, 0x89, 0xf7, 0xf2, 0xad, 0xce, 0x46, 0x5c, 0xd6);
__CRT_UUID_DECL(IDXGIVkInteropAdapter, 0x3a6d8f2c, 0xb0e8, 0x4ab4, 0xb4, 0xdc, 0x4f, 0xd2, 0x48, 0x91, 0xbf, 0xa5);
__CRT_UUID_DECL(ID3D11VkExtDevice, 0x8a6e3c42, 0xf74c, 0x45b7, 0x82, 0x65, 0xa2, 0x31, 0xb6, 0x77, 0xca, 0x17);
__CRT_UUID_DECL(ID3D11VkExtDevice1, 0xcfcf64ef, 0x9586, 0x46d0, 0xbc, 0xa4, 0x97, 0xcf, 0x2c, 0xa6, 0x1b, 0x06);
__CRT_UUID_DECL(ID3D11VkExtContext, 0xfd0bca13, 0x5cb6, 0x4c3a, 0x98, 0x7e, 0x47, 0x50, 0xde, 0x2c, 0xa7, 0x91);
__CRT_UUID_DECL(ID3D11VkExtContext1, 0x874b09b2, 0xae0b, 0x41d8, 0x84, 0x76, 0x5f, 0x3b, 0x7a, 0x0e, 0x87, 0x9d);
#endif
