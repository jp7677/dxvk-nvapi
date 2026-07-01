#include "nvapi_d3d12_device.h"
#include "../util/util_log.h"

namespace dxvk {
    std::atomic<LONGLONG> NvapiD3d12Device::m_resetTimestamp;
    std::mutex NvapiD3d12Device::m_mutex;

    std::unordered_map<NVDX_ObjectHandle, NvU32> NvapiD3d12Device::m_cubinSmemMap;
    std::mutex NvapiD3d12Device::m_cubinSmemMutex;

    std::optional<bool> NvapiD3d12Device::m_cubin64bitSupportAvailable;

    void NvapiD3d12Device::Reset() {
        std::scoped_lock lock{m_mutex, m_cubinSmemMutex};
        LARGE_INTEGER count;
        QueryPerformanceCounter(&count);
        m_resetTimestamp.store(count.QuadPart, std::memory_order_release);
        m_cubinSmemMap.clear();
        m_cubin64bitSupportAvailable.reset();
    }

    bool NvapiD3d12Device::Cubin64bitSupportAvailable(NvapiResourceFactory* factory, NvapiAdapterRegistry* registry) {
        if (m_cubin64bitSupportAvailable.has_value())
            return m_cubin64bitSupportAvailable.value();

        if (!factory || !registry)
            return false;

        uint32_t adapterCount = registry->GetAdapterCount();
        for (uint32_t i = 0; i < adapterCount; ++i) {
            auto dxgiAdapter = registry->GetAdapter(i)->GetDxgiAdapter();

            auto d3d12Device = factory->CreateD3D12Device(dxgiAdapter, D3D_FEATURE_LEVEL_12_0);
            if (d3d12Device == nullptr)
                continue;

            Com<ID3D12DeviceExt2> d3d12DeviceExt2;
            if (FAILED(d3d12Device->QueryInterface(IID_PPV_ARGS(&d3d12DeviceExt2))))
                continue;

            if (d3d12DeviceExt2->SupportsCubin64bit())
                return m_cubin64bitSupportAvailable.emplace(true);
        }

        return m_cubin64bitSupportAvailable.emplace(false);
    }

