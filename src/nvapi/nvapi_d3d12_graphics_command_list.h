#pragma once

#include "../nvapi_private.h"
#include "../interfaces/vkd3d-proton_interfaces.h"
#include "nvapi_as_convert.h"

namespace dxvk {
    class NvapiD3d12GraphicsCommandList final {

      public:
        static void Reset();
        [[nodiscard]] static std::optional<NvapiD3d12GraphicsCommandList> GetOrCreate(ID3D12GraphicsCommandList* commandList);

        explicit NvapiD3d12GraphicsCommandList(ID3D12GraphicsCommandListExt* vkd3dCommandList);

        [[nodiscard]] HRESULT LaunchCubinShader(NVDX_ObjectHandle shader, NvU32 blockX, NvU32 blockY, NvU32 blockZ, const void* params, NvU32 paramSize) const;
        [[nodiscard]] bool IsOpacityMicromapSupported() const { return m_supportsOpacityMicromap; }

        NvapiAsConverter& GetAsConverter() { return m_asConverter; }

        [[nodiscard]] bool VerifyOpacityMicromapArrayNVAPI(D3D12_GPU_VIRTUAL_ADDRESS opacity_micromap_array) const;

        [[nodiscard]] NvAPI_Status RaytracingExecuteMultiIndirectClusterOperation(const NVAPI_RAYTRACING_EXECUTE_MULTI_INDIRECT_CLUSTER_OPERATION_PARAMS* params) const;
        [[nodiscard]] NvAPI_Status BuildRaytracingPartitionedTlasIndirect(const NVAPI_BUILD_RAYTRACING_PARTITIONED_TLAS_INDIRECT_PARAMS* params) const;

      private:
        static std::atomic<LONGLONG> m_resetTimestamp;
        static std::mutex m_mutex;

        LONGLONG m_creationTimestamp{};
        ID3D12GraphicsCommandListExt3* m_vkd3dGraphicsCommandList{};
        bool m_supportsCubinSMem = false;
        bool m_supportsOpacityMicromap = false;
        bool m_supportsClusterAccelerationStructure = false;
        bool m_supportsPartitionedAccelerationStructure = false;

        // Reused across calls so the per-build geometry-desc scratch settles at
        // its max-N and stops hitting the heap. Not thread-safe.
        static thread_local NvapiAsConverter m_asConverter;
    };
}
