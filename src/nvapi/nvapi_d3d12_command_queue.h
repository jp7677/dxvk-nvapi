#pragma once

#include "../nvapi_private.h"
#include "../interfaces/vkd3d-proton_interfaces.h"

namespace dxvk {
    class NvapiD3d12CommandQueue final {

      public:
        static void Reset();
        [[nodiscard]] static std::optional<NvapiD3d12CommandQueue> GetOrCreate(ID3D12CommandQueue* commandQueue);

        explicit NvapiD3d12CommandQueue(ID3D12CommandQueueExt* vkd3dCommandQueue);

        [[nodiscard]] HRESULT NotifyOutOfBandCommandQueue(D3D12_OUT_OF_BAND_CQ_TYPE type) const;

      private:
        static std::atomic<LONGLONG> m_resetTimestamp;
        static std::mutex m_mutex;

        LONGLONG m_creationTimestamp{};
        ID3D12CommandQueueExt* m_vkd3dCommandQueue{};
    };
}
