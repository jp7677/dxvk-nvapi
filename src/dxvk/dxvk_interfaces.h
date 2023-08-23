#pragma once

#include "../nvapi_private.h"

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#endif // __GNUC__

#define DXVK_DEFINE_GUID(iface) \
    template <>                 \
    inline GUID const& __mingw_uuidof<iface>() { return iface::guid; }

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
    static const GUID guid;

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
    static const GUID guid;

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
    static const GUID guid;

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
    static const GUID guid;

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
        uint32_t * pCudaTextureHandle) = 0;
};

MIDL_INTERFACE("fd0bca13-5cb6-4c3a-987e-4750de2ca791")
ID3D11VkExtContext : public IUnknown {
    static const GUID guid;

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
    static const GUID guid;

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
    static const GUID guid;

    virtual HRESULT STDMETHODCALLTYPE GetGlobalHDRState(
        DXGI_COLOR_SPACE_TYPE * pOutColorSpace,
        DXGI_HDR_METADATA_HDR10 * ppOutMetadata) = 0;

    virtual HRESULT STDMETHODCALLTYPE SetGlobalHDRState(
        DXGI_COLOR_SPACE_TYPE ColorSpace,
        const DXGI_HDR_METADATA_HDR10* pMetadata) = 0;
};

DXVK_DEFINE_GUID(IDXGIVkInteropFactory)
DXVK_DEFINE_GUID(IDXGIVkInteropFactory1)
DXVK_DEFINE_GUID(IDXGIVkInteropAdapter)
DXVK_DEFINE_GUID(ID3D11VkExtDevice)
DXVK_DEFINE_GUID(ID3D11VkExtDevice1)
DXVK_DEFINE_GUID(ID3D11VkExtContext)
DXVK_DEFINE_GUID(ID3D11VkExtContext1)
