#include "nvapi_d3d11_device.h"

namespace dxvk {
    bool NvapiD3d11Device::SetDepthBoundsTest(IUnknown* deviceOrContext, const bool enable, const float minDepth, const float maxDepth) {
        auto sdbtDeviceContext = GetSdbtDeviceContext(deviceOrContext);
        if (sdbtDeviceContext == nullptr)
            return false;

        sdbtDeviceContext->SetDepthBoundsTest(enable, minDepth, maxDepth);
        return true;
    }

    bool NvapiD3d11Device::BeginUAVOverlap(IUnknown* deviceOrContext) {
        auto barrierControlDeviceContext = GetBarrierControlDeviceContext(deviceOrContext);
        if (barrierControlDeviceContext == nullptr)
            return false;

        barrierControlDeviceContext->SetBarrierControl(D3D11_VK_BARRIER_CONTROL_IGNORE_WRITE_AFTER_WRITE);
        return true;
    }

    bool NvapiD3d11Device::EndUAVOverlap(IUnknown* deviceOrContext) {
        auto barrierControlDeviceContext = GetBarrierControlDeviceContext(deviceOrContext);
        if (barrierControlDeviceContext == nullptr)
            return false;

        barrierControlDeviceContext->SetBarrierControl(0U);
        return true;
    }

    bool NvapiD3d11Device::MultiDrawInstancedIndirect(ID3D11DeviceContext* deviceContext, const NvU32 drawCount, ID3D11Buffer* buffer, const NvU32 alignedByteOffsetForArgs, const NvU32 alignedByteStrideForArgs) {
        auto multiDrawDeviceContext = GetMultiDrawDeviceContext(deviceContext);
        if (multiDrawDeviceContext == nullptr)
            return false;

        multiDrawDeviceContext->MultiDrawIndirect(drawCount, buffer, alignedByteOffsetForArgs, alignedByteStrideForArgs);
        return true;
    }

    bool NvapiD3d11Device::MultiDrawIndexedInstancedIndirect(ID3D11DeviceContext* deviceContext, const NvU32 drawCount, ID3D11Buffer* buffer, const NvU32 alignedByteOffsetForArgs, const NvU32 alignedByteStrideForArgs) {
        auto multiDrawDeviceContext = GetMultiDrawDeviceContext(deviceContext);
        if (multiDrawDeviceContext == nullptr)
            return false;

        multiDrawDeviceContext->MultiDrawIndexedIndirect(drawCount, buffer, alignedByteOffsetForArgs, alignedByteStrideForArgs);
        return true;
    }

    bool NvapiD3d11Device::CreateCubinComputeShaderWithName(ID3D11Device* pDevice, const void* pCubin, NvU32 size, NvU32 blockX, NvU32 blockY, NvU32 blockZ, const char* pShaderName, NVDX_ObjectHandle* phShader) {
        // note - creating a cuda kernel is a one-shot thing, no need to add complexity by caching availability of the underlying extensions, we'll just let DXVK tell us if it failed.
        Com<ID3D11VkExtDevice1> d3d11ExtDevice1;
        if (FAILED(pDevice->QueryInterface(IID_PPV_ARGS(&d3d11ExtDevice1))))
            return false;

        return d3d11ExtDevice1->CreateCubinComputeShaderWithNameNVX(pCubin, size, blockX, blockY, blockZ, pShaderName, reinterpret_cast<IUnknown**>(phShader));
    }

    bool NvapiD3d11Device::LaunchCubinShader(ID3D11DeviceContext* pDeviceContext, NVDX_ObjectHandle hShader, NvU32 gridX, NvU32 gridY, NvU32 gridZ, const void* pParams, NvU32 paramSize, const NVDX_ObjectHandle* pReadResources, NvU32 numReadResources, const NVDX_ObjectHandle* pWriteResources, NvU32 numWriteResources) {
        auto binaryImportDeviceContext = GetBinaryImportDeviceContext(pDeviceContext);
        if (binaryImportDeviceContext == nullptr)
            return false;

        return binaryImportDeviceContext->LaunchCubinShaderNVX(reinterpret_cast<IUnknown*>(hShader), gridX, gridY, gridZ, pParams, paramSize, reinterpret_cast<void* const*>(pReadResources), numReadResources, reinterpret_cast<void* const*>(pWriteResources), numWriteResources);
    }

    bool NvapiD3d11Device::DestroyCubinShader(NVDX_ObjectHandle hShader) {
        if (auto cubinShader = reinterpret_cast<IUnknown*>(hShader)) {
            cubinShader->Release();
            return true;
        }

        return false;
    }

