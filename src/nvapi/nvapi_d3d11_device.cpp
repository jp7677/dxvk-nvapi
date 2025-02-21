#include "nvapi_d3d11_device.h"
#include "../util/com_pointer.h"

namespace dxvk {
    std::unordered_map<IUnknown*, std::shared_ptr<NvapiD3d11Device>> NvapiD3d11Device::m_nvapiDeviceMap;
    std::mutex NvapiD3d11Device::m_mutex;

    void NvapiD3d11Device::Reset() {
        std::scoped_lock lock{m_mutex};
        m_nvapiDeviceMap.clear();
    }

    std::pair<Com<ID3D11VkExtDevice>, Com<ID3D11VkExtContext>> GetDxvkDevice(IUnknown* deviceOrContext) {
        Com<ID3D11Device> d3d11Device;
        Com<ID3D11DeviceContext> d3d11DeviceContext;

        if (SUCCEEDED(deviceOrContext->QueryInterface(IID_PPV_ARGS(&d3d11Device))))
            d3d11Device->GetImmediateContext(&d3d11DeviceContext);
        else if (SUCCEEDED(deviceOrContext->QueryInterface(IID_PPV_ARGS(&d3d11DeviceContext))))
            d3d11DeviceContext->GetDevice(&d3d11Device);

        if (d3d11Device == nullptr || d3d11DeviceContext == nullptr)
            return {};

        Com<ID3D11VkExtDevice> dxvkDevice;
        Com<ID3D11VkExtContext> dxvkContext;
        if (SUCCEEDED(d3d11Device->QueryInterface(IID_PPV_ARGS(&dxvkDevice))) && SUCCEEDED(d3d11DeviceContext->QueryInterface(IID_PPV_ARGS(&dxvkContext))))
            return std::make_pair(dxvkDevice, dxvkContext);

        return {};
    }

    NvapiD3d11Device* NvapiD3d11Device::GetOrCreate(IUnknown* deviceOrContext) {
        std::scoped_lock lock{m_mutex};

        if (auto nvapiD3D11Device = Get(deviceOrContext))
            return nvapiD3D11Device;

        auto [dxvkDevice, dxvkContext] = GetDxvkDevice(deviceOrContext);
        if (dxvkDevice == nullptr || dxvkContext == nullptr)
            return nullptr;

        // Look for a cache entry where NvapiD3d11Device's m_dxvkDevice and m_dxvkContext matches the ones we found
        auto itF = std::find_if(m_nvapiDeviceMap.begin(), m_nvapiDeviceMap.end(),
            [&dxvkDevice, &dxvkContext](auto& item) { return item.second->m_dxvkDevice == dxvkDevice.ptr() && item.second->m_dxvkContext == dxvkContext.ptr(); });

        auto [itI, inserted] = itF == m_nvapiDeviceMap.end()
            ? m_nvapiDeviceMap.emplace(deviceOrContext, std::make_shared<NvapiD3d11Device>(dxvkDevice.ptr(), dxvkContext.ptr()))
            : m_nvapiDeviceMap.emplace(deviceOrContext, itF->second);

        if (!inserted)
            return nullptr;

        return itI->second.get();
    }

    NvapiD3d11Device* NvapiD3d11Device::Get(IUnknown* deviceOrContext) {
        auto it = m_nvapiDeviceMap.find(deviceOrContext);
        return it == m_nvapiDeviceMap.end() ? nullptr : it->second.get();
    }

