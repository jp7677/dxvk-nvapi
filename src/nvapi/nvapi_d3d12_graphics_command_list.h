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

      private:
        static std::unordered_map<ID3D12GraphicsCommandList*, NvapiD3d12GraphicsCommandList> m_nvapiDeviceMap;
        static std::mutex m_mutex;

        ID3D12GraphicsCommandListExt1* m_vkd3dGraphicsCommandList{};
        bool m_supportsExtGraphicsCommandList1 = false;
    };
}
