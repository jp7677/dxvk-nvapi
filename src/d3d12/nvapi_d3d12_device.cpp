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

        return SUCCEEDED(cubinDevice->CreateCubinComputeShaderWithName(cubinData, cubinSize, blockX, blockY, blockZ, shaderName, reinterpret_cast<D3D12_CUBIN_DATA_HANDLE**>(pShader)));
    }

    bool NvapiD3d12Device::DestroyCubinComputeShader(ID3D12Device* device, NVDX_ObjectHandle shader) {
        auto cubinDevice = GetCubinDevice(device);
        if (cubinDevice == nullptr)
            return false;

        return SUCCEEDED(cubinDevice->DestroyCubinComputeShader(reinterpret_cast<D3D12_CUBIN_DATA_HANDLE*>(shader)));
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
        if (commandListExt == nullptr)
            return false;

        return SUCCEEDED(commandListExt->LaunchCubinShader(reinterpret_cast<D3D12_CUBIN_DATA_HANDLE*>(pShader), blockX, blockY, blockZ, params, paramSize));
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

    bool NvapiD3d12Device::AreOpacityMicromapsSupported(ID3D12Device* device) {
        auto ommDevice = GetOmmDevice(device);
        return ommDevice != nullptr;
    }

    std::optional<NvAPI_Status> NvapiD3d12Device::GetRaytracingOpacityMicromapArrayPrebuildInfo(ID3D12Device5* device, NVAPI_GET_RAYTRACING_OPACITY_MICROMAP_ARRAY_PREBUILD_INFO_PARAMS* params) {
        auto ommDevice = GetOmmDevice(device);
        if (ommDevice == nullptr)
            return std::nullopt;

        return static_cast<NvAPI_Status>(ommDevice->GetRaytracingOpacityMicromapArrayPrebuildInfo(params));
    }

    std::optional<NvAPI_Status> NvapiD3d12Device::SetCreatePipelineStateOptions(ID3D12Device5* device, const NVAPI_D3D12_SET_CREATE_PIPELINE_STATE_OPTIONS_PARAMS* params) {
        auto ommDevice = GetOmmDevice(device);
        if (ommDevice == nullptr)
            return std::nullopt;

        return static_cast<NvAPI_Status>(ommDevice->SetCreatePipelineStateOptions(params));
    }

    std::optional<NvAPI_Status> NvapiD3d12Device::CheckDriverMatchingIdentifierEx(ID3D12Device5* device, NVAPI_CHECK_DRIVER_MATCHING_IDENTIFIER_EX_PARAMS* params) {
        auto ommDevice = GetOmmDevice(device);
        if (ommDevice == nullptr)
            return std::nullopt;

        return static_cast<NvAPI_Status>(ommDevice->CheckDriverMatchingIdentifierEx(params));
    }

    std::optional<NvAPI_Status> NvapiD3d12Device::GetRaytracingAccelerationStructurePrebuildInfoEx(ID3D12Device5* device, NVAPI_GET_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO_EX_PARAMS* params) {
        auto ommDevice = GetOmmDevice(device);
        if (ommDevice == nullptr)
            return std::nullopt;

        return static_cast<NvAPI_Status>(ommDevice->GetRaytracingAccelerationStructurePrebuildInfoEx(params));
    }

    std::optional<NvAPI_Status> NvapiD3d12Device::BuildRaytracingOpacityMicromapArray(ID3D12GraphicsCommandList4* commandList, NVAPI_BUILD_RAYTRACING_OPACITY_MICROMAP_ARRAY_PARAMS* params) {
        auto commandListExt1 = GetCommandListExt1(commandList);
        if (commandListExt1 == nullptr)
            return std::nullopt;

        return static_cast<NvAPI_Status>(commandListExt1->BuildRaytracingOpacityMicromapArray(params));
    }

    std::optional<NvAPI_Status> NvapiD3d12Device::RelocateRaytracingOpacityMicromapArray(ID3D12GraphicsCommandList4* commandList, const NVAPI_RELOCATE_RAYTRACING_OPACITY_MICROMAP_ARRAY_PARAMS* params) {
        auto commandListExt1 = GetCommandListExt1(commandList);
        if (commandListExt1 == nullptr)
            return std::nullopt;

        return static_cast<NvAPI_Status>(commandListExt1->RelocateRaytracingOpacityMicromapArray(params));
    }

    std::optional<NvAPI_Status> NvapiD3d12Device::EmitRaytracingOpacityMicromapArrayPostbuildInfo(ID3D12GraphicsCommandList4* commandList, const NVAPI_EMIT_RAYTRACING_OPACITY_MICROMAP_ARRAY_POSTBUILD_INFO_PARAMS* params) {
        auto commandListExt1 = GetCommandListExt1(commandList);
        if (commandListExt1 == nullptr)
            return std::nullopt;

        return static_cast<NvAPI_Status>(commandListExt1->EmitRaytracingOpacityMicromapArrayPostbuildInfo(params));
    }

    std::optional<NvAPI_Status> NvapiD3d12Device::BuildRaytracingAccelerationStructureEx(ID3D12GraphicsCommandList4* commandList, const NVAPI_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_EX_PARAMS* params) {
        auto commandListExt1 = GetCommandListExt1(commandList);
        if (commandListExt1 == nullptr)
            return std::nullopt;

        return static_cast<NvAPI_Status>(commandListExt1->BuildRaytracingAccelerationStructureEx(params));
    }

    // We are going to have single map for storing devices with extensions D3D12_VK_NVX_BINARY_IMPORT & D3D12_VK_NVX_IMAGE_VIEW_HANDLE.
    // These are specific to NVIDIA and both of these extensions goes together.
    Com<ID3D12DeviceExt> NvapiD3d12Device::GetCubinDevice(ID3D12Device* device) {
        std::scoped_lock lock(m_CubinDeviceMutex);
        auto it = m_cubinDeviceMap.find(device);
        if (it != m_cubinDeviceMap.end())
            return it->second;

        auto cubinDevice = GetDeviceExt<ID3D12DeviceExt>(device, D3D12_VK_NVX_BINARY_IMPORT);
        if (cubinDevice != nullptr)
            m_cubinDeviceMap.emplace(device, cubinDevice.ptr());

        return cubinDevice;
    }

    Com<ID3D12DeviceExt1> NvapiD3d12Device::GetOmmDevice(ID3D12Device* device) {
        std::scoped_lock lock(m_OmmDeviceMutex);
        auto it = m_ommDeviceMap.find(device);
        if (it != m_ommDeviceMap.end())
            return it->second;

        auto ommDevice = GetDeviceExt<ID3D12DeviceExt1>(device, D3D12_VK_EXT_OPACITY_MICROMAP);
        if (ommDevice != nullptr)
            m_ommDeviceMap.emplace(device, ommDevice.ptr());

        return ommDevice;
    }

    template <typename T>
    Com<T> NvapiD3d12Device::GetDeviceExt(ID3D12Device* device, D3D12_VK_EXTENSION extension) {
        Com<T> deviceExt;
        if (FAILED(device->QueryInterface(IID_PPV_ARGS(&deviceExt))))
            return nullptr;

        if (!deviceExt->GetExtensionSupport(extension))
            return nullptr;

        return deviceExt;
    }

    Com<ID3D12GraphicsCommandListExt> NvapiD3d12Device::GetCommandListExt(ID3D12GraphicsCommandList* commandList) {
        std::scoped_lock lock(m_CommandListMutex);
        auto it = m_CommandListMap.find(commandList);
        if (it != m_CommandListMap.end())
            return it->second;

        Com<ID3D12GraphicsCommandListExt> commandListExt;
        if (FAILED(commandList->QueryInterface(IID_PPV_ARGS(&commandListExt))))
            return nullptr;

        m_CommandListMap.emplace(commandList, commandListExt.ptr());
        return commandListExt;
    }

    Com<ID3D12GraphicsCommandListExt1> NvapiD3d12Device::GetCommandListExt1(ID3D12GraphicsCommandList* commandList) {
        std::scoped_lock lock(m_CommandList1Mutex);
        auto it = m_CommandList1Map.find(commandList);
        if (it != m_CommandList1Map.end())
            return it->second;

        Com<ID3D12GraphicsCommandListExt1> commandListExt1;
        if (FAILED(commandList->QueryInterface(IID_PPV_ARGS(&commandListExt1))))
            return nullptr;

        m_CommandList1Map.emplace(commandList, commandListExt1.ptr());
        return commandListExt1;
    }
}
