#include "../util/util_log.h"
#include "nvapi_d3d12_device.h"

namespace dxvk {

    std::optional<LUID> NvapiD3d12Device::GetLuid(IUnknown* unknown) {
        Com<ID3D12Device> device;
        if (FAILED(unknown->QueryInterface(IID_PPV_ARGS(&device))))
            return {};

        return device->GetAdapterLuid();
    }

    bool NvapiD3d12Device::CreateGraphicsPipelineState(ID3D12Device* device, const D3D12_GRAPHICS_PIPELINE_STATE_DESC* pipelineStateDescription, NvU32 numberOfExtensions, const NVAPI_D3D12_PSO_EXTENSION_DESC** extensions, ID3D12PipelineState** pipelineState) {
        if (numberOfExtensions != 1 || extensions[0]->psoExtension != NV_PSO_ENABLE_DEPTH_BOUND_TEST_EXTENSION)
            return false;

        return SUCCEEDED(device->CreateGraphicsPipelineState(pipelineStateDescription, __uuidof(ID3D12PipelineState), reinterpret_cast<void**>(pipelineState)));
    }

    bool NvapiD3d12Device::SetDepthBoundsTestValues(ID3D12GraphicsCommandList* commandList, const float minDepth, const float maxDepth) {
        Com<ID3D12GraphicsCommandList1> commandList1;
        if (FAILED(commandList->QueryInterface(IID_PPV_ARGS(&commandList1)))) // There is no VKD3D-Proton version out there that does not implement ID3D12GraphicsCommandList1, this should always succeed
            return false;

        commandList1->OMSetDepthBounds(minDepth, maxDepth);
        return true;
    }

    bool NvapiD3d12Device::CreateCubinComputeShaderWithName(ID3D12Device* device, const void* cubinData, NvU32 cubinSize, NvU32 blockX, NvU32 blockY, NvU32 blockZ, const char* shaderName, NVDX_ObjectHandle* pShader) {
        auto cubinDevice = GetCubinDevice(device);
        if (cubinDevice == nullptr)
            return false;

        return CreateCubinComputeShaderEx(device, cubinData, cubinSize, blockX, blockY, blockZ, 0 /* smemSize */, shaderName, pShader);
    }

    bool NvapiD3d12Device::CreateCubinComputeShaderEx(ID3D12Device* device, const void* cubinData, NvU32 cubinSize, NvU32 blockX, NvU32 blockY, NvU32 blockZ, NvU32 smemSize, const char* shaderName, NVDX_ObjectHandle* pShader) {
        auto cubinDevice = GetCubinDevice(device);
        if (cubinDevice == nullptr)
            return false;

        Com<ID3D12DeviceExt> deviceExt;
        if (FAILED(cubinDevice->QueryInterface(IID_PPV_ARGS(&deviceExt))))
            return false;

        if (FAILED(deviceExt->CreateCubinComputeShaderWithName(cubinData, cubinSize, blockX, blockY, blockZ, shaderName, reinterpret_cast<D3D12_CUBIN_DATA_HANDLE**>(pShader))))
            return false;

        std::scoped_lock lock(m_CubinSmemMutex);
        m_cubinSmemMap.emplace(*pShader, smemSize);
        return true;
    }

    bool NvapiD3d12Device::DestroyCubinComputeShader(ID3D12Device* device, NVDX_ObjectHandle shader) {
        auto cubinDevice = GetCubinDevice(device);
        if (cubinDevice == nullptr)
            return false;

        if (FAILED(cubinDevice->DestroyCubinComputeShader(reinterpret_cast<D3D12_CUBIN_DATA_HANDLE*>(shader))))
            return false;

        std::scoped_lock lock(m_CubinSmemMutex);
        m_cubinSmemMap.erase(shader);
        return true;
    }

    bool NvapiD3d12Device::GetCudaTextureObject(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE srvHandle, D3D12_CPU_DESCRIPTOR_HANDLE samplerHandle, NvU32* cudaTextureHandle) {
        auto cubinDevice = GetCubinDevice(device);
        if (cubinDevice == nullptr)
            return false;

        return SUCCEEDED(cubinDevice->GetCudaTextureObject(srvHandle, samplerHandle, reinterpret_cast<UINT32*>(cudaTextureHandle)));
    }

    bool NvapiD3d12Device::GetCudaSurfaceObject(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE uavHandle, NvU32* cudaSurfaceHandle) {
        auto cubinDevice = GetCubinDevice(device);
        if (cubinDevice == nullptr)
            return false;

        return SUCCEEDED(cubinDevice->GetCudaSurfaceObject(uavHandle, reinterpret_cast<UINT32*>(cudaSurfaceHandle)));
    }

