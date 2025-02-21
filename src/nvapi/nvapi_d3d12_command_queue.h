#pragma once

#include "../nvapi_private.h"
#include "../interfaces/vkd3d-proton_interfaces.h"

namespace dxvk {
    class NvapiD3d12CommandQueue {

      public:
        static void Reset();
        [[nodiscard]] static NvapiD3d12CommandQueue* GetOrCreate(ID3D12CommandQueue* device);

        explicit NvapiD3d12CommandQueue(ID3D12CommandQueueExt* vkd3dCommandQueue);

        [[nodiscard]] HRESULT NotifyOutOfBandCommandQueue(D3D12_OUT_OF_BAND_CQ_TYPE type) const;

      private:
        static std::unordered_map<ID3D12CommandQueue*, NvapiD3d12CommandQueue> m_nvapiDeviceMap;
        static std::mutex m_mutex;

        ID3D12CommandQueueExt* m_vkd3dCommandQueue{};
    };
}