    NvapiD3d11Device::NvapiD3d11Device(ID3D11VkExtDevice* dxvkDevice, ID3D11VkExtContext* dxvkContext)
        : m_dxvkDevice(static_cast<ID3D11VkExtDevice1*>(dxvkDevice)), m_dxvkContext(static_cast<ID3D11VkExtContext1*>(dxvkContext)) { // NOLINT(*-pro-type-static-cast-downcast)
        m_supportsExtDepthBounds = m_dxvkDevice->GetExtensionSupport(D3D11_VK_EXT_DEPTH_BOUNDS);
        m_supportsNvxBinaryImport = m_dxvkDevice->GetExtensionSupport(D3D11_VK_NVX_BINARY_IMPORT);
        m_supportsExtBarrierControl = m_dxvkDevice->GetExtensionSupport(D3D11_VK_EXT_BARRIER_CONTROL);
        m_supportsNvxImageViewHandle = m_dxvkDevice->GetExtensionSupport(D3D11_VK_NVX_IMAGE_VIEW_HANDLE);
        m_supportsExtMultiDrawIndirect = m_dxvkDevice->GetExtensionSupport(D3D11_VK_EXT_MULTI_DRAW_INDIRECT);

        Com<ID3D11VkExtDevice1> dxvkDevice1;
        Com<ID3D11VkExtContext1> dxvkContext1;
        m_supportsExtDevice1 = SUCCEEDED(dxvkDevice->QueryInterface(IID_PPV_ARGS(&dxvkDevice1)));
        m_supportsExtContext1 = SUCCEEDED(dxvkContext->QueryInterface(IID_PPV_ARGS(&dxvkContext1)));
    }

    HRESULT NvapiD3d11Device::SetDepthBoundsTest(const bool enable, const float minDepth, const float maxDepth) const {
        if (!m_supportsExtDepthBounds)
            return E_NOTIMPL;

        m_dxvkContext->SetDepthBoundsTest(enable, minDepth, maxDepth);
        return S_OK;
    }

    HRESULT NvapiD3d11Device::BeginUAVOverlap() const {
        if (!m_supportsExtBarrierControl)
            return E_NOTIMPL;

        m_dxvkContext->SetBarrierControl(D3D11_VK_BARRIER_CONTROL_IGNORE_WRITE_AFTER_WRITE);
        return S_OK;
    }

    HRESULT NvapiD3d11Device::EndUAVOverlap() const {
        if (!m_supportsExtBarrierControl)
            return E_NOTIMPL;

        m_dxvkContext->SetBarrierControl(0U);
        return S_OK;
    }

    HRESULT NvapiD3d11Device::MultiDrawInstancedIndirect(const NvU32 drawCount, ID3D11Buffer* buffer, const NvU32 alignedByteOffsetForArgs, const NvU32 alignedByteStrideForArgs) const {
        if (!m_supportsExtMultiDrawIndirect)
            return E_NOTIMPL;

        m_dxvkContext->MultiDrawIndirect(drawCount, buffer, alignedByteOffsetForArgs, alignedByteStrideForArgs);
        return S_OK;
    }

    HRESULT NvapiD3d11Device::MultiDrawIndexedInstancedIndirect(const NvU32 drawCount, ID3D11Buffer* buffer, const NvU32 alignedByteOffsetForArgs, const NvU32 alignedByteStrideForArgs) const {
        if (!m_supportsExtMultiDrawIndirect)
            return E_NOTIMPL;

        m_dxvkContext->MultiDrawIndexedIndirect(drawCount, buffer, alignedByteOffsetForArgs, alignedByteStrideForArgs);
        return S_OK;
    }

    HRESULT NvapiD3d11Device::CreateCubinComputeShaderWithName(const void* pCubin, NvU32 size, NvU32 blockX, NvU32 blockY, NvU32 blockZ, const char* pShaderName, NVDX_ObjectHandle* phShader) const {
        if (!m_supportsExtDevice1 || !m_supportsNvxBinaryImport)
            return E_NOTIMPL;

        auto success = m_dxvkDevice->CreateCubinComputeShaderWithNameNVX(pCubin, size, blockX, blockY, blockZ, pShaderName, reinterpret_cast<IUnknown**>(phShader));
        return success ? S_OK : E_FAIL;
    }

    HRESULT NvapiD3d11Device::LaunchCubinShader(NVDX_ObjectHandle hShader, NvU32 gridX, NvU32 gridY, NvU32 gridZ, const void* pParams, NvU32 paramSize, const NVDX_ObjectHandle* pReadResources, NvU32 numReadResources, const NVDX_ObjectHandle* pWriteResources, NvU32 numWriteResources) const {
        if (!m_supportsExtContext1 || !m_supportsNvxBinaryImport)
            return E_NOTIMPL;

        auto success = m_dxvkContext->LaunchCubinShaderNVX(reinterpret_cast<IUnknown*>(hShader), gridX, gridY, gridZ, pParams, paramSize, reinterpret_cast<void* const*>(pReadResources), numReadResources, reinterpret_cast<void* const*>(pWriteResources), numWriteResources);
        return success ? S_OK : E_FAIL;
    }