    bool NvapiD3d11Device::GetResourceDriverHandle(ID3D11Resource* pResource, NVDX_ObjectHandle* phObject) {
        // This trivial implementation is here instead of in DXVK for performance reasons; DXVK will assume that this is a straight cast though, so if either layer has to change that assumption then they will have to be upgraded in lockstep.
        *phObject = reinterpret_cast<NVDX_ObjectHandle>(pResource);
        return true;
    }

    bool NvapiD3d11Device::GetResourceHandleGPUVirtualAddressAndSize(ID3D11Device* pDevice, NVDX_ObjectHandle hObject, NvU64* gpuVAStart, NvU64* gpuVASize) {
        Com<ID3D11VkExtDevice1> d3d11ExtDevice1;
        if (FAILED(pDevice->QueryInterface(IID_PPV_ARGS(&d3d11ExtDevice1))))
            return false;

        return d3d11ExtDevice1->GetResourceHandleGPUVirtualAddressAndSizeNVX(reinterpret_cast<void*>(hObject), reinterpret_cast<uint64_t*>(gpuVAStart), reinterpret_cast<uint64_t*>(gpuVASize));
    }

    bool NvapiD3d11Device::CreateUnorderedAccessViewAndGetDriverHandle(ID3D11Device* pDevice, ID3D11Resource* pResource, const D3D11_UNORDERED_ACCESS_VIEW_DESC* pDesc, ID3D11UnorderedAccessView** ppUAV, NvU32* pDriverHandle) {
        Com<ID3D11VkExtDevice1> d3d11ExtDevice1;
        if (FAILED(pDevice->QueryInterface(IID_PPV_ARGS(&d3d11ExtDevice1))))
            return false;

        return d3d11ExtDevice1->CreateUnorderedAccessViewAndGetDriverHandleNVX(pResource, pDesc, ppUAV, reinterpret_cast<uint32_t*>(pDriverHandle));
    }

    bool NvapiD3d11Device::CreateShaderResourceViewAndGetDriverHandle(ID3D11Device* pDevice, ID3D11Resource* pResource, const D3D11_SHADER_RESOURCE_VIEW_DESC* pDesc, ID3D11ShaderResourceView** ppSRV, NvU32* pDriverHandle) {
        Com<ID3D11VkExtDevice1> d3d11ExtDevice1;
        if (FAILED(pDevice->QueryInterface(IID_PPV_ARGS(&d3d11ExtDevice1))))
            return false;

        return d3d11ExtDevice1->CreateShaderResourceViewAndGetDriverHandleNVX(pResource, pDesc, ppSRV, reinterpret_cast<uint32_t*>(pDriverHandle));
    }

    bool NvapiD3d11Device::GetCudaTextureObject(ID3D11Device* pDevice, uint32_t srvDriverHandle, uint32_t samplerDriverHandle, uint32_t* pCudaTextureHandle) {
        // note - deriving a 'cuda texture object' is typically a one-shot thing, no need to add complexity by caching availability of the underlying extensions, we'll just let DXVK tell us if it failed.
        Com<ID3D11VkExtDevice1> d3d11ExtDevice1;
        if (FAILED(pDevice->QueryInterface(IID_PPV_ARGS(&d3d11ExtDevice1))))
            return false;

        return d3d11ExtDevice1->GetCudaTextureObjectNVX(srvDriverHandle, samplerDriverHandle, reinterpret_cast<uint32_t*>(pCudaTextureHandle));
    }

    bool NvapiD3d11Device::CreateSamplerStateAndGetDriverHandle(ID3D11Device* pDevice, const D3D11_SAMPLER_DESC* pSamplerDesc, ID3D11SamplerState** ppSamplerState, uint32_t* pDriverHandle) {
        // note - getting the driver handle for a sampler state is typically a one-shot thing, no need to add complexity by caching availability of the underlying extensions, we'll just let DXVK tell us if it failed.
        Com<ID3D11VkExtDevice1> d3d11ExtDevice1;
        if (FAILED(pDevice->QueryInterface(IID_PPV_ARGS(&d3d11ExtDevice1))))
            return false;

        return d3d11ExtDevice1->CreateSamplerStateAndGetDriverHandleNVX(pSamplerDesc, ppSamplerState, pDriverHandle);
    }

    bool NvapiD3d11Device::IsFatbinPTXSupported(ID3D11Device* pDevice) {
        return GetBinaryImportDeviceContext(pDevice) != nullptr;
    }

    Com<ID3D11VkExtContext> NvapiD3d11Device::GetSdbtDeviceContext(IUnknown* deviceOrContext) {
        std::scoped_lock lock(m_depthBoundsDeviceOrContextMutex);
        return GetCachedDeviceContextExt(deviceOrContext, m_depthBoundsDeviceOrContextMap, D3D11_VK_EXT_DEPTH_BOUNDS);
    }

