#pragma once

#include "../nvapi_private.h"
#include "../util/com_pointer.h"

#include "../vkd3d-proton/vkd3d-proton_interfaces.h"

namespace dxvk {

    class NvapiD3d12Device {

    public:                                          
        static bool CreateCubinComputeShaderWithName(ID3D12Device* pDevice, const void* cubinData, NvU32 cubinSize, NvU32 blockX, NvU32 blockY, NvU32 blockZ, const char* shaderName, NVDX_ObjectHandle* pShader);
        static bool DestroyCubinComputeShader(ID3D12Device* device, NVDX_ObjectHandle pShader);
        static bool GetCudaTextureObject(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE srvHandle, D3D12_CPU_DESCRIPTOR_HANDLE samplerHandle, NvU32* cudaTextureHandle);
        static bool GetCudaSurfaceObject(ID3D12Device* pDevice, D3D12_CPU_DESCRIPTOR_HANDLE uavHandle, NvU32* cudaSurfaceHandle);
        static bool LaunchCubinShader(ID3D12GraphicsCommandList* pCmdList, NVDX_ObjectHandle pShader, NvU32 blockX, NvU32 blockY, NvU32 blockZ, const void* params, NvU32 paramSize);
        static bool CaptureUAVInfo(ID3D12Device* pDevice, NVAPI_UAV_INFO* pUAVInfo);
        static bool IsFatbinPTXSupported(ID3D12Device* pDevice);

    private:
        inline static std::unordered_map<ID3D12Device*, ID3D12DeviceExt*> m_CubinDeviceMap;

        [[nodiscard]] static Com<ID3D12DeviceExt> GetCubinDevice(ID3D12Device* pDevice);
    };
}