    std::optional<NvapiD3d12Device> NvapiD3d12Device::GetOrCreate(ID3D12Device* device) {
        static constexpr GUID NvapiD3d12DeviceGuid = {0x0266efbf, 0x6dc7, 0x4017, {0xb4, 0x21, 0x3d, 0x93, 0xce, 0xd0, 0x90, 0x64}};

        NvapiD3d12Device nvapiDevice{nullptr};
        UINT size = sizeof(nvapiDevice);

        auto result = device->GetPrivateData(NvapiD3d12DeviceGuid, &size, &nvapiDevice);
        if (SUCCEEDED(result) && size == sizeof(nvapiDevice) && nvapiDevice.m_creationTimestamp > m_resetTimestamp.load(std::memory_order_acquire))
            return nvapiDevice;

        size = sizeof(nvapiDevice);
        std::scoped_lock lock{m_mutex};

        result = device->GetPrivateData(NvapiD3d12DeviceGuid, &size, &nvapiDevice);
        if (SUCCEEDED(result) && size == sizeof(nvapiDevice) && nvapiDevice.m_creationTimestamp > m_resetTimestamp.load(std::memory_order_acquire))
            return nvapiDevice;

        Com<ID3D12DeviceExt> deviceExt;
        if (FAILED(device->QueryInterface(IID_PPV_ARGS(&deviceExt))))
            return std::nullopt;

        LARGE_INTEGER count;
        QueryPerformanceCounter(&count);

        nvapiDevice = NvapiD3d12Device{deviceExt.ptr()};
        nvapiDevice.m_creationTimestamp = count.QuadPart;

        device->SetPrivateData(NvapiD3d12DeviceGuid, sizeof(nvapiDevice), &nvapiDevice);

        return nvapiDevice;
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
        : m_vkd3dDevice(static_cast<ID3D12DeviceExt5*>(vkd3dDevice)) { // NOLINT(*-pro-type-static-cast-downcast)
        if (!vkd3dDevice)
            return;

        uint32_t deviceExtTier = probeInterfaceChain(vkd3dDevice, {
                                                                      __uuidof(ID3D12DeviceExt1),
                                                                      __uuidof(ID3D12DeviceExt2),
                                                                      __uuidof(ID3D12DeviceExt3),
                                                                      __uuidof(ID3D12DeviceExt4),
                                                                      __uuidof(ID3D12DeviceExt5),
                                                                  });

        m_supportsNvxBinaryImport = vkd3dDevice->GetExtensionSupport(D3D12_VK_NVX_BINARY_IMPORT);
        m_supportsNvxImageViewHandle = vkd3dDevice->GetExtensionSupport(D3D12_VK_NVX_IMAGE_VIEW_HANDLE);
        m_supportsGlobalPipelineStateFlags = deviceExtTier >= 5;
        m_supportsOpacityMicromap = m_supportsGlobalPipelineStateFlags && vkd3dDevice->GetExtensionSupport(D3D12_VK_OPACITY_MICROMAP);

        if (deviceExtTier >= 2 && m_supportsNvxBinaryImport && m_supportsNvxImageViewHandle)
            m_supportsCubin64bit = m_vkd3dDevice->SupportsCubin64bit();
        if (deviceExtTier >= 4)
            m_supportsNvShaderExtn = true;
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

    HRESULT NvapiD3d12Device::CreateCubinComputeShaderExV2(D3D12_CREATE_CUBIN_SHADER_PARAMS* params) {
        if (!m_supportsCubin64bit)
            return E_NOTIMPL;

        auto result = m_vkd3dDevice->CreateCubinComputeShaderExV2(params);
        if (FAILED(result))
            return result;

        std::scoped_lock lock(m_cubinSmemMutex);
        m_cubinSmemMap.emplace(reinterpret_cast<NVDX_ObjectHandle>(params->hShader), params->dynSharedMemBytes);

        return result;
    }

    HRESULT NvapiD3d12Device::GetCudaMergedTextureSamplerObject(D3D12_GET_CUDA_MERGED_TEXTURE_SAMPLER_OBJECT_PARAMS* params) const {
        if (!m_supportsCubin64bit)
            return E_NOTIMPL;

        return m_vkd3dDevice->GetCudaMergedTextureSamplerObject(params);
    }

    HRESULT NvapiD3d12Device::GetCudaIndependentDescriptorObject(D3D12_GET_CUDA_INDEPENDENT_DESCRIPTOR_OBJECT_PARAMS* params) const {
        if (!m_supportsCubin64bit)
            return E_NOTIMPL;

        return m_vkd3dDevice->GetCudaIndependentDescriptorObject(params);
    }

    bool NvapiD3d12Device::IsNvShaderExtnOpCodeSupported(UINT32 opCode) const {
        return m_supportsNvShaderExtn && m_vkd3dDevice->IsNvShaderExtnOpCodeSupported(opCode);
    }

    HRESULT NvapiD3d12Device::SetNvShaderExtnSlotSpace(UINT32 uavSlot, UINT32 uavSpace, bool localThread) const {
        if (!m_supportsNvShaderExtn)
            return E_NOTIMPL;

        return m_vkd3dDevice->SetNvShaderExtnSlotSpace(uavSlot, uavSpace, localThread);
    }

    bool NvapiD3d12Device::SetCreatePipelineStateFlagsNVAPI(D3D12_VK_EXT_PIPELINE_CREATION_STATE_FLAG pipeline_state_flags) {
        return m_supportsGlobalPipelineStateFlags && m_vkd3dDevice->SetCreatePipelineStateFlagsNVAPI(pipeline_state_flags);
    }

    bool NvapiD3d12Device::IsOpacityMicromapSupported() const {
        return m_vkd3dDevice && m_supportsOpacityMicromap;
    }

    bool NvapiD3d12Device::IsOpacityMicromapSupported(ID3D12Device* d3dDevice) {
        auto device = GetOrCreate(d3dDevice);
        if (!device)
            return false;

        return device->IsOpacityMicromapSupported();
    }
}
