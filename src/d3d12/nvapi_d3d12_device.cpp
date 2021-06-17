#include "nvapi_d3d12_device.h"

namespace dxvk {
    Com<ID3D12DeviceExt> GetDevice(ID3D12Device* pDevice, D3D12_VK_EXTENSION extension) {
        Com<ID3D12DeviceExt> d3d12Device;
        if (FAILED(pDevice->QueryInterface(IID_PPV_ARGS(&d3d12Device))))
            return nullptr;

        if (!d3d12Device->GetExtensionSupport(extension))
            return nullptr;

        return d3d12Device;
    }
    

    // We are going to have single map for storing devices with extensions D3D12_VK_NVX_BINARY_IMPORT & D3D12_VK_NVX_IMAGE_VIEW_HANDLE.
    // These are specific to NVIDIA and both of these extensions goes together.
    Com<ID3D12DeviceExt> NvapiD3d12Device::GetCubinDevice(ID3D12Device* pDevice) {
        auto it = m_CubinDeviceMap.find(pDevice);
        Com<ID3D12DeviceExt> d3d12Device = nullptr;
        if(it == m_CubinDeviceMap.end())
        {
            d3d12Device = GetDevice(pDevice, D3D12_VK_NVX_BINARY_IMPORT);
            if(d3d12Device != nullptr){
                 m_CubinDeviceMap.emplace(pDevice, d3d12Device.ptr());
            }
               
        }
        else {
            d3d12Device = it->second;
        }

        return d3d12Device;
    }

    bool NvapiD3d12Device::CreateCubinComputeShaderWithName(ID3D12Device* pDevice, const void* cubinData, NvU32 cubinSize, NvU32 blockX, NvU32 blockY, NvU32 blockZ, const char* shaderName, NVDX_ObjectHandle* pShader){
        auto d3d12Device = GetCubinDevice(pDevice);
        if(d3d12Device == nullptr)
            return false;

        return d3d12Device->CreateCubinComputeShaderWithName(cubinData, cubinSize, blockX, blockY, blockZ, shaderName, reinterpret_cast<D3D12_CUBIN_DATA_HANDLE**>(pShader)) >= 0 ;
    }

    bool NvapiD3d12Device::DestroyCubinComputeShader(ID3D12Device* pDevice, NVDX_ObjectHandle pShader) {
        auto d3d12Device = GetCubinDevice(pDevice);
        if(d3d12Device == nullptr)
            return false;

        return d3d12Device->DestroyCubinComputeShader(reinterpret_cast<D3D12_CUBIN_DATA_HANDLE*>(pShader)) >= 0;
    }

    bool NvapiD3d12Device::GetCudaTextureObject(ID3D12Device* pDevice, D3D12_CPU_DESCRIPTOR_HANDLE srvHandle, D3D12_CPU_DESCRIPTOR_HANDLE samplerHandle, NvU32* cudaTextureHandle) {
        auto d3d12Device = GetCubinDevice(pDevice);
        if(d3d12Device == nullptr)
            return false;
            
        return d3d12Device->GetCudaTextureObject(srvHandle, samplerHandle, reinterpret_cast<UINT32*>(cudaTextureHandle)) >= 0;

    }

    bool NvapiD3d12Device::GetCudaSurfaceObject(ID3D12Device* pDevice, D3D12_CPU_DESCRIPTOR_HANDLE uavHandle, NvU32* cudaSurfaceHandle) {
        auto d3d12Device = GetCubinDevice(pDevice);
        if(d3d12Device == nullptr)
            return false;

        return d3d12Device->GetCudaSurfaceObject(uavHandle, reinterpret_cast<UINT32*>(cudaSurfaceHandle)) >= 0;
    }

    bool NvapiD3d12Device::LaunchCubinShader(ID3D12GraphicsCommandList* pCmdList, NVDX_ObjectHandle pShader, NvU32 blockX, NvU32 blockY, NvU32 blockZ, const void* params, NvU32 paramSize) {
        Com<ID3D12GraphicsCommandListExt> d3d12GraphicsCmdList;
        if (FAILED(pCmdList->QueryInterface(IID_PPV_ARGS(&d3d12GraphicsCmdList))))
            return false;

        return d3d12GraphicsCmdList->LaunchCubinShader(reinterpret_cast<D3D12_CUBIN_DATA_HANDLE*>(pShader), blockX, blockY, blockZ, params, paramSize) >= 0;
    }

    bool NvapiD3d12Device::CaptureUAVInfo(ID3D12Device* pDevice, NVAPI_UAV_INFO* pUAVInfo) {
        auto d3d12Device = GetCubinDevice(pDevice);
        if(d3d12Device == nullptr)
            return false;

        return d3d12Device->CaptureUAVInfo(reinterpret_cast<D3D12_UAV_INFO*>(pUAVInfo)) >= 0;
    }

    bool NvapiD3d12Device::IsFatbinPTXSupported(ID3D12Device* pDevice) {
        auto d3d12Device = GetCubinDevice(pDevice);
        return d3d12Device != nullptr;
    }

}
