#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"

#include "../inc/catch.hpp"
#include "../inc/catch2/trompeloeil.hpp"
#include "../src/nvapi_d3d12.cpp"
#include "nvapi_d3d12_mocks.cpp"

using namespace trompeloeil;

TEST_CASE("D3D12 methods succeed", "[.d3d12]") {
    D3D12DeviceMock device;
    D3D12DeviceExtMock deviceExt;
    D3D12GraphicsCommandListExtMock cmdListExt;
    auto deviceRefCount = 0;
    auto cmdListRefCount = 0;

    ALLOW_CALL(deviceExt, QueryInterface(ID3D12DeviceExt::guid, _))
        .LR_SIDE_EFFECT(*_2 = static_cast<ID3D12DeviceExt*>(&deviceExt))
        .LR_SIDE_EFFECT(deviceRefCount++)
        .RETURN(S_OK);
    ALLOW_CALL(deviceExt, AddRef())
        .LR_SIDE_EFFECT(deviceRefCount++)
        .RETURN(deviceRefCount);
    ALLOW_CALL(deviceExt, Release())
        .LR_SIDE_EFFECT(deviceRefCount--)
        .RETURN(deviceRefCount);

    ALLOW_CALL(cmdListExt, QueryInterface(ID3D12GraphicsCommandListExt::guid, _))
        .LR_SIDE_EFFECT(*_2 = static_cast<ID3D12GraphicsCommandListExt*>(&cmdListExt))
        .LR_SIDE_EFFECT(cmdListRefCount++)
        .RETURN(S_OK);
    ALLOW_CALL(cmdListExt, AddRef())
        .LR_SIDE_EFFECT(cmdListRefCount++)
        .RETURN(cmdListRefCount);
    ALLOW_CALL(cmdListExt, Release())
        .LR_SIDE_EFFECT(cmdListRefCount--)
        .RETURN(cmdListRefCount);

    ALLOW_CALL(device, QueryInterface(ID3D12DeviceExt::guid, _))
        .LR_SIDE_EFFECT(*_2 = static_cast<ID3D12DeviceExt*>(&deviceExt))
        .RETURN(E_NOINTERFACE);

    SECTION("D3D12 methods without vkd3d-proton return error") {
        
        ALLOW_CALL(deviceExt, GetExtensionSupport(D3D12_VK_NVX_BINARY_IMPORT))
            .RETURN(true);

        FORBID_CALL(deviceExt, CreateCubinComputeShaderWithName(_, _, _, _, _, _, _));
        FORBID_CALL(deviceExt, DestroyCubinComputeShader(_));
        FORBID_CALL(deviceExt, GetCudaTextureObject(_, _, _));
        FORBID_CALL(deviceExt, GetCudaSurfaceObject(_, _));
        FORBID_CALL(deviceExt, CaptureUAVInfo(_));
        FORBID_CALL(cmdListExt, LaunchCubinShader(_, _, _, _, _, _));

        REQUIRE(NvAPI_D3D12_CreateCubinComputeShaderWithName(static_cast<ID3D12Device*>(&device), nullptr, 0, 0, 0, 0, "shader_name", nullptr) == NVAPI_ERROR);
        REQUIRE(NvAPI_D3D12_CreateCubinComputeShader(static_cast<ID3D12Device*>(&device), nullptr, 0, 0, 0, 0, nullptr) == NVAPI_ERROR);

        REQUIRE(NvAPI_D3D12_DestroyCubinComputeShader(static_cast<ID3D12Device*>(&device), nullptr) == NVAPI_ERROR);
        D3D12_CPU_DESCRIPTOR_HANDLE srvHandle;

        REQUIRE(NvAPI_D3D12_GetCudaTextureObject(static_cast<ID3D12Device*>(&device), srvHandle, srvHandle, nullptr)  == NVAPI_ERROR);
        REQUIRE(NvAPI_D3D12_GetCudaSurfaceObject(static_cast<ID3D12Device*>(&device), srvHandle, nullptr)  == NVAPI_ERROR);

        REQUIRE(NvAPI_D3D12_CaptureUAVInfo(static_cast<ID3D12Device*>(&device), nullptr) == NVAPI_ERROR);
    }

    SECTION("D3D12 methods without cubin extension return error") {
        ALLOW_CALL(deviceExt, GetExtensionSupport(D3D12_VK_NVX_BINARY_IMPORT))
            .RETURN(false);

        REQUIRE(NvAPI_D3D12_CreateCubinComputeShaderWithName(static_cast<ID3D12Device*>(&device), nullptr, 0, 0, 0, 0, "shader_name", nullptr) == NVAPI_ERROR);
        REQUIRE(NvAPI_D3D12_CreateCubinComputeShader(static_cast<ID3D12Device*>(&device), nullptr, 0, 0, 0, 0, nullptr) == NVAPI_ERROR);

        REQUIRE(NvAPI_D3D12_DestroyCubinComputeShader(static_cast<ID3D12Device*>(&device), nullptr) == NVAPI_ERROR);
        D3D12_CPU_DESCRIPTOR_HANDLE srvHandle;

        REQUIRE(NvAPI_D3D12_GetCudaTextureObject(static_cast<ID3D12Device*>(&device), srvHandle, srvHandle, nullptr)  == NVAPI_ERROR);
        REQUIRE(NvAPI_D3D12_GetCudaSurfaceObject(static_cast<ID3D12Device*>(&device), srvHandle, nullptr)  == NVAPI_ERROR);

        REQUIRE(NvAPI_D3D12_CaptureUAVInfo(static_cast<ID3D12Device*>(&device), nullptr) == NVAPI_ERROR);
        
        bool isPTXSupported;
        REQUIRE(NvAPI_D3D12_IsFatbinPTXSupported(static_cast<ID3D12Device*>(&device), &isPTXSupported) == NVAPI_ERROR);
    }

    SECTION("IsNvShaderExtnOpCodeSupported with device returns OK") {
        bool supported = true;
        REQUIRE(NvAPI_D3D12_IsNvShaderExtnOpCodeSupported(&device, 1U, &supported) == NVAPI_OK);
        REQUIRE(supported == false);
    }

    SECTION("NvAPI_D3D12_GetGraphicsCapabilities returns ok") {
        NV_D3D12_GRAPHICS_CAPS graphicsCaps;
        graphicsCaps.bFastUAVClearSupported = false;
        REQUIRE(NvAPI_D3D12_GetGraphicsCapabilities(static_cast<ID3D12Device*>(&device), NV_D3D12_GRAPHICS_CAPS_VER1, &graphicsCaps) == NVAPI_OK);
        REQUIRE(graphicsCaps.bFastUAVClearSupported == true);
    }

    SECTION("D3D12 methods with vkd3d-proton returns OK ") {
        REQUIRE_CALL(device, QueryInterface(ID3D12DeviceExt::guid, _))
            .LR_SIDE_EFFECT(*_2 = static_cast<ID3D12DeviceExt*>(&deviceExt))
            .RETURN(S_OK)
            .TIMES(1);

        ALLOW_CALL(deviceExt, GetExtensionSupport(D3D12_VK_NVX_BINARY_IMPORT))
            .RETURN(true);
        
        const void* cubin_data = nullptr;
        UINT32 cubin_size = 2;
        UINT32 block_x = 3;
        UINT32 block_y = 4;
        UINT32 block_z = 5;
        const char* shader_name = "";
        D3D12_CUBIN_DATA_HANDLE* shader_handle = reinterpret_cast<D3D12_CUBIN_DATA_HANDLE*>(0x912122);
        D3D12_CUBIN_DATA_HANDLE** handle = &shader_handle;

        ALLOW_CALL(deviceExt, CreateCubinComputeShaderWithName(cubin_data, cubin_size, block_x, block_y, block_z, shader_name, handle))
            .RETURN(S_OK);
        REQUIRE(NvAPI_D3D12_CreateCubinComputeShaderWithName(static_cast<ID3D12Device*>(&device), cubin_data, cubin_size, block_x, block_y, block_z, shader_name, reinterpret_cast<NVDX_ObjectHandle*>(handle)) == NVAPI_OK);
        REQUIRE(NvAPI_D3D12_CreateCubinComputeShader(static_cast<ID3D12Device*>(&device), cubin_data, cubin_size, block_x, block_y, block_z, reinterpret_cast<NVDX_ObjectHandle*>(handle)) == NVAPI_OK);

        ALLOW_CALL(deviceExt, DestroyCubinComputeShader(shader_handle))
            .RETURN(S_OK);
        REQUIRE(NvAPI_D3D12_DestroyCubinComputeShader(static_cast<ID3D12Device*>(&device), reinterpret_cast<NVDX_ObjectHandle>(shader_handle)) == NVAPI_OK);

        const D3D12_CPU_DESCRIPTOR_HANDLE srvHandle = {0};
        D3D12_CPU_DESCRIPTOR_HANDLE samplerHandle = {0};
        ALLOW_CALL(deviceExt, GetCudaTextureObject(_, _, nullptr)) // TODO: We should use `srvHandle, samplerHandle` here instead of `_, _`, investigate why this wont compile (no match for ‘operator==’ (operand types are ‘D3D12_CPU_DESCRIPTOR_HANDLE’ and ‘const D3D12_CPU_DESCRIPTOR_HANDLE’))
                .RETURN(S_OK);
        REQUIRE(NvAPI_D3D12_GetCudaTextureObject(static_cast<ID3D12Device*>(&device), srvHandle, samplerHandle, nullptr)  == NVAPI_OK);

        ALLOW_CALL(deviceExt, GetCudaSurfaceObject(_, nullptr)) // TODO: We should use `srvHandle` here instead of `_`, investigate why this wont compile (no match for ‘operator==’ (operand types are ‘D3D12_CPU_DESCRIPTOR_HANDLE’ and ‘const D3D12_CPU_DESCRIPTOR_HANDLE’))
            .RETURN(S_OK);
        REQUIRE(NvAPI_D3D12_GetCudaSurfaceObject(static_cast<ID3D12Device*>(&device), srvHandle, nullptr)  == NVAPI_OK);

        ALLOW_CALL(deviceExt, CaptureUAVInfo(nullptr))
            .RETURN(1);
        REQUIRE(NvAPI_D3D12_CaptureUAVInfo(static_cast<ID3D12Device*>(&device), nullptr) == NVAPI_OK);

        bool isPTXSupported = false;
        REQUIRE(NvAPI_D3D12_IsFatbinPTXSupported(static_cast<ID3D12Device*>(&device), &isPTXSupported) == NVAPI_OK);
        REQUIRE(isPTXSupported == true);
    }

    SECTION("LaunchCubinShader returns error when vkd3d-proton disabled ") {
        D3D12GraphicsCommandListMock cmdList;
        ALLOW_CALL(cmdList, QueryInterface(ID3D12GraphicsCommandListExt::guid, _))
            .LR_SIDE_EFFECT(*_2 = static_cast<ID3D12GraphicsCommandListExt*>(&cmdListExt))
            .RETURN(E_NOINTERFACE);
        
        FORBID_CALL(cmdListExt, LaunchCubinShader(_, _, _, _, _, _));
        REQUIRE(NvAPI_D3D12_LaunchCubinShader(static_cast<ID3D12GraphicsCommandList*>(&cmdList), reinterpret_cast<NVDX_ObjectHandle>(0), 0, 0, 0, nullptr, 0) == NVAPI_ERROR);
    }
    
    SECTION("LaunchCubinShader returns ok when vkd3d-proton enabled ") {
        D3D12GraphicsCommandListMock cmdList;
        ALLOW_CALL(cmdList, QueryInterface(ID3D12GraphicsCommandListExt::guid, _))
            .LR_SIDE_EFFECT(*_2 = static_cast<ID3D12GraphicsCommandListExt*>(&cmdListExt))
            .RETURN(S_OK);

        NVDX_ObjectHandle pShader = nullptr;
        NvU32 blockX = 0;
        NvU32 blockY = 0;
        NvU32 blockZ = 0;
        const void* params = nullptr;
        NvU32 paramSize = 0;

        ALLOW_CALL(cmdListExt, LaunchCubinShader(reinterpret_cast<D3D12_CUBIN_DATA_HANDLE*>(pShader), blockX, blockY, blockZ, params, paramSize))
            .RETURN(S_OK);

        REQUIRE(NvAPI_D3D12_LaunchCubinShader(static_cast<ID3D12GraphicsCommandList*>(&cmdList), pShader, blockX, blockY, blockZ, params, paramSize) == NVAPI_OK);
    }
}
