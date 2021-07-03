#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"

#include "../inc/catch.hpp"
#include "../inc/catch2/trompeloeil.hpp"
#include "../src/nvapi_d3d11.cpp"
#include "nvapi_d3d11_mocks.cpp"

using namespace trompeloeil;

TEST_CASE("D3D11 methods return OK", "[d3d11]") {
    D3D11DxvkDeviceMock device;
    D3D11DxvkDeviceContextMock context;

    ALLOW_CALL(device, QueryInterface(ID3D11VkExtDevice::guid, _))
        .LR_SIDE_EFFECT(*_2 = static_cast<ID3D11VkExtDevice*>(&device))
        .RETURN(S_OK);
    ALLOW_CALL(device, QueryInterface(IID_ID3D11Device, _))
        .LR_SIDE_EFFECT(*_2 = static_cast<ID3D11Device*>(&device))
        .RETURN(S_OK);
    ALLOW_CALL(device, Release())
        .RETURN(0);
    ALLOW_CALL(device, GetExtensionSupport(_))
        .RETURN(true);
    ALLOW_CALL(device, GetImmediateContext(_))
        .LR_SIDE_EFFECT(*_1 = &context);

    ALLOW_CALL(context, QueryInterface(ID3D11VkExtContext::guid, _))
        .LR_SIDE_EFFECT(*_2 = static_cast<ID3D11VkExtContext*>(&context))
        .RETURN(S_OK);
    ALLOW_CALL(context, QueryInterface(IID_ID3D11DeviceContext, _))
        .LR_SIDE_EFFECT(*_2 = static_cast<ID3D11DeviceContext*>(&context))
        .RETURN(S_OK);
    ALLOW_CALL(context, QueryInterface(IID_ID3D11Device, _))
        .RETURN(E_FAIL);
    ALLOW_CALL(context, Release())
        .RETURN(0);
    ALLOW_CALL(context, GetDevice(_))
        .LR_SIDE_EFFECT(*_1 = &device);

    SECTION("SetDepthBoundsTests with device returns OK") {
        auto enable = true;
        auto min = 0.4f;
        auto max = 0.7f;
        REQUIRE_CALL(context, SetDepthBoundsTest(enable, min, max));
        REQUIRE(NvAPI_D3D11_SetDepthBoundsTest(static_cast<ID3D11Device*>(&device), enable, min, max) == NVAPI_OK);
    }

    SECTION("SetDepthBoundsTests with context returns OK") {
        auto enable = true;
        auto min = 0.5f;
        auto max = 0.8f;
        REQUIRE_CALL(context, SetDepthBoundsTest(enable, min, max));
        REQUIRE(NvAPI_D3D11_SetDepthBoundsTest(static_cast<ID3D11DeviceContext*>(&context), enable, min, max) == NVAPI_OK);
    }

    SECTION("BeginUAVOverlap with device returns OK") {
        REQUIRE_CALL(context, SetBarrierControl(D3D11_VK_BARRIER_CONTROL_IGNORE_WRITE_AFTER_WRITE));
        REQUIRE(NvAPI_D3D11_BeginUAVOverlap(static_cast<ID3D11Device*>(&device)) == NVAPI_OK);
    }

    SECTION("BeginUAVOverlap with context returns OK") {
        REQUIRE_CALL(context, SetBarrierControl(D3D11_VK_BARRIER_CONTROL_IGNORE_WRITE_AFTER_WRITE));
        REQUIRE(NvAPI_D3D11_BeginUAVOverlap(static_cast<ID3D11DeviceContext*>(&context)) == NVAPI_OK);
    }

    SECTION("EndUAVOverlap with device returns OK") {
        REQUIRE_CALL(context, SetBarrierControl(0U));
        REQUIRE(NvAPI_D3D11_EndUAVOverlap(static_cast<ID3D11Device*>(&device)) == NVAPI_OK);
    }

    SECTION("EndUAVOverlap with context returns OK") {
        REQUIRE_CALL(context, SetBarrierControl(0U));
        REQUIRE(NvAPI_D3D11_EndUAVOverlap(static_cast<ID3D11DeviceContext*>(&context)) == NVAPI_OK);
    }

    SECTION("MultiDrawInstancedIndirect returns OK") {
        auto drawCount = 4U;
        D3D11BufferMock buffer;
        auto offsetForArgs = 8U;
        auto strideForArgs = 16U;
        REQUIRE_CALL(context, MultiDrawIndirect(drawCount, &buffer, offsetForArgs, strideForArgs));
        REQUIRE(NvAPI_D3D11_MultiDrawInstancedIndirect(static_cast<ID3D11DeviceContext*>(&context), drawCount, &buffer, offsetForArgs, strideForArgs) == NVAPI_OK);
    }

    SECTION("MultiDrawIndexedInstancedIndirect returns OK") {
        auto drawCount = 6U;
        D3D11BufferMock buffer;
        auto offsetForArgs = 12U;
        auto strideForArgs = 20U;
        REQUIRE_CALL(context, MultiDrawIndexedIndirect(drawCount, &buffer, offsetForArgs, strideForArgs));
        REQUIRE(NvAPI_D3D11_MultiDrawIndexedInstancedIndirect(static_cast<ID3D11DeviceContext*>(&context), drawCount, &buffer, offsetForArgs, strideForArgs) == NVAPI_OK);
    }
}
