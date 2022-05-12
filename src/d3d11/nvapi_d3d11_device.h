#pragma once

#include "../nvapi_private.h"
#include "../dxvk/dxvk_interfaces.h"
#include "../util/com_pointer.h"

namespace dxvk {
    class NvapiD3d11Device {

      public:
        static bool SetDepthBoundsTest(IUnknown* deviceOrContext, bool enable, float minDepth, float maxDepth);
        static bool BeginUAVOverlap(IUnknown* deviceOrContext);
        static bool EndUAVOverlap(IUnknown* deviceOrContext);
        static bool MultiDrawInstancedIndirect(ID3D11DeviceContext* deviceContext, NvU32 drawCount, ID3D11Buffer* buffer, NvU32 alignedByteOffsetForArgs, NvU32 alignedByteStrideForArgs);
        static bool MultiDrawIndexedInstancedIndirect(ID3D11DeviceContext* deviceContext, NvU32 drawCount, ID3D11Buffer* buffer, NvU32 alignedByteOffsetForArgs, NvU32 alignedByteStrideForArgs);
        static bool CreateCubinComputeShaderWithName(ID3D11Device* pDevice, const void* pCubin, NvU32 size, NvU32 blockX, NvU32 blockY, NvU32 blockZ, const char* pShaderName, NVDX_ObjectHandle* phShader);
        static bool LaunchCubinShader(ID3D11DeviceContext* pDeviceContext, NVDX_ObjectHandle hShader, NvU32 gridX, NvU32 gridY, NvU32 gridZ, const void* pParams, NvU32 paramSize, const NVDX_ObjectHandle* pReadResources, NvU32 numReadResources, const NVDX_ObjectHandle* pWriteResources, NvU32 numWriteResources);
        static bool DestroyCubinShader(NVDX_ObjectHandle hShader);
        static bool GetResourceDriverHandle(ID3D11Resource* pResource, NVDX_ObjectHandle* phObject);
        static bool GetResourceHandleGPUVirtualAddressAndSize(ID3D11Device* pDevice, NVDX_ObjectHandle hObject, NvU64* gpuVAStart, NvU64* gpuVASize);
        static bool CreateUnorderedAccessViewAndGetDriverHandle(ID3D11Device* pDevice, ID3D11Resource* pResource, const D3D11_UNORDERED_ACCESS_VIEW_DESC* pDesc, ID3D11UnorderedAccessView** ppUAV, NvU32* pDriverHandle);
        static bool CreateShaderResourceViewAndGetDriverHandle(ID3D11Device* pDevice, ID3D11Resource* pResource, const D3D11_SHADER_RESOURCE_VIEW_DESC* pDesc, ID3D11ShaderResourceView** ppSRV, NvU32* pDriverHandle);
        static bool GetCudaTextureObject(ID3D11Device* pDevice, uint32_t srvDriverHandle, uint32_t samplerDriverHandle, uint32_t* pCudaTextureHandle);
        static bool CreateSamplerStateAndGetDriverHandle(ID3D11Device* pDevice, const D3D11_SAMPLER_DESC* pSamplerDesc, ID3D11SamplerState** ppSamplerState, uint32_t* pDriverHandle);
        static bool IsFatbinPTXSupported(ID3D11Device* pDevice);

      private:
        inline static std::unordered_map<IUnknown*, ID3D11VkExtContext*> m_depthBoundsDeviceOrContextMap;
        inline static std::unordered_map<IUnknown*, ID3D11VkExtContext*> m_barrierControlDeviceOrContextMap;
        inline static std::unordered_map<IUnknown*, ID3D11VkExtContext*> m_multiDrawIndirectContextMap;
        inline static std::unordered_map<IUnknown*, ID3D11VkExtContext*> m_binaryImportContextMap;

        inline static std::mutex m_depthBoundsDeviceOrContextMutex;
        inline static std::mutex m_barrierControlDeviceOrContextMutex;
        inline static std::mutex m_multiDrawIndirectContextMutex;
        inline static std::mutex m_binaryImportContextMutex;

        [[nodiscard]] static Com<ID3D11VkExtContext> GetSdbtDeviceContext(IUnknown* deviceOrContext);
        [[nodiscard]] static Com<ID3D11VkExtContext> GetBarrierControlDeviceContext(IUnknown* deviceOrContext);
        [[nodiscard]] static Com<ID3D11VkExtContext> GetMultiDrawDeviceContext(ID3D11DeviceContext* deviceContext);
        [[nodiscard]] static Com<ID3D11VkExtContext1> GetBinaryImportDeviceContext(IUnknown* deviceOrContext);
        [[nodiscard]] static Com<ID3D11VkExtContext> GetCachedDeviceContextExt(IUnknown* deviceOrContext, std::unordered_map<IUnknown*, ID3D11VkExtContext*>& cacheMap, D3D11_VK_EXTENSION extension);
        [[nodiscard]] static Com<ID3D11VkExtContext> GetDeviceContextExt(IUnknown* deviceOrContext, D3D11_VK_EXTENSION extension);
        [[nodiscard]] static Com<ID3D11VkExtContext> GetDeviceContextExt(ID3D11DeviceContext* deviceContext, D3D11_VK_EXTENSION extension);
        [[nodiscard]] static Com<ID3D11VkExtContext> GetDeviceContextExt(ID3D11Device* device, ID3D11DeviceContext* deviceContext, D3D11_VK_EXTENSION extension);
    };
}