    Com<ID3D11VkExtContext> NvapiD3d11Device::GetBarrierControlDeviceContext(IUnknown* deviceOrContext) {
        std::scoped_lock lock(m_barrierControlDeviceOrContextMutex);
        return GetCachedDeviceContextExt(deviceOrContext, m_barrierControlDeviceOrContextMap, D3D11_VK_EXT_BARRIER_CONTROL);
    }

    Com<ID3D11VkExtContext> NvapiD3d11Device::GetMultiDrawDeviceContext(ID3D11DeviceContext* deviceContext) {
        std::scoped_lock lock(m_multiDrawIndirectContextMutex);
        return GetCachedDeviceContextExt(deviceContext, m_multiDrawIndirectContextMap, D3D11_VK_EXT_MULTI_DRAW_INDIRECT);
    }

    // We are going to have single map for storing devices with extensions D3D11_VK_NVX_BINARY_IMPORT & D3D11_VK_NVX_IMAGE_VIEW_HANDLE.
    // These are specific to NVIDIA and both of these extensions goes together.
    Com<ID3D11VkExtContext1> NvapiD3d11Device::GetBinaryImportDeviceContext(IUnknown* deviceOrContext) {
        std::scoped_lock lock(m_binaryImportContextMutex);
        auto context = GetCachedDeviceContextExt(deviceOrContext, m_binaryImportContextMap, D3D11_VK_NVX_BINARY_IMPORT);
        if (context == nullptr)
            return nullptr;

        Com<ID3D11VkExtContext1> d3d11ExtContext1;
        if (FAILED(context->QueryInterface(IID_PPV_ARGS(&d3d11ExtContext1))))
            return nullptr;

        return d3d11ExtContext1;
    }

    Com<ID3D11VkExtContext> NvapiD3d11Device::GetCachedDeviceContextExt(IUnknown* deviceOrContext, std::unordered_map<IUnknown*, ID3D11VkExtContext*>& cacheMap, D3D11_VK_EXTENSION extension) {
        auto it = cacheMap.find(deviceOrContext);
        if (it != cacheMap.end())
            return it->second;

        auto deviceContextExt = GetDeviceContextExt(deviceOrContext, extension);
        if (deviceContextExt != nullptr)
            cacheMap.emplace(deviceOrContext, deviceContextExt.ptr());

        return deviceContextExt;
    }

    Com<ID3D11VkExtContext> NvapiD3d11Device::GetDeviceContextExt(IUnknown* deviceOrContext, D3D11_VK_EXTENSION extension) {
        Com<ID3D11Device> device;
        Com<ID3D11DeviceContext> deviceContext;
        if (SUCCEEDED(deviceOrContext->QueryInterface(IID_PPV_ARGS(&device))))
            device->GetImmediateContext(&deviceContext);
        else if (SUCCEEDED(deviceOrContext->QueryInterface(IID_PPV_ARGS(&deviceContext))))
            deviceContext->GetDevice(&device);
        else
            return nullptr;

        return GetDeviceContextExt(device.ptr(), deviceContext.ptr(), extension);
    }

    Com<ID3D11VkExtContext> NvapiD3d11Device::GetDeviceContextExt(ID3D11DeviceContext* deviceContext, D3D11_VK_EXTENSION extension) {
        Com<ID3D11Device> device;
        deviceContext->GetDevice(&device);

        return GetDeviceContextExt(device.ptr(), deviceContext, extension);
    }

    Com<ID3D11VkExtContext> NvapiD3d11Device::GetDeviceContextExt(ID3D11Device* device, ID3D11DeviceContext* deviceContext, D3D11_VK_EXTENSION extension) {
        Com<ID3D11VkExtDevice> deviceExt;
        if (FAILED(device->QueryInterface(IID_PPV_ARGS(&deviceExt))))
            return nullptr;

        if (!deviceExt->GetExtensionSupport(extension))
            return nullptr;

        Com<ID3D11VkExtContext> deviceContextExt;
        if (FAILED(deviceContext->QueryInterface(IID_PPV_ARGS(&deviceContextExt))))
            return nullptr;

        return deviceContextExt;
    }

    void NvapiD3d11Device::Reset() {
        std::scoped_lock depthBoundsDeviceOrContextLock(m_depthBoundsDeviceOrContextMutex);
        std::scoped_lock barrierControlDeviceOrContextLock(m_barrierControlDeviceOrContextMutex);
        std::scoped_lock multiDrawIndirectContextLock(m_multiDrawIndirectContextMutex);
        std::scoped_lock binaryImportContextLock(m_binaryImportContextMutex);

        m_depthBoundsDeviceOrContextMap.clear();
        m_barrierControlDeviceOrContextMap.clear();
        m_multiDrawIndirectContextMap.clear();
        m_binaryImportContextMap.clear();
    }
}
