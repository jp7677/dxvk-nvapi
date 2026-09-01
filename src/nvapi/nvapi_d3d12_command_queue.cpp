#include "nvapi_d3d12_command_queue.h"
#include "../util/com_pointer.h"

namespace dxvk {
    std::atomic<LONGLONG> NvapiD3d12CommandQueue::m_resetTimestamp;
    std::mutex NvapiD3d12CommandQueue::m_mutex;

    void NvapiD3d12CommandQueue::Reset() {
        LARGE_INTEGER count;
        QueryPerformanceCounter(&count);
        m_resetTimestamp.store(count.QuadPart, std::memory_order_release);
    }

    std::optional<NvapiD3d12CommandQueue> NvapiD3d12CommandQueue::GetOrCreate(ID3D12CommandQueue* commandQueue) {
        static constexpr GUID NvapiD3d12CommandQueueGuid = {0x3579ec19, 0x2741, 0x488a, {0xbb, 0x39, 0x5c, 0x03, 0xcd, 0x89, 0x5e, 0x2a}};

        NvapiD3d12CommandQueue nvapiCommandQueue{nullptr};
        UINT size = sizeof(nvapiCommandQueue);

        auto result = commandQueue->GetPrivateData(NvapiD3d12CommandQueueGuid, &size, &nvapiCommandQueue);
        if (SUCCEEDED(result) && size == sizeof(nvapiCommandQueue) && nvapiCommandQueue.m_creationTimestamp > m_resetTimestamp.load(std::memory_order_acquire))
            return nvapiCommandQueue;

        size = sizeof(nvapiCommandQueue);
        std::scoped_lock lock{m_mutex};

        result = commandQueue->GetPrivateData(NvapiD3d12CommandQueueGuid, &size, &nvapiCommandQueue);
        if (SUCCEEDED(result) && size == sizeof(nvapiCommandQueue) && nvapiCommandQueue.m_creationTimestamp > m_resetTimestamp.load(std::memory_order_acquire))
            return nvapiCommandQueue;

        Com<ID3D12CommandQueueExt> commandQueueExt;
        if (FAILED(commandQueue->QueryInterface(IID_PPV_ARGS(&commandQueueExt))))
            return std::nullopt;

        LARGE_INTEGER count;
        QueryPerformanceCounter(&count);

        nvapiCommandQueue = NvapiD3d12CommandQueue{commandQueueExt.ptr()};
        nvapiCommandQueue.m_creationTimestamp = count.QuadPart;

        commandQueue->SetPrivateData(NvapiD3d12CommandQueueGuid, sizeof(nvapiCommandQueue), &nvapiCommandQueue);

        return nvapiCommandQueue;
    }

    NvapiD3d12CommandQueue::NvapiD3d12CommandQueue(ID3D12CommandQueueExt* vkd3dCommandQueue)
        : m_vkd3dCommandQueue(vkd3dCommandQueue) {}

    HRESULT NvapiD3d12CommandQueue::NotifyOutOfBandCommandQueue(D3D12_OUT_OF_BAND_CQ_TYPE type) const {
        if (!m_vkd3dCommandQueue)
            return E_NOTIMPL;

        return m_vkd3dCommandQueue->NotifyOutOfBandCommandQueue(type);
    }
}
