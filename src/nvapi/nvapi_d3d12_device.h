#pragma once

#include "../nvapi_private.h"
#include "../interfaces/vkd3d-proton_interfaces.h"

namespace dxvk {
    class NvapiD3d12Device {

      public:
        static void Reset();
        [[nodiscard]] static NvapiD3d12Device* GetOrCreate(ID3D12Device* device);
        [[nodiscard]] static uint32_t FindCubinSmem(NVDX_ObjectHandle);

        explicit NvapiD3d12Device(ID3D12DeviceExt* vkd3dDevice);

        [[nodiscard]] HRESULT CreateCubinComputeShaderWithName(const void* cubinData, NvU32 cubinSize, NvU32 blockX, NvU32 blockY, NvU32 blockZ, const char* shaderName, NVDX_ObjectHandle* pShader);
        [[nodiscard]] HRESULT CreateCubinComputeShaderEx(const void* cubinData, NvU32 cubinSize, NvU32 blockX, NvU32 blockY, NvU32 blockZ, NvU32 smemSize, const char* shaderName, NVDX_ObjectHandle* pShader);
        [[nodiscard]] HRESULT DestroyCubinComputeShader(NVDX_ObjectHandle shader);

        [[nodiscard]] HRESULT GetCudaTextureObject(D3D12_CPU_DESCRIPTOR_HANDLE srvHandle, D3D12_CPU_DESCRIPTOR_HANDLE samplerHandle, NvU32* cudaTextureHandle) const;
        [[nodiscard]] HRESULT GetCudaSurfaceObject(D3D12_CPU_DESCRIPTOR_HANDLE uavHandle, NvU32* cudaSurfaceHandle) const;
        [[nodiscard]] HRESULT CaptureUAVInfo(NVAPI_UAV_INFO* uavInfo) const;
        [[nodiscard]] bool IsFatbinPTXSupported() const;

        [[nodiscard]] HRESULT CreateCubinComputeShaderExV2(D3D12_CREATE_CUBIN_SHADER_PARAMS* params);
        [[nodiscard]] HRESULT GetCudaMergedTextureSamplerObject(D3D12_GET_CUDA_MERGED_TEXTURE_SAMPLER_OBJECT_PARAMS* params) const;
        [[nodiscard]] HRESULT GetCudaIndependentDescriptorObject(D3D12_GET_CUDA_INDEPENDENT_DESCRIPTOR_OBJECT_PARAMS* params) const;

        [[nodiscard]] bool IsOpacityMicromapSupported() const;
        [[nodiscard]] NvAPI_Status SetCreatePipelineStateOptions(const NVAPI_D3D12_SET_CREATE_PIPELINE_STATE_OPTIONS_PARAMS* params) const;
        [[nodiscard]] NvAPI_Status CheckDriverMatchingIdentifierEx(NVAPI_CHECK_DRIVER_MATCHING_IDENTIFIER_EX_PARAMS* params) const;
        [[nodiscard]] NvAPI_Status GetRaytracingAccelerationStructurePrebuildInfoEx(NVAPI_GET_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO_EX_PARAMS* params) const;
        [[nodiscard]] NvAPI_Status GetRaytracingOpacityMicromapArrayPrebuildInfo(NVAPI_GET_RAYTRACING_OPACITY_MICROMAP_ARRAY_PREBUILD_INFO_PARAMS* params) const;

        [[nodiscard]] bool IsClusterAccelerationStructureSupported() const;
        [[nodiscard]] bool IsPartitionedAccelerationStructureSupported() const;
        [[nodiscard]] bool IsNvShaderExtnOpCodeSupported(uint32_t opCode) const;
        [[nodiscard]] NvAPI_Status SetNvShaderExtnSlotSpace(uint32_t uavSlot, uint32_t uavSpace, bool localThread) const;
        [[nodiscard]] NvAPI_Status GetRaytracingMultiIndirectClusterOperationRequirementsInfo(const NVAPI_GET_RAYTRACING_MULTI_INDIRECT_CLUSTER_OPERATION_REQUIREMENTS_INFO_PARAMS* params) const;
        [[nodiscard]] NvAPI_Status GetRaytracingPartitionedTlasIndirectPrebuildInfo(const NVAPI_GET_BUILD_RAYTRACING_PARTITIONED_TLAS_INDIRECT_PREBUILD_INFO_PARAMS* params) const;

      private:
        static std::unordered_map<ID3D12Device*, NvapiD3d12Device> m_nvapiDeviceMap;
        static std::mutex m_mutex;

        static std::unordered_map<NVDX_ObjectHandle, NvU32> m_cubinSmemMap;
        static std::mutex m_cubinSmemMutex;

        ID3D12DeviceExt4* m_vkd3dDevice{};
        bool m_supportsCubin64bit = false;
        bool m_supportsNvxBinaryImport = false;
        bool m_supportsNvxImageViewHandle = false;
        bool m_supportsDeviceExt3 = false;
        bool m_supportsOpacityMicromap = false;
        bool m_supportsDeviceExt4 = false;
        bool m_supportsClusterAccelerationStructure = false;
        bool m_supportsPartitionedAccelerationStructure = false;
    };
}
