#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"

#include "../inc/catch.hpp"
#include "../inc/catch2/trompeloeil.hpp"
#include "../src/nvapi_d3d12.cpp"
#include "nvapi_d3d12_mocks.cpp"

using namespace trompeloeil;

TEST_CASE("D3D12 methods succeed", "[.d3d12]") {
    D3D12Vkd3dDeviceMock device;
    D3D12Vkd3dGraphicsCommandListMock commandList;
    auto deviceRefCount = 0;
    auto commandListRefCount = 0;

    ALLOW_CALL(device, QueryInterface(ID3D12DeviceExt::guid, _))
        .LR_SIDE_EFFECT(*_2 = static_cast<ID3D12DeviceExt*>(&device))
        .LR_SIDE_EFFECT(deviceRefCount++)
        .RETURN(S_OK);
    ALLOW_CALL(device, AddRef())
        .LR_SIDE_EFFECT(deviceRefCount++)
        .RETURN(deviceRefCount);
    ALLOW_CALL(device, Release())
        .LR_SIDE_EFFECT(deviceRefCount--)
        .RETURN(deviceRefCount);

    ALLOW_CALL(device, GetExtensionSupport(_))
            .RETURN(true);

    ALLOW_CALL(commandList, QueryInterface(ID3D12GraphicsCommandListExt::guid, _))
        .LR_SIDE_EFFECT(*_2 = static_cast<ID3D12GraphicsCommandListExt*>(&commandList))
        .LR_SIDE_EFFECT(commandListRefCount++)
        .RETURN(S_OK);
    ALLOW_CALL(commandList, AddRef())
        .LR_SIDE_EFFECT(commandListRefCount++)
        .RETURN(commandListRefCount);
    ALLOW_CALL(commandList, Release())
        .LR_SIDE_EFFECT(commandListRefCount--)
        .RETURN(commandListRefCount);

    SECTION("D3D12 methods without vkd3d-proton return error") {
        ALLOW_CALL(device, QueryInterface(ID3D12DeviceExt::guid, _))
            .RETURN(E_NOINTERFACE);
        ALLOW_CALL(commandList, QueryInterface(ID3D12GraphicsCommandListExt::guid, _))
            .RETURN(E_NOINTERFACE);

        FORBID_CALL(device, CreateCubinComputeShaderWithName(_, _, _, _, _, _, _));
        FORBID_CALL(device, DestroyCubinComputeShader(_));
        FORBID_CALL(device, GetCudaTextureObject(_, _, _));
        FORBID_CALL(device, GetCudaSurfaceObject(_, _));
        FORBID_CALL(device, CaptureUAVInfo(_));
        FORBID_CALL(commandList, LaunchCubinShader(_, _, _, _, _, _));

        REQUIRE(NvAPI_D3D12_CreateCubinComputeShaderWithName(static_cast<ID3D12Device*>(&device), nullptr, 0, 0, 0, 0, "shader_name", nullptr) == NVAPI_ERROR);
        REQUIRE(NvAPI_D3D12_CreateCubinComputeShader(static_cast<ID3D12Device*>(&device), nullptr, 0, 0, 0, 0, nullptr) == NVAPI_ERROR);
        REQUIRE(NvAPI_D3D12_DestroyCubinComputeShader(static_cast<ID3D12Device*>(&device), nullptr) == NVAPI_ERROR);

        D3D12_CPU_DESCRIPTOR_HANDLE srvHandle;
        REQUIRE(NvAPI_D3D12_GetCudaTextureObject(static_cast<ID3D12Device*>(&device), srvHandle, srvHandle, nullptr)  == NVAPI_ERROR);
        REQUIRE(NvAPI_D3D12_GetCudaSurfaceObject(static_cast<ID3D12Device*>(&device), srvHandle, nullptr)  == NVAPI_ERROR);

        REQUIRE(NvAPI_D3D12_CaptureUAVInfo(static_cast<ID3D12Device*>(&device), nullptr) == NVAPI_ERROR);
        bool isPTXSupported;
        REQUIRE(NvAPI_D3D12_IsFatbinPTXSupported(static_cast<ID3D12Device*>(&device), &isPTXSupported) == NVAPI_ERROR);

        REQUIRE(NvAPI_D3D12_LaunchCubinShader(static_cast<ID3D12GraphicsCommandList*>(&commandList), reinterpret_cast<NVDX_ObjectHandle>(0), 0, 0, 0, nullptr, 0) == NVAPI_ERROR);
        REQUIRE(deviceRefCount == 0);
        REQUIRE(commandListRefCount == 0);
    }

    SECTION("D3D12 methods without cubin extension return error") {
        ALLOW_CALL(device, GetExtensionSupport(D3D12_VK_NVX_BINARY_IMPORT))
            .RETURN(false);

        FORBID_CALL(device, CreateCubinComputeShaderWithName(_, _, _, _, _, _, _));
        FORBID_CALL(device, DestroyCubinComputeShader(_));
        FORBID_CALL(device, GetCudaTextureObject(_, _, _));
        FORBID_CALL(device, GetCudaSurfaceObject(_, _));
        FORBID_CALL(device, CaptureUAVInfo(_));

        REQUIRE(NvAPI_D3D12_CreateCubinComputeShaderWithName(static_cast<ID3D12Device*>(&device), nullptr, 0, 0, 0, 0, "shader_name", nullptr) == NVAPI_ERROR);
        REQUIRE(NvAPI_D3D12_CreateCubinComputeShader(static_cast<ID3D12Device*>(&device), nullptr, 0, 0, 0, 0, nullptr) == NVAPI_ERROR);
        REQUIRE(NvAPI_D3D12_DestroyCubinComputeShader(static_cast<ID3D12Device*>(&device), nullptr) == NVAPI_ERROR);

        D3D12_CPU_DESCRIPTOR_HANDLE srvHandle;
        REQUIRE(NvAPI_D3D12_GetCudaTextureObject(static_cast<ID3D12Device*>(&device), srvHandle, srvHandle, nullptr)  == NVAPI_ERROR);
        REQUIRE(NvAPI_D3D12_GetCudaSurfaceObject(static_cast<ID3D12Device*>(&device), srvHandle, nullptr)  == NVAPI_ERROR);

        REQUIRE(NvAPI_D3D12_CaptureUAVInfo(static_cast<ID3D12Device*>(&device), nullptr) == NVAPI_ERROR);
        bool isPTXSupported;
        REQUIRE(NvAPI_D3D12_IsFatbinPTXSupported(static_cast<ID3D12Device*>(&device), &isPTXSupported) == NVAPI_ERROR);
        REQUIRE(deviceRefCount == 0);
        REQUIRE(commandListRefCount == 0);
    }

    SECTION("IsNvShaderExtnOpCodeSupported with device returns OK") {
        auto supported = true;
        REQUIRE(NvAPI_D3D12_IsNvShaderExtnOpCodeSupported(&device, 1U, &supported) == NVAPI_OK);
        REQUIRE(supported == false);
    }

    SECTION("NvAPI_D3D12_GetGraphicsCapabilities returns OK") {
        NV_D3D12_GRAPHICS_CAPS graphicsCaps;
        graphicsCaps.bFastUAVClearSupported = false;
        REQUIRE(NvAPI_D3D12_GetGraphicsCapabilities(static_cast<ID3D12Device*>(&device), NV_D3D12_GRAPHICS_CAPS_VER1, &graphicsCaps) == NVAPI_OK);
        REQUIRE(graphicsCaps.bFastUAVClearSupported == true);
        REQUIRE(deviceRefCount == 0);
        REQUIRE(commandListRefCount == 0);
    }

    SECTION("CreateCubinComputeShader returns OK") {
        const void* cubinData = nullptr;
        auto cubinSize = 2U;
        auto blockX = 3U;
        auto blockY = 4U;
        auto blockZ = 5U;
        const char* shaderName = "";
        auto shaderHandle = reinterpret_cast<D3D12_CUBIN_DATA_HANDLE*>(0x912122);
        auto handle = &shaderHandle;
        REQUIRE_CALL(device, CreateCubinComputeShaderWithName(cubinData, cubinSize, blockX, blockY, blockZ, shaderName, handle))
            .RETURN(S_OK)
            .TIMES(2);

        REQUIRE(NvAPI_D3D12_CreateCubinComputeShaderWithName(static_cast<ID3D12Device*>(&device), cubinData, cubinSize, blockX, blockY, blockZ, shaderName, reinterpret_cast<NVDX_ObjectHandle*>(handle)) == NVAPI_OK);
        REQUIRE(NvAPI_D3D12_CreateCubinComputeShader(static_cast<ID3D12Device*>(&device), cubinData, cubinSize, blockX, blockY, blockZ, reinterpret_cast<NVDX_ObjectHandle*>(handle)) == NVAPI_OK);
        REQUIRE(deviceRefCount == 0);
        REQUIRE(commandListRefCount == 0);
    }

    SECTION("DestroyCubinComputeShader returns OK") {
        auto shaderHandle = reinterpret_cast<D3D12_CUBIN_DATA_HANDLE*>(0x912122);
        REQUIRE_CALL(device, DestroyCubinComputeShader(shaderHandle))
            .RETURN(S_OK)
            .TIMES(1);

        REQUIRE(NvAPI_D3D12_DestroyCubinComputeShader(static_cast<ID3D12Device*>(&device), reinterpret_cast<NVDX_ObjectHandle>(shaderHandle)) == NVAPI_OK);
        REQUIRE(deviceRefCount == 0);
        REQUIRE(commandListRefCount == 0);
    }

    SECTION("GetCudaTextureObject/GetCudaSurfaceObject returns OK") {
        D3D12_CPU_DESCRIPTOR_HANDLE srvHandle = {0x123456};
        D3D12_CPU_DESCRIPTOR_HANDLE samplerHandle = {0x654321};
        REQUIRE_CALL(device, GetCudaTextureObject(_, _, nullptr))
            .LR_WITH(_1.ptr == srvHandle.ptr && _2.ptr == samplerHandle.ptr)
            .RETURN(S_OK)
            .TIMES(1);
        REQUIRE_CALL(device, GetCudaSurfaceObject(_, nullptr))
            .LR_WITH(_1.ptr == srvHandle.ptr)
            .RETURN(S_OK)
            .TIMES(1);

        REQUIRE(NvAPI_D3D12_GetCudaTextureObject(static_cast<ID3D12Device*>(&device), srvHandle, samplerHandle, nullptr)  == NVAPI_OK);
        REQUIRE(NvAPI_D3D12_GetCudaSurfaceObject(static_cast<ID3D12Device*>(&device), srvHandle, nullptr)  == NVAPI_OK);
        REQUIRE(deviceRefCount == 0);
        REQUIRE(commandListRefCount == 0);
    }

    SECTION("CaptureUAVInfo returns OK") {
        REQUIRE_CALL(device, CaptureUAVInfo(nullptr))
            .RETURN(true)
            .TIMES(1);

        REQUIRE(NvAPI_D3D12_CaptureUAVInfo(static_cast<ID3D12Device*>(&device), nullptr) == NVAPI_OK);
        REQUIRE(deviceRefCount == 0);
        REQUIRE(commandListRefCount == 0);
    }

    SECTION("IsFatbinPTXSupported returns OK") {
        auto isPTXSupported = false;
        REQUIRE(NvAPI_D3D12_IsFatbinPTXSupported(static_cast<ID3D12Device*>(&device), &isPTXSupported) == NVAPI_OK);
        REQUIRE(isPTXSupported == true);
        REQUIRE(deviceRefCount == 0);
        REQUIRE(commandListRefCount == 0);
    }

    SECTION("LaunchCubinShader returns OK") {
        NVDX_ObjectHandle pShader = nullptr;
        auto blockX = 1U;
        auto blockY = 2U;
        auto blockZ = 3U;
        const void* params = nullptr;
        auto paramSize = 4U;
        REQUIRE_CALL(commandList, LaunchCubinShader(reinterpret_cast<D3D12_CUBIN_DATA_HANDLE*>(pShader), blockX, blockY, blockZ, params, paramSize))
            .RETURN(S_OK)
            .TIMES(1);

        REQUIRE(NvAPI_D3D12_LaunchCubinShader(static_cast<ID3D12GraphicsCommandList*>(&commandList), pShader, blockX, blockY, blockZ, params, paramSize) == NVAPI_OK);
        REQUIRE(deviceRefCount == 0);
        REQUIRE(commandListRefCount == 0);
    }
}
