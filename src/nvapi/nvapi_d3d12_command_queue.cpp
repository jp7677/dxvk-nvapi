#include "nvapi_d3d12_command_queue.h"
#include "../util/com_pointer.h"

namespace dxvk {
    std::unordered_map<ID3D12CommandQueue*, NvapiD3d12CommandQueue> NvapiD3d12CommandQueue::m_nvapiDeviceMap;
    std::mutex NvapiD3d12CommandQueue::m_mutex;

    void NvapiD3d12CommandQueue::Reset() {
        std::scoped_lock lock{m_mutex};
        m_nvapiDeviceMap.clear();
    }

    NvapiD3d12CommandQueue* NvapiD3d12CommandQueue::GetOrCreate(ID3D12CommandQueue* commandQueue) {
        std::scoped_lock lock{m_mutex};

        auto itF = m_nvapiDeviceMap.find(commandQueue);
        if (itF != m_nvapiDeviceMap.end())
            return &itF->second;

        Com<ID3D12CommandQueueExt> commandQueueExt;
        if (FAILED(commandQueue->QueryInterface(IID_PPV_ARGS(&commandQueueExt))))
            return nullptr;

        auto [itI, inserted] = m_nvapiDeviceMap.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(commandQueue),
            std::forward_as_tuple(commandQueueExt.ptr()));

        if (!inserted)
            return nullptr;

        return &itI->second;
    }

    NvapiD3d12CommandQueue::NvapiD3d12CommandQueue(ID3D12CommandQueueExt* vkd3dCommandQueue)
        : m_vkd3dCommandQueue(vkd3dCommandQueue) {}

    HRESULT NvapiD3d12CommandQueue::NotifyOutOfBandCommandQueue(D3D12_OUT_OF_BAND_CQ_TYPE type) const {
        if (!m_vkd3dCommandQueue)
            return E_NOTIMPL;

        return m_vkd3dCommandQueue->NotifyOutOfBandCommandQueue(type);
    }
}
