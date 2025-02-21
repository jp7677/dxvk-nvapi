#pragma once

#include "../nvapi_private.h"
#include "../interfaces/dxvk_interfaces.h"

namespace dxvk {
    class NvapiD3d11Device {

      public:
        static void Reset();
        [[nodiscard]] static NvapiD3d11Device* GetOrCreate(IUnknown* deviceOrContext);

        explicit NvapiD3d11Device(ID3D11VkExtDevice* dxvkDevice, ID3D11VkExtContext* dxvkContext);

        [[nodiscard]] HRESULT SetDepthBoundsTest(bool enable, float minDepth, float maxDepth) const;
        [[nodiscard]] HRESULT BeginUAVOverlap() const;
        [[nodiscard]] HRESULT EndUAVOverlap() const;
        [[nodiscard]] HRESULT MultiDrawInstancedIndirect(NvU32 drawCount, ID3D11Buffer* buffer, NvU32 alignedByteOffsetForArgs, NvU32 alignedByteStrideForArgs) const;
        [[nodiscard]] HRESULT MultiDrawIndexedInstancedIndirect(NvU32 drawCount, ID3D11Buffer* buffer, NvU32 alignedByteOffsetForArgs, NvU32 alignedByteStrideForArgs) const;

        [[nodiscard]] HRESULT CreateCubinComputeShaderWithName(const void* pCubin, NvU32 size, NvU32 blockX, NvU32 blockY, NvU32 blockZ, const char* pShaderName, NVDX_ObjectHandle* phShader) const;
        [[nodiscard]] HRESULT LaunchCubinShader(NVDX_ObjectHandle hShader, NvU32 gridX, NvU32 gridY, NvU32 gridZ, const void* pParams, NvU32 paramSize, const NVDX_ObjectHandle* pReadResources, NvU32 numReadResources, const NVDX_ObjectHandle* pWriteResources, NvU32 numWriteResources) const;
        [[nodiscard]] HRESULT DestroyCubinShader(NVDX_ObjectHandle hShader);

        void GetResourceDriverHandle(ID3D11Resource* pResource, NVDX_ObjectHandle* phObject);
        [[nodiscard]] HRESULT GetResourceHandleGPUVirtualAddressAndSize(NVDX_ObjectHandle hObject, NvU64* gpuVAStart, NvU64* gpuVASize) const;
        [[nodiscard]] HRESULT CreateUnorderedAccessViewAndGetDriverHandle(ID3D11Resource* pResource, const D3D11_UNORDERED_ACCESS_VIEW_DESC* pDesc, ID3D11UnorderedAccessView** ppUAV, NvU32* pDriverHandle) const;
        [[nodiscard]] HRESULT CreateShaderResourceViewAndGetDriverHandle(ID3D11Resource* pResource, const D3D11_SHADER_RESOURCE_VIEW_DESC* pDesc, ID3D11ShaderResourceView** ppSRV, NvU32* pDriverHandle) const;
        [[nodiscard]] HRESULT GetCudaTextureObject(uint32_t srvDriverHandle, uint32_t samplerDriverHandle, uint32_t* pCudaTextureHandle) const;
        [[nodiscard]] HRESULT CreateSamplerStateAndGetDriverHandle(const D3D11_SAMPLER_DESC* pSamplerDesc, ID3D11SamplerState** ppSamplerState, uint32_t* pDriverHandle) const;
        [[nodiscard]] bool IsFatbinPTXSupported() const;

      private:
        [[nodiscard]] static NvapiD3d11Device* Get(IUnknown* deviceOrContext);

        static std::unordered_map<IUnknown*, std::shared_ptr<NvapiD3d11Device>> m_nvapiDeviceMap;
        static std::mutex m_mutex;

        ID3D11VkExtDevice1* m_dxvkDevice{};
        ID3D11VkExtContext1* m_dxvkContext{};

        bool m_supportsExtDepthBounds;
        bool m_supportsNvxBinaryImport;
        bool m_supportsExtBarrierControl;
        bool m_supportsNvxImageViewHandle;
        bool m_supportsExtMultiDrawIndirect;
        bool m_supportsExtDevice1;
        bool m_supportsExtContext1;
    };
}
