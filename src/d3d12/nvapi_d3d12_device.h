#pragma once

#include "../nvapi_private.h"
#include "../vkd3d-proton/vkd3d-proton_interfaces.h"
#include "../util/com_pointer.h"

namespace dxvk {

    class NvapiD3d12Device {

      public:
        static std::optional<LUID> GetLuid(IUnknown* unknown);

        static bool CreateGraphicsPipelineState(ID3D12Device* device, const D3D12_GRAPHICS_PIPELINE_STATE_DESC* pipelineStateDescription, NvU32 numberOfExtensions, const NVAPI_D3D12_PSO_EXTENSION_DESC** extensions, ID3D12PipelineState** pipelineState);
        static bool SetDepthBoundsTestValues(ID3D12GraphicsCommandList* commandList, float minDepth, float maxDepth);

        static bool CreateCubinComputeShaderWithName(ID3D12Device* device, const void* cubinData, NvU32 cubinSize, NvU32 blockX, NvU32 blockY, NvU32 blockZ, const char* shaderName, NVDX_ObjectHandle* pShader);
        static bool DestroyCubinComputeShader(ID3D12Device* device, NVDX_ObjectHandle shader);
        static bool GetCudaTextureObject(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE srvHandle, D3D12_CPU_DESCRIPTOR_HANDLE samplerHandle, NvU32* cudaTextureHandle);
        static bool GetCudaSurfaceObject(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE uavHandle, NvU32* cudaSurfaceHandle);
        static bool LaunchCubinShader(ID3D12GraphicsCommandList* commandList, NVDX_ObjectHandle shader, NvU32 blockX, NvU32 blockY, NvU32 blockZ, const void* params, NvU32 paramSize);
        static bool CaptureUAVInfo(ID3D12Device* device, NVAPI_UAV_INFO* uavInfo);
        static bool IsFatbinPTXSupported(ID3D12Device* device);

        static bool AreOpacityMicromapsSupported(ID3D12Device* device);
        static std::optional<NvAPI_Status> GetRaytracingOpacityMicromapArrayPrebuildInfo(ID3D12Device5* device, NVAPI_GET_RAYTRACING_OPACITY_MICROMAP_ARRAY_PREBUILD_INFO_PARAMS* params);
        static std::optional<NvAPI_Status> SetCreatePipelineStateOptions(ID3D12Device5* device, const NVAPI_D3D12_SET_CREATE_PIPELINE_STATE_OPTIONS_PARAMS* params);
        static std::optional<NvAPI_Status> CheckDriverMatchingIdentifierEx(ID3D12Device5* device, NVAPI_CHECK_DRIVER_MATCHING_IDENTIFIER_EX_PARAMS* params);
        static std::optional<NvAPI_Status> GetRaytracingAccelerationStructurePrebuildInfoEx(ID3D12Device5* device, NVAPI_GET_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO_EX_PARAMS* params);
        static std::optional<NvAPI_Status> BuildRaytracingOpacityMicromapArray(ID3D12GraphicsCommandList4* commandList, NVAPI_BUILD_RAYTRACING_OPACITY_MICROMAP_ARRAY_PARAMS* params);
        static std::optional<NvAPI_Status> RelocateRaytracingOpacityMicromapArray(ID3D12GraphicsCommandList4* commandList, const NVAPI_RELOCATE_RAYTRACING_OPACITY_MICROMAP_ARRAY_PARAMS* params);
        static std::optional<NvAPI_Status> EmitRaytracingOpacityMicromapArrayPostbuildInfo(ID3D12GraphicsCommandList4* commandList, const NVAPI_EMIT_RAYTRACING_OPACITY_MICROMAP_ARRAY_POSTBUILD_INFO_PARAMS* params);
        static std::optional<NvAPI_Status> BuildRaytracingAccelerationStructureEx(ID3D12GraphicsCommandList4* commandList, const NVAPI_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_EX_PARAMS* params);

      private:
        inline static std::unordered_map<ID3D12Device*, ID3D12DeviceExt*> m_cubinDeviceMap;
        inline static std::unordered_map<ID3D12Device*, ID3D12DeviceExt1*> m_ommDeviceMap;
        inline static std::unordered_map<ID3D12GraphicsCommandList*, ID3D12GraphicsCommandListExt*> m_CommandListMap;
        inline static std::unordered_map<ID3D12GraphicsCommandList*, ID3D12GraphicsCommandListExt1*> m_CommandList1Map;

        inline static std::mutex m_CommandListMutex;
        inline static std::mutex m_CommandList1Mutex;
        inline static std::mutex m_CubinDeviceMutex;
        inline static std::mutex m_OmmDeviceMutex;

        [[nodiscard]] static Com<ID3D12DeviceExt> GetCubinDevice(ID3D12Device* device);
        [[nodiscard]] static Com<ID3D12DeviceExt1> GetOmmDevice(ID3D12Device* device);
        template <typename T>
        [[nodiscard]] static Com<T> GetDeviceExt(ID3D12Device* device, D3D12_VK_EXTENSION extension);
        [[nodiscard]] static Com<ID3D12GraphicsCommandListExt> GetCommandListExt(ID3D12GraphicsCommandList* commandList);
        [[nodiscard]] static Com<ID3D12GraphicsCommandListExt1> GetCommandListExt1(ID3D12GraphicsCommandList* commandList);
    };
}
