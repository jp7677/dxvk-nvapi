#include "nvapi_d3d12_graphics_command_list.h"
#include "nvapi_d3d12_device.h"
#include "../util/com_pointer.h"
#include "../util/util_log.h"

namespace dxvk {
    thread_local NvapiAsConverter NvapiD3d12GraphicsCommandList::m_asConverter;
    std::atomic<LONGLONG> NvapiD3d12GraphicsCommandList::m_resetTimestamp;
    std::mutex NvapiD3d12GraphicsCommandList::m_mutex;

    void NvapiD3d12GraphicsCommandList::Reset() {
        LARGE_INTEGER count;
        QueryPerformanceCounter(&count);
        m_resetTimestamp.store(count.QuadPart, std::memory_order_release);
    }

    std::optional<NvapiD3d12GraphicsCommandList> NvapiD3d12GraphicsCommandList::GetOrCreate(ID3D12GraphicsCommandList* commandList) {
        static constexpr GUID NvapiD3d12GraphicsCommandListGuid = {0x1d409261, 0xd613, 0x4ef4, {0x86, 0x10, 0x25, 0x0e, 0x8b, 0x3b, 0x56, 0x56}};

        NvapiD3d12GraphicsCommandList nvapiGraphicsCommandList{nullptr};
        UINT size = sizeof(nvapiGraphicsCommandList);

        auto result = commandList->GetPrivateData(NvapiD3d12GraphicsCommandListGuid, &size, &nvapiGraphicsCommandList);
        if (SUCCEEDED(result) && size == sizeof(nvapiGraphicsCommandList) && nvapiGraphicsCommandList.m_creationTimestamp > m_resetTimestamp.load(std::memory_order_acquire))
            return nvapiGraphicsCommandList;

        size = sizeof(nvapiGraphicsCommandList);
        std::scoped_lock lock{m_mutex};

        result = commandList->GetPrivateData(NvapiD3d12GraphicsCommandListGuid, &size, &nvapiGraphicsCommandList);
        if (SUCCEEDED(result) && size == sizeof(nvapiGraphicsCommandList) && nvapiGraphicsCommandList.m_creationTimestamp > m_resetTimestamp.load(std::memory_order_acquire))
            return nvapiGraphicsCommandList;

        Com<ID3D12GraphicsCommandListExt> commandListExt;
        if (FAILED(commandList->QueryInterface(IID_PPV_ARGS(&commandListExt))))
            return std::nullopt;

        LARGE_INTEGER count;
        QueryPerformanceCounter(&count);

        nvapiGraphicsCommandList = NvapiD3d12GraphicsCommandList{commandListExt.ptr()};
        nvapiGraphicsCommandList.m_creationTimestamp = count.QuadPart;

        commandList->SetPrivateData(NvapiD3d12GraphicsCommandListGuid, sizeof(nvapiGraphicsCommandList), &nvapiGraphicsCommandList);

        return nvapiGraphicsCommandList;
    }

    NvapiD3d12GraphicsCommandList::NvapiD3d12GraphicsCommandList(ID3D12GraphicsCommandListExt* vkd3dCommandList)
        : m_vkd3dGraphicsCommandList(static_cast<ID3D12GraphicsCommandListExt3*>(vkd3dCommandList)) { // NOLINT(*-pro-type-static-cast-downcast)
        if (!vkd3dCommandList)
            return;

        uint32_t commandListTier = probeInterfaceChain(vkd3dCommandList, {
                                                                             __uuidof(ID3D12GraphicsCommandListExt1),
                                                                             __uuidof(ID3D12GraphicsCommandListExt2),
                                                                             __uuidof(ID3D12GraphicsCommandListExt3),
                                                                         });
        m_supportsCubinSMem = commandListTier >= 1;

        Com<ID3D12GraphicsCommandList> d3d12CommandList;
        if (SUCCEEDED(vkd3dCommandList->QueryInterface(IID_PPV_ARGS(&d3d12CommandList)))) {
            Com<ID3D12Device> device;
            if (SUCCEEDED(d3d12CommandList->GetDevice(IID_PPV_ARGS(&device)))) {
                if (auto nvapiDevice = NvapiD3d12Device::GetOrCreate(device.ptr())) {
                    m_supportsOpacityMicromap = nvapiDevice->IsOpacityMicromapSupported();

                    if (commandListTier >= 3) {
                        m_supportsClusterAccelerationStructure = nvapiDevice->IsClusterAccelerationStructureSupported();
                        m_supportsPartitionedAccelerationStructure = nvapiDevice->IsPartitionedAccelerationStructureSupported();
                    }
                }
            }
        }
    }

    HRESULT NvapiD3d12GraphicsCommandList::LaunchCubinShader(NVDX_ObjectHandle pShader, NvU32 blockX, NvU32 blockY, NvU32 blockZ, const void* params, NvU32 paramSize) const {
        if (!m_vkd3dGraphicsCommandList)
            return E_NOTIMPL;

        auto smem = NvapiD3d12Device::FindCubinSmem(pShader);

        if (m_supportsCubinSMem)
            return m_vkd3dGraphicsCommandList->LaunchCubinShaderEx(reinterpret_cast<D3D12_CUBIN_DATA_HANDLE*>(pShader), blockX, blockY, blockZ, smem, params, paramSize, nullptr, 0);

        if (smem != 0)
            log::info("Non-zero SMEM value supplied for CuBIN but ID3D12GraphicsCommandListExt1 not supported! This may cause corruption");

        return m_vkd3dGraphicsCommandList->LaunchCubinShader(reinterpret_cast<D3D12_CUBIN_DATA_HANDLE*>(pShader), blockX, blockY, blockZ, params, paramSize);
    }

    bool NvapiD3d12GraphicsCommandList::VerifyOpacityMicromapArrayNVAPI(D3D12_GPU_VIRTUAL_ADDRESS opacity_micromap_array) const {
        return m_supportsOpacityMicromap && m_vkd3dGraphicsCommandList->VerifyOpacityMicromapArrayNVAPI(opacity_micromap_array);
    }

    NvAPI_Status NvapiD3d12GraphicsCommandList::RaytracingExecuteMultiIndirectClusterOperation(const NVAPI_RAYTRACING_EXECUTE_MULTI_INDIRECT_CLUSTER_OPERATION_PARAMS* params) const {
        if (!m_supportsClusterAccelerationStructure)
            return NVAPI_NO_IMPLEMENTATION;

        return static_cast<NvAPI_Status>(m_vkd3dGraphicsCommandList->RaytracingExecuteMultiIndirectClusterOperation(params));
    }

    NvAPI_Status NvapiD3d12GraphicsCommandList::BuildRaytracingPartitionedTlasIndirect(const NVAPI_BUILD_RAYTRACING_PARTITIONED_TLAS_INDIRECT_PARAMS* params) const {
        if (!m_supportsPartitionedAccelerationStructure)
            return NVAPI_NO_IMPLEMENTATION;

        return static_cast<NvAPI_Status>(m_vkd3dGraphicsCommandList->BuildRaytracingPartitionedTlasIndirect(params));
    }
}
