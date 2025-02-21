#include "nvapi_d3d12_graphics_command_list.h"
#include "nvapi_d3d12_device.h"
#include "../util/com_pointer.h"
#include "../util/util_log.h"

namespace dxvk {
    std::unordered_map<ID3D12GraphicsCommandList*, NvapiD3d12GraphicsCommandList> NvapiD3d12GraphicsCommandList::m_nvapiDeviceMap;
    std::mutex NvapiD3d12GraphicsCommandList::m_mutex;

    void NvapiD3d12GraphicsCommandList::Reset() {
        std::scoped_lock lock{m_mutex};
        m_nvapiDeviceMap.clear();
    }

    NvapiD3d12GraphicsCommandList* NvapiD3d12GraphicsCommandList::GetOrCreate(ID3D12GraphicsCommandList* commandList) {
        std::scoped_lock lock{m_mutex};

        auto itF = m_nvapiDeviceMap.find(commandList);
        if (itF != m_nvapiDeviceMap.end())
            return &itF->second;

        Com<ID3D12GraphicsCommandListExt> commandListExt;
        if (FAILED(commandList->QueryInterface(IID_PPV_ARGS(&commandListExt))))
            return nullptr;

        auto [itI, inserted] = m_nvapiDeviceMap.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(commandList),
            std::forward_as_tuple(commandListExt.ptr()));

        if (!inserted)
            return nullptr;

        return &itI->second;
    }

    NvapiD3d12GraphicsCommandList::NvapiD3d12GraphicsCommandList(ID3D12GraphicsCommandListExt* vkd3dCommandList)
        : m_vkd3dGraphicsCommandList(static_cast<ID3D12GraphicsCommandListExt1*>(vkd3dCommandList)) { // NOLINT(*-pro-type-static-cast-downcast)
        Com<ID3D12GraphicsCommandListExt1> commandListExt1;
        m_supportsExtGraphicsCommandList1 = SUCCEEDED(vkd3dCommandList->QueryInterface(IID_PPV_ARGS(&commandListExt1)));
    }

    HRESULT NvapiD3d12GraphicsCommandList::LaunchCubinShader(NVDX_ObjectHandle pShader, NvU32 blockX, NvU32 blockY, NvU32 blockZ, const void* params, NvU32 paramSize) const {
        if (!m_vkd3dGraphicsCommandList)
            return E_NOTIMPL;

        auto smem = NvapiD3d12Device::FindCubinSmem(pShader);

        if (m_supportsExtGraphicsCommandList1)
            return m_vkd3dGraphicsCommandList->LaunchCubinShaderEx(reinterpret_cast<D3D12_CUBIN_DATA_HANDLE*>(pShader), blockX, blockY, blockZ, smem, params, paramSize, nullptr, 0);

        if (smem != 0)
            log::info("Non-zero SMEM value supplied for CuBIN but ID3D12GraphicsCommandListExt1 not supported! This may cause corruption");

        return m_vkd3dGraphicsCommandList->LaunchCubinShader(reinterpret_cast<D3D12_CUBIN_DATA_HANDLE*>(pShader), blockX, blockY, blockZ, params, paramSize);
    }
}
