#pragma once

#include "../nvapi_private.h"
#include "../interfaces/vkd3d-proton_interfaces.h"

namespace dxvk {
    class NvapiD3d12GraphicsCommandList {

      public:
        static void Reset();
        [[nodiscard]] static NvapiD3d12GraphicsCommandList* GetOrCreate(ID3D12GraphicsCommandList* device);

        explicit NvapiD3d12GraphicsCommandList(ID3D12GraphicsCommandListExt* vkd3dCommandList);

        [[nodiscard]] HRESULT LaunchCubinShader(NVDX_ObjectHandle shader, NvU32 blockX, NvU32 blockY, NvU32 blockZ, const void* params, NvU32 paramSize) const;

        [[nodiscard]] NvAPI_Status BuildRaytracingAccelerationStructureEx(const NVAPI_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_EX_PARAMS* params) const;
        [[nodiscard]] NvAPI_Status BuildRaytracingOpacityMicromapArray(NVAPI_BUILD_RAYTRACING_OPACITY_MICROMAP_ARRAY_PARAMS* params) const;
        [[nodiscard]] NvAPI_Status RelocateRaytracingOpacityMicromapArray(const NVAPI_RELOCATE_RAYTRACING_OPACITY_MICROMAP_ARRAY_PARAMS* params) const;
        [[nodiscard]] NvAPI_Status EmitRaytracingOpacityMicromapArrayPostbuildInfo(const NVAPI_EMIT_RAYTRACING_OPACITY_MICROMAP_ARRAY_POSTBUILD_INFO_PARAMS* params) const;

        [[nodiscard]] NvAPI_Status RaytracingExecuteMultiIndirectClusterOperation(const NVAPI_RAYTRACING_EXECUTE_MULTI_INDIRECT_CLUSTER_OPERATION_PARAMS* params) const;
        [[nodiscard]] NvAPI_Status BuildRaytracingPartitionedTlasIndirect(const NVAPI_BUILD_RAYTRACING_PARTITIONED_TLAS_INDIRECT_PARAMS* params) const;

      private:
        static std::unordered_map<ID3D12GraphicsCommandList*, NvapiD3d12GraphicsCommandList> m_nvapiDeviceMap;
        static std::mutex m_mutex;

        ID3D12GraphicsCommandListExt3* m_vkd3dGraphicsCommandList{};
        bool m_supportsExtGraphicsCommandList1 = false;
        bool m_supportsExtGraphicsCommandList2 = false;
        bool m_supportsExtGraphicsCommandList3 = false;
    };
}