    HRESULT NvapiD3d11Device::DestroyCubinShader(NVDX_ObjectHandle hShader) {
        if (auto cubinShader = reinterpret_cast<IUnknown*>(hShader)) {
            cubinShader->Release();
            return S_OK;
        }

        return E_FAIL;
    }

    void NvapiD3d11Device::GetResourceDriverHandle(ID3D11Resource* pResource, NVDX_ObjectHandle* phObject) {
        *phObject = reinterpret_cast<NVDX_ObjectHandle>(pResource);
    }

    HRESULT NvapiD3d11Device::GetResourceHandleGPUVirtualAddressAndSize(NVDX_ObjectHandle hObject, NvU64* gpuVAStart, NvU64* gpuVASize) const {
        if (!m_supportsExtDevice1 || !m_supportsNvxImageViewHandle)
            return E_NOTIMPL;

        auto success = m_dxvkDevice->GetResourceHandleGPUVirtualAddressAndSizeNVX(hObject, gpuVAStart, gpuVASize);
        return success ? S_OK : E_FAIL;
    }

    HRESULT NvapiD3d11Device::CreateUnorderedAccessViewAndGetDriverHandle(ID3D11Resource* pResource, const D3D11_UNORDERED_ACCESS_VIEW_DESC* pDesc, ID3D11UnorderedAccessView** ppUAV, NvU32* pDriverHandle) const {
        if (!m_supportsExtDevice1 || !m_supportsNvxImageViewHandle)
            return E_NOTIMPL;

        auto success = m_dxvkDevice->CreateUnorderedAccessViewAndGetDriverHandleNVX(pResource, pDesc, ppUAV, reinterpret_cast<uint32_t*>(pDriverHandle));
        return success ? S_OK : E_FAIL;
    }

    HRESULT NvapiD3d11Device::CreateShaderResourceViewAndGetDriverHandle(ID3D11Resource* pResource, const D3D11_SHADER_RESOURCE_VIEW_DESC* pDesc, ID3D11ShaderResourceView** ppSRV, NvU32* pDriverHandle) const {
        if (!m_supportsExtDevice1 || !m_supportsNvxImageViewHandle)
            return E_NOTIMPL;

        auto success = m_dxvkDevice->CreateShaderResourceViewAndGetDriverHandleNVX(pResource, pDesc, ppSRV, reinterpret_cast<uint32_t*>(pDriverHandle));
        return success ? S_OK : E_FAIL;
    }

    HRESULT NvapiD3d11Device::GetCudaTextureObject(uint32_t srvDriverHandle, uint32_t samplerDriverHandle, uint32_t* pCudaTextureHandle) const {
        if (!m_supportsExtDevice1 || !m_supportsNvxImageViewHandle)
            return E_NOTIMPL;

        auto success = m_dxvkDevice->GetCudaTextureObjectNVX(srvDriverHandle, samplerDriverHandle, pCudaTextureHandle);
        return success ? S_OK : E_FAIL;
    }

    HRESULT NvapiD3d11Device::CreateSamplerStateAndGetDriverHandle(const D3D11_SAMPLER_DESC* pSamplerDesc, ID3D11SamplerState** ppSamplerState, uint32_t* pDriverHandle) const {
        if (!m_supportsExtDevice1 || !m_supportsNvxImageViewHandle)
            return E_NOTIMPL;

        auto success = m_dxvkDevice->CreateSamplerStateAndGetDriverHandleNVX(pSamplerDesc, ppSamplerState, pDriverHandle);
        return success ? S_OK : E_FAIL;
    }

    bool NvapiD3d11Device::IsFatbinPTXSupported() const {
        return m_supportsExtDevice1 && m_supportsExtContext1 && m_supportsNvxBinaryImport && m_supportsNvxImageViewHandle;
    }
}