    bool NvapiD3d12Device::LaunchCubinShader(ID3D12GraphicsCommandList* commandList, NVDX_ObjectHandle pShader, NvU32 blockX, NvU32 blockY, NvU32 blockZ, const void* params, NvU32 paramSize) {
        auto commandListExt = GetCommandListExt(commandList);
        if (!commandListExt.has_value())
            return false;

        auto cmdList = commandListExt.value().CommandListExt;
        auto interfaceVersion = commandListExt.value().InterfaceVersion;

        uint32_t smem = 0;
        std::scoped_lock lock(m_CubinSmemMutex);
        auto it = m_cubinSmemMap.find(pShader);
        if (it != m_cubinSmemMap.end())
            smem = it->second;
        else
            log::write("Failed to find CuBIN in m_cubinSmemMap, defaulting to 0");

        if (interfaceVersion >= 1)
            return SUCCEEDED(cmdList->LaunchCubinShaderEx(reinterpret_cast<D3D12_CUBIN_DATA_HANDLE*>(pShader), blockX, blockY, blockZ, smem, params, paramSize, nullptr, 0));
        else {
            if (smem != 0)
                log::write("Non-zero SMEM value supplied for CuBIN but ID3D12GraphicsCommandListExt1 not supported! This may cause corruption");

            return SUCCEEDED(cmdList->LaunchCubinShader(reinterpret_cast<D3D12_CUBIN_DATA_HANDLE*>(pShader), blockX, blockY, blockZ, params, paramSize));
        }
    }

    bool NvapiD3d12Device::CaptureUAVInfo(ID3D12Device* device, NVAPI_UAV_INFO* pUAVInfo) {
        auto cubinDevice = GetCubinDevice(device);
        if (cubinDevice == nullptr)
            return false;

        return SUCCEEDED(cubinDevice->CaptureUAVInfo(reinterpret_cast<D3D12_UAV_INFO*>(pUAVInfo)));
    }

    bool NvapiD3d12Device::IsFatbinPTXSupported(ID3D12Device* device) {
        auto cubinDevice = GetCubinDevice(device);
        return cubinDevice != nullptr;
    }

    // We are going to have single map for storing devices with extensions D3D12_VK_NVX_BINARY_IMPORT & D3D12_VK_NVX_IMAGE_VIEW_HANDLE.
    // These are specific to NVIDIA and both of these extensions goes together.
    Com<ID3D12DeviceExt> NvapiD3d12Device::GetCubinDevice(ID3D12Device* device) {
        std::scoped_lock lock(m_CubinDeviceMutex);
        auto it = m_cubinDeviceMap.find(device);
        if (it != m_cubinDeviceMap.end())
            return it->second;

        auto cubinDevice = GetDeviceExt(device, D3D12_VK_NVX_BINARY_IMPORT);
        if (cubinDevice != nullptr)
            m_cubinDeviceMap.emplace(device, cubinDevice.ptr());

        return cubinDevice;
    }

    Com<ID3D12DeviceExt> NvapiD3d12Device::GetDeviceExt(ID3D12Device* device, D3D12_VK_EXTENSION extension) {
        Com<ID3D12DeviceExt> deviceExt;
        if (FAILED(device->QueryInterface(IID_PPV_ARGS(&deviceExt))))
            return nullptr;

        if (!deviceExt->GetExtensionSupport(extension))
            return nullptr;

        return deviceExt;
    }

    std::optional<NvapiD3d12Device::CommandListExtWithVersion> NvapiD3d12Device::GetCommandListExt(ID3D12GraphicsCommandList* commandList) {
        std::scoped_lock lock(m_CommandListMutex);
        auto it = m_CommandListMap.find(commandList);
        if (it != m_CommandListMap.end())
            return it->second;

        Com<ID3D12GraphicsCommandListExt1> commandListExt1 = nullptr;
        if (SUCCEEDED(commandList->QueryInterface(IID_PPV_ARGS(&commandListExt1)))) {
            NvapiD3d12Device::CommandListExtWithVersion cmdListVer{(ID3D12GraphicsCommandListExt1*)commandListExt1.ptr(), 1};
            return std::make_optional(m_CommandListMap.emplace(commandList, cmdListVer).first->second);
        }

        Com<ID3D12GraphicsCommandListExt> commandListExt = nullptr;
        if (SUCCEEDED(commandList->QueryInterface(IID_PPV_ARGS(&commandListExt)))) {
            NvapiD3d12Device::CommandListExtWithVersion cmdListVer{(ID3D12GraphicsCommandListExt1*)commandListExt.ptr(), 0};
            return std::make_optional(m_CommandListMap.emplace(commandList, cmdListVer).first->second);
        }

        log::write("commandList has no ID3D12GraphicsCommandListExt compatible interface!");
        return {};
    }
}
