#pragma once

#include "../nvapi_private.h"
#include "../interfaces/vkd3d-proton_interfaces.h"

namespace dxvk {
    class NvapiD3d12Device {

      public:
        static void Reset();
        [[nodiscard]] static NvapiD3d12Device* GetOrCreate(ID3D12Device* device);
        [[nodiscard]] static uint32_t FindCubinSmem(NVDX_ObjectHandle);

        explicit NvapiD3d12Device(ID3D12DeviceExt* vkd3dDevice);

        [[nodiscard]] HRESULT CreateCubinComputeShaderWithName(const void* cubinData, NvU32 cubinSize, NvU32 blockX, NvU32 blockY, NvU32 blockZ, const char* shaderName, NVDX_ObjectHandle* pShader);
        [[nodiscard]] HRESULT CreateCubinComputeShaderEx(const void* cubinData, NvU32 cubinSize, NvU32 blockX, NvU32 blockY, NvU32 blockZ, NvU32 smemSize, const char* shaderName, NVDX_ObjectHandle* pShader);
        [[nodiscard]] HRESULT DestroyCubinComputeShader(NVDX_ObjectHandle shader);

        [[nodiscard]] HRESULT GetCudaTextureObject(D3D12_CPU_DESCRIPTOR_HANDLE srvHandle, D3D12_CPU_DESCRIPTOR_HANDLE samplerHandle, NvU32* cudaTextureHandle) const;
        [[nodiscard]] HRESULT GetCudaSurfaceObject(D3D12_CPU_DESCRIPTOR_HANDLE uavHandle, NvU32* cudaSurfaceHandle) const;
        [[nodiscard]] HRESULT CaptureUAVInfo(NVAPI_UAV_INFO* uavInfo) const;
        [[nodiscard]] bool IsFatbinPTXSupported() const;

      private:
        static std::unordered_map<ID3D12Device*, NvapiD3d12Device> m_nvapiDeviceMap;
        static std::mutex m_mutex;

        static std::unordered_map<NVDX_ObjectHandle, NvU32> m_cubinSmemMap;
        static std::mutex m_cubinSmemMutex;

        ID3D12DeviceExt2* m_vkd3dDevice{};
        bool m_supportsCubin64bit = false;
        bool m_supportsNvxBinaryImport = false;
        bool m_supportsNvxImageViewHandle = false;
    };
}
