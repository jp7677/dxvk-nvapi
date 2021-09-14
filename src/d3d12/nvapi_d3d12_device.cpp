#include "nvapi_d3d12_device.h"

namespace dxvk {

    bool NvapiD3d12Device::CreateCubinComputeShaderWithName(ID3D12Device* device, const void* cubinData, NvU32 cubinSize, NvU32 blockX, NvU32 blockY, NvU32 blockZ, const char* shaderName, NVDX_ObjectHandle* pShader){
        auto cubinDevice = GetCubinDevice(device);
        if(cubinDevice == nullptr)
            return false;

        return cubinDevice->CreateCubinComputeShaderWithName(cubinData, cubinSize, blockX, blockY, blockZ, shaderName, reinterpret_cast<D3D12_CUBIN_DATA_HANDLE**>(pShader)) >= 0 ;
    }

    bool NvapiD3d12Device::DestroyCubinComputeShader(ID3D12Device* device, NVDX_ObjectHandle shader) {
        auto cubinDevice = GetCubinDevice(device);
        if(cubinDevice == nullptr)
            return false;

        return cubinDevice->DestroyCubinComputeShader(reinterpret_cast<D3D12_CUBIN_DATA_HANDLE*>(shader)) >= 0;
    }

    bool NvapiD3d12Device::GetCudaTextureObject(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE srvHandle, D3D12_CPU_DESCRIPTOR_HANDLE samplerHandle, NvU32* cudaTextureHandle) {
        auto cubinDevice = GetCubinDevice(device);
        if(cubinDevice == nullptr)
            return false;
            
        return cubinDevice->GetCudaTextureObject(srvHandle, samplerHandle, reinterpret_cast<UINT32*>(cudaTextureHandle)) >= 0;

    }

    bool NvapiD3d12Device::GetCudaSurfaceObject(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE uavHandle, NvU32* cudaSurfaceHandle) {
        auto cubinDevice = GetCubinDevice(device);
        if(cubinDevice == nullptr)
            return false;

        return cubinDevice->GetCudaSurfaceObject(uavHandle, reinterpret_cast<UINT32*>(cudaSurfaceHandle)) >= 0;
    }

    bool NvapiD3d12Device::LaunchCubinShader(ID3D12GraphicsCommandList* commandList, NVDX_ObjectHandle pShader, NvU32 blockX, NvU32 blockY, NvU32 blockZ, const void* params, NvU32 paramSize) {
        Com<ID3D12GraphicsCommandListExt> commandListExt;
        if (FAILED(commandList->QueryInterface(IID_PPV_ARGS(&commandListExt))))
            return false;

        return commandListExt->LaunchCubinShader(reinterpret_cast<D3D12_CUBIN_DATA_HANDLE*>(pShader), blockX, blockY, blockZ, params, paramSize) >= 0;
    }

    bool NvapiD3d12Device::CaptureUAVInfo(ID3D12Device* device, NVAPI_UAV_INFO* pUAVInfo) {
        auto cubinDevice = GetCubinDevice(device);
        if(cubinDevice == nullptr)
            return false;

        return cubinDevice->CaptureUAVInfo(reinterpret_cast<D3D12_UAV_INFO*>(pUAVInfo)) >= 0;
    }

    bool NvapiD3d12Device::IsFatbinPTXSupported(ID3D12Device* device) {
        auto cubinDevice = GetCubinDevice(device);
        return cubinDevice != nullptr;
    }

    // We are going to have single map for storing devices with extensions D3D12_VK_NVX_BINARY_IMPORT & D3D12_VK_NVX_IMAGE_VIEW_HANDLE.
    // These are specific to NVIDIA and both of these extensions goes together.
    Com<ID3D12DeviceExt> NvapiD3d12Device::GetCubinDevice(ID3D12Device* device) {
        auto it = m_cubinDeviceMap.find(device);
        Com<ID3D12DeviceExt> cubinDevice = nullptr;
        if(it == m_cubinDeviceMap.end())
        {
            cubinDevice = GetCubinDevice(device, D3D12_VK_NVX_BINARY_IMPORT);
            if(cubinDevice != nullptr)
                 m_cubinDeviceMap.emplace(device, cubinDevice.ptr());
        }
        else
            cubinDevice = it->second;

        return cubinDevice;
    }

    Com<ID3D12DeviceExt> NvapiD3d12Device::GetCubinDevice(ID3D12Device* device, D3D12_VK_EXTENSION extension) {
        Com<ID3D12DeviceExt> cubinDevice;
        if (FAILED(device->QueryInterface(IID_PPV_ARGS(&cubinDevice))))
            return nullptr;

        if (!cubinDevice->GetExtensionSupport(extension))
            return nullptr;

        return cubinDevice;
    }
}
