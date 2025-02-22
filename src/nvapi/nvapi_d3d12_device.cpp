#include "nvapi_d3d12_device.h"
#include "../util/com_pointer.h"
#include "../util/util_log.h"

namespace dxvk {
    std::unordered_map<ID3D12Device*, NvapiD3d12Device> NvapiD3d12Device::m_nvapiDeviceMap;
    std::mutex NvapiD3d12Device::m_mutex;

    std::unordered_map<NVDX_ObjectHandle, NvU32> NvapiD3d12Device::m_cubinSmemMap;
    std::mutex NvapiD3d12Device::m_cubinSmemMutex;

    void NvapiD3d12Device::Reset() {
        std::scoped_lock lock{m_mutex, m_cubinSmemMutex};
        m_nvapiDeviceMap.clear();
        m_cubinSmemMap.clear();
    }

    NvapiD3d12Device* NvapiD3d12Device::GetOrCreate(ID3D12Device* device) {
        std::scoped_lock lock{m_mutex};

        auto itF = m_nvapiDeviceMap.find(device);
        if (itF != m_nvapiDeviceMap.end())
            return &itF->second;

        Com<ID3D12DeviceExt> deviceExt;
        if (FAILED(device->QueryInterface(IID_PPV_ARGS(&deviceExt))))
            return nullptr;

        auto [itI, inserted] = m_nvapiDeviceMap.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(device),
            std::forward_as_tuple(deviceExt.ptr()));

        if (!inserted)
            return nullptr;

        return &itI->second;
    }

    uint32_t NvapiD3d12Device::FindCubinSmem(NVDX_ObjectHandle pShader) {
        std::scoped_lock lock(m_cubinSmemMutex);
        auto it = m_cubinSmemMap.find(pShader);
        if (it != m_cubinSmemMap.end())
            return it->second;

        log::info("Failed to find CuBIN in m_cubinSmemMap, defaulting to 0");
        return 0;
    }

    NvapiD3d12Device::NvapiD3d12Device(ID3D12DeviceExt* vkd3dDevice)
        : m_vkd3dDevice(static_cast<ID3D12DeviceExt2*>(vkd3dDevice)) {
        m_supportsNvxBinaryImport = vkd3dDevice->GetExtensionSupport(D3D12_VK_NVX_BINARY_IMPORT);
        m_supportsNvxImageViewHandle = vkd3dDevice->GetExtensionSupport(D3D12_VK_NVX_IMAGE_VIEW_HANDLE);

        if (m_supportsNvxBinaryImport && m_supportsNvxImageViewHandle) {
            if (Com<ID3D12DeviceExt2> deviceExt2; SUCCEEDED(m_vkd3dDevice->QueryInterface(IID_PPV_ARGS(&deviceExt2)))) {
                m_supportsCubin64bit = deviceExt2->SupportsCubin64bit();
            }
        }
    }

    HRESULT NvapiD3d12Device::CreateCubinComputeShaderWithName(const void* cubinData, NvU32 cubinSize, NvU32 blockX, NvU32 blockY, NvU32 blockZ, const char* shaderName, NVDX_ObjectHandle* pShader) {
        return CreateCubinComputeShaderEx(cubinData, cubinSize, blockX, blockY, blockZ, 0 /* smemSize */, shaderName, pShader);
    }

    HRESULT NvapiD3d12Device::CreateCubinComputeShaderEx(const void* cubinData, NvU32 cubinSize, NvU32 blockX, NvU32 blockY, NvU32 blockZ, NvU32 smemSize, const char* shaderName, NVDX_ObjectHandle* pShader) {
        if (!m_vkd3dDevice || !m_supportsNvxBinaryImport)
            return E_NOTIMPL;

        auto result = m_vkd3dDevice->CreateCubinComputeShaderWithName(cubinData, cubinSize, blockX, blockY, blockZ, shaderName, reinterpret_cast<D3D12_CUBIN_DATA_HANDLE**>(pShader));
        if (FAILED(result))
            return result;

        std::scoped_lock lock(m_cubinSmemMutex);
        m_cubinSmemMap.emplace(*pShader, smemSize);

        return result;
    }

    HRESULT NvapiD3d12Device::DestroyCubinComputeShader(NVDX_ObjectHandle shader) {
        if (!m_vkd3dDevice || !m_supportsNvxBinaryImport)
            return E_NOTIMPL;

        auto result = m_vkd3dDevice->DestroyCubinComputeShader(reinterpret_cast<D3D12_CUBIN_DATA_HANDLE*>(shader));
        if (FAILED(result))
            return result;

        std::scoped_lock lock(m_cubinSmemMutex);
        m_cubinSmemMap.erase(shader);

        return result;
    }

    HRESULT NvapiD3d12Device::GetCudaTextureObject(D3D12_CPU_DESCRIPTOR_HANDLE srvHandle, D3D12_CPU_DESCRIPTOR_HANDLE samplerHandle, NvU32* cudaTextureHandle) const {
        if (!m_vkd3dDevice || !m_supportsNvxImageViewHandle)
            return E_NOTIMPL;

        return m_vkd3dDevice->GetCudaTextureObject(srvHandle, samplerHandle, reinterpret_cast<UINT32*>(cudaTextureHandle));
    }

    HRESULT NvapiD3d12Device::GetCudaSurfaceObject(D3D12_CPU_DESCRIPTOR_HANDLE uavHandle, NvU32* cudaSurfaceHandle) const {
        if (!m_vkd3dDevice || !m_supportsNvxImageViewHandle)
            return E_NOTIMPL;

        return m_vkd3dDevice->GetCudaSurfaceObject(uavHandle, reinterpret_cast<UINT32*>(cudaSurfaceHandle));
    }

    HRESULT NvapiD3d12Device::CaptureUAVInfo(NVAPI_UAV_INFO* pUAVInfo) const {
        if (!m_vkd3dDevice || !m_supportsNvxImageViewHandle)
            return E_NOTIMPL;

        return m_vkd3dDevice->CaptureUAVInfo(reinterpret_cast<D3D12_UAV_INFO*>(pUAVInfo));
    }

    bool NvapiD3d12Device::IsFatbinPTXSupported() const {
        return m_vkd3dDevice && m_supportsNvxBinaryImport && m_supportsNvxImageViewHandle;
    }
}
