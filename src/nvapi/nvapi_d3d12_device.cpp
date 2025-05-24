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
        : m_vkd3dDevice(static_cast<ID3D12DeviceExt4*>(vkd3dDevice)) {
        m_supportsNvxBinaryImport = vkd3dDevice->GetExtensionSupport(D3D12_VK_NVX_BINARY_IMPORT);
        m_supportsNvxImageViewHandle = vkd3dDevice->GetExtensionSupport(D3D12_VK_NVX_IMAGE_VIEW_HANDLE);

        if (m_supportsNvxBinaryImport && m_supportsNvxImageViewHandle) {
            if (Com<ID3D12DeviceExt2> deviceExt2; SUCCEEDED(m_vkd3dDevice->QueryInterface(IID_PPV_ARGS(&deviceExt2)))) {
                m_supportsCubin64bit = deviceExt2->SupportsCubin64bit();
            }
        }

        if (Com<ID3D12DeviceExt3> deviceExt3; SUCCEEDED(m_vkd3dDevice->QueryInterface(IID_PPV_ARGS(&deviceExt3)))) {
            m_supportsDeviceExt3 = true;
            m_supportsOpacityMicromap = vkd3dDevice->GetExtensionSupport(D3D12_VK_EXT_OPACITY_MICROMAP);
        }

        if (Com<ID3D12DeviceExt4> deviceExt4; SUCCEEDED(m_vkd3dDevice->QueryInterface(IID_PPV_ARGS(&deviceExt4)))) {
            m_supportsDeviceExt4 = true;
            m_supportsClusterAccelerationStructure = vkd3dDevice->GetExtensionSupport(D3D12_VK_NV_CLUSTER_ACCELERATION_STRUCTURE);
            m_supportsPartitionedAccelerationStructure = vkd3dDevice->GetExtensionSupport(D3D12_VK_NV_PARTITIONED_ACCELERATION_STRUCTURE);
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

    HRESULT NvapiD3d12Device::CreateCubinComputeShaderExV2(D3D12_CREATE_CUBIN_SHADER_PARAMS* params) {
        if (!m_supportsCubin64bit)
            return E_NOTIMPL;

        auto result = m_vkd3dDevice->CreateCubinComputeShaderExV2(params);

        if (result == S_OK) {
            std::scoped_lock lock(m_cubinSmemMutex);
            m_cubinSmemMap.emplace(reinterpret_cast<NVDX_ObjectHandle>(params->hShader), params->dynSharedMemBytes);
        }

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

    bool NvapiD3d12Device::IsOpacityMicromapSupported() const {
        return m_supportsOpacityMicromap;
    }

    NvAPI_Status NvapiD3d12Device::SetCreatePipelineStateOptions(const NVAPI_D3D12_SET_CREATE_PIPELINE_STATE_OPTIONS_PARAMS* params) const {
        if (!m_supportsDeviceExt3)
            return NVAPI_NO_IMPLEMENTATION;

        return static_cast<NvAPI_Status>(m_vkd3dDevice->SetCreatePipelineStateOptions(params));
    }

    NvAPI_Status NvapiD3d12Device::CheckDriverMatchingIdentifierEx(NVAPI_CHECK_DRIVER_MATCHING_IDENTIFIER_EX_PARAMS* params) const {
        if (!m_supportsDeviceExt3)
            return NVAPI_NO_IMPLEMENTATION;

        return static_cast<NvAPI_Status>(m_vkd3dDevice->CheckDriverMatchingIdentifierEx(params));
    }

    NvAPI_Status NvapiD3d12Device::GetRaytracingAccelerationStructurePrebuildInfoEx(NVAPI_GET_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO_EX_PARAMS* params) const {
        if (!m_supportsDeviceExt3)
            return NVAPI_NO_IMPLEMENTATION;

        return static_cast<NvAPI_Status>(m_vkd3dDevice->GetRaytracingAccelerationStructurePrebuildInfoEx(params));
    }

    NvAPI_Status NvapiD3d12Device::GetRaytracingOpacityMicromapArrayPrebuildInfo(NVAPI_GET_RAYTRACING_OPACITY_MICROMAP_ARRAY_PREBUILD_INFO_PARAMS* params) const {
        if (!m_supportsDeviceExt3 || !m_supportsOpacityMicromap)
            return NVAPI_NO_IMPLEMENTATION;

        return static_cast<NvAPI_Status>(m_vkd3dDevice->GetRaytracingOpacityMicromapArrayPrebuildInfo(params));
    }

    bool NvapiD3d12Device::IsClusterAccelerationStructureSupported() const {
        return m_supportsClusterAccelerationStructure;
    }

    bool NvapiD3d12Device::IsPartitionedAccelerationStructureSupported() const {
        return m_supportsPartitionedAccelerationStructure;
    }

    bool NvapiD3d12Device::IsNvShaderExtnOpCodeSupported(uint32_t opCode) const {
        return m_supportsDeviceExt4 && m_vkd3dDevice->IsNvShaderExtnOpCodeSupported(opCode);
    }

    NvAPI_Status NvapiD3d12Device::SetNvShaderExtnSlotSpace(uint32_t uavSlot, uint32_t uavSpace, bool localThread) const {
        if (!m_supportsDeviceExt4)
            return NVAPI_NO_IMPLEMENTATION;

        return static_cast<NvAPI_Status>(m_vkd3dDevice->SetNvShaderExtnSlotSpace(uavSlot, uavSpace, localThread));
    }

    NvAPI_Status NvapiD3d12Device::GetRaytracingMultiIndirectClusterOperationRequirementsInfo(const NVAPI_GET_RAYTRACING_MULTI_INDIRECT_CLUSTER_OPERATION_REQUIREMENTS_INFO_PARAMS* params) const {
        if (!m_supportsDeviceExt4 || !m_supportsClusterAccelerationStructure)
            return NVAPI_NO_IMPLEMENTATION;

        return static_cast<NvAPI_Status>(m_vkd3dDevice->GetRaytracingMultiIndirectClusterOperationRequirementsInfo(params));
    }

    NvAPI_Status NvapiD3d12Device::GetRaytracingPartitionedTlasIndirectPrebuildInfo(const NVAPI_GET_BUILD_RAYTRACING_PARTITIONED_TLAS_INDIRECT_PREBUILD_INFO_PARAMS* params) const {
        if (!m_supportsDeviceExt4 || !m_supportsPartitionedAccelerationStructure)
            return NVAPI_NO_IMPLEMENTATION;

        return static_cast<NvAPI_Status>(m_vkd3dDevice->GetRaytracingPartitionedTlasIndirectPrebuildInfo(params));
    }
}
