#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"

#include "../inc/catch.hpp"
#include "../inc/catch2/trompeloeil.hpp"
#include "../src/nvapi_d3d11.cpp"
#include "nvapi_d3d11_mocks.cpp"

using namespace trompeloeil;

TEST_CASE("D3D11 methods succeed", "[.d3d11]") {
    D3D11DxvkDeviceMock device;
    D3D11DxvkDeviceContextMock context;
    auto deviceRefCount = 0;
    auto contextRefCount = 0;

    ALLOW_CALL(device, QueryInterface(IID_ID3D11Device, _))
        .LR_SIDE_EFFECT(*_2 = static_cast<ID3D11Device*>(&device))
        .LR_SIDE_EFFECT(deviceRefCount++)
        .RETURN(S_OK);
    ALLOW_CALL(device, QueryInterface(ID3D11VkExtDevice::guid, _))
        .LR_SIDE_EFFECT(*_2 = static_cast<ID3D11VkExtDevice*>(&device))
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
    ALLOW_CALL(device, GetImmediateContext(_))
        .LR_SIDE_EFFECT(*_1 = &context)
        .LR_SIDE_EFFECT(contextRefCount++);

    ALLOW_CALL(context, QueryInterface(IID_ID3D11Device, _))
        .RETURN(E_NOINTERFACE);
    ALLOW_CALL(context, QueryInterface(IID_ID3D11DeviceContext, _))
        .LR_SIDE_EFFECT(*_2 = static_cast<ID3D11DeviceContext*>(&context))
        .LR_SIDE_EFFECT(contextRefCount++)
        .RETURN(S_OK);
    ALLOW_CALL(context, QueryInterface(ID3D11VkExtContext::guid, _))
        .LR_SIDE_EFFECT(*_2 = static_cast<ID3D11VkExtContext*>(&context))
        .LR_SIDE_EFFECT(contextRefCount++)
        .RETURN(S_OK);
    ALLOW_CALL(context, AddRef())
        .LR_SIDE_EFFECT(contextRefCount++)
        .RETURN(contextRefCount);
    ALLOW_CALL(context, Release())
        .LR_SIDE_EFFECT(contextRefCount--)
        .RETURN(contextRefCount);
    ALLOW_CALL(context, GetDevice(_))
        .LR_SIDE_EFFECT(*_1 = &device)
        .LR_SIDE_EFFECT(deviceRefCount++);

    SECTION("D3D11 methods without DXVK return error") {
        ALLOW_CALL(device, QueryInterface(ID3D11VkExtDevice::guid, _))
            .RETURN(E_NOINTERFACE);
        ALLOW_CALL(context, QueryInterface(ID3D11VkExtContext::guid, _))
            .RETURN(E_NOINTERFACE);
        FORBID_CALL(context, SetDepthBoundsTest(_, _, _));
        FORBID_CALL(context, SetBarrierControl(_));
        FORBID_CALL(context, MultiDrawIndirect(_, _, _, _));
        FORBID_CALL(context, MultiDrawIndexedIndirect(_, _, _, _));

        REQUIRE(NvAPI_D3D11_SetDepthBoundsTest(static_cast<ID3D11Device*>(&device), true, 0.5f, 0.8f) == NVAPI_ERROR);
        REQUIRE(NvAPI_D3D11_SetDepthBoundsTest(static_cast<ID3D11DeviceContext*>(&context), true, 0.5f, 0.8f) == NVAPI_ERROR);
        REQUIRE(NvAPI_D3D11_BeginUAVOverlap(static_cast<ID3D11Device*>(&device)) == NVAPI_ERROR);
        REQUIRE(NvAPI_D3D11_BeginUAVOverlap(static_cast<ID3D11DeviceContext*>(&context)) == NVAPI_ERROR);
        REQUIRE(NvAPI_D3D11_EndUAVOverlap(static_cast<ID3D11Device*>(&device)) == NVAPI_ERROR);
        REQUIRE(NvAPI_D3D11_EndUAVOverlap(static_cast<ID3D11DeviceContext*>(&context)) == NVAPI_ERROR);
        D3D11BufferMock buffer;
        REQUIRE(NvAPI_D3D11_MultiDrawInstancedIndirect(static_cast<ID3D11DeviceContext*>(&context), 4U, &buffer, 8U, 16U) == NVAPI_ERROR);
        REQUIRE(NvAPI_D3D11_MultiDrawIndexedInstancedIndirect(static_cast<ID3D11DeviceContext*>(&context), 6U, &buffer, 12U, 20U) == NVAPI_ERROR);
        REQUIRE(deviceRefCount == 0);
        REQUIRE(contextRefCount == 0);
    }

    SECTION("SetDepthBoundsTests without DXVK extension support returns error") {
        ALLOW_CALL(device, GetExtensionSupport(D3D11_VK_EXT_DEPTH_BOUNDS))
            .RETURN(false);
        FORBID_CALL(context, SetDepthBoundsTest(_, _, _));

        REQUIRE(NvAPI_D3D11_SetDepthBoundsTest(static_cast<ID3D11Device*>(&device), true, 0.4f, 0.7f) == NVAPI_ERROR);
        REQUIRE(NvAPI_D3D11_SetDepthBoundsTest(static_cast<ID3D11DeviceContext*>(&context), true, 0.5f, 0.8f) == NVAPI_ERROR);
        REQUIRE(deviceRefCount == 0);
        REQUIRE(contextRefCount == 0);
    }

    SECTION("BeginUAVOverlap/EndUAVOverlap without DXVK extension support returns error") {
        ALLOW_CALL(device, GetExtensionSupport(D3D11_VK_EXT_BARRIER_CONTROL))
            .RETURN(false);
        FORBID_CALL(context, SetBarrierControl(_));

        REQUIRE(NvAPI_D3D11_BeginUAVOverlap(static_cast<ID3D11Device *>(&device)) == NVAPI_ERROR);
        REQUIRE(NvAPI_D3D11_EndUAVOverlap(static_cast<ID3D11Device *>(&device)) == NVAPI_ERROR);
        REQUIRE(NvAPI_D3D11_BeginUAVOverlap(static_cast<ID3D11DeviceContext *>(&context)) == NVAPI_ERROR);
        REQUIRE(NvAPI_D3D11_EndUAVOverlap(static_cast<ID3D11DeviceContext *>(&context)) == NVAPI_ERROR);
        REQUIRE(deviceRefCount == 0);
        REQUIRE(contextRefCount == 0);
    }

    SECTION("MultiDrawInstancedIndirect/MultiDrawIndexedInstancedIndirect without DXVK extension support returns error") {
        ALLOW_CALL(device, GetExtensionSupport(D3D11_VK_EXT_MULTI_DRAW_INDIRECT))
            .RETURN(false);
        FORBID_CALL(context, MultiDrawIndirect(_, _, _, _));
        FORBID_CALL(context, MultiDrawIndexedIndirect(_, _, _, _));

        D3D11BufferMock buffer;
        REQUIRE(NvAPI_D3D11_MultiDrawInstancedIndirect(static_cast<ID3D11DeviceContext*>(&context), 4U, &buffer, 8U, 16U) == NVAPI_ERROR);
        REQUIRE(NvAPI_D3D11_MultiDrawIndexedInstancedIndirect(static_cast<ID3D11DeviceContext*>(&context), 6U, &buffer, 12U, 20U) == NVAPI_ERROR);
        REQUIRE(deviceRefCount == 0);
        REQUIRE(contextRefCount == 0);
    }

    // Test failing scenarios first because caches won't be reset between tests (we don't cache negatives)

    SECTION("IsNvShaderExtnOpCodeSupported returns OK") {
        bool supportedForDevice = true;
        bool supportedForContext = true;
        REQUIRE(NvAPI_D3D11_IsNvShaderExtnOpCodeSupported(static_cast<ID3D11Device*>(&device), 1U, &supportedForDevice) == NVAPI_OK);
        REQUIRE(NvAPI_D3D11_IsNvShaderExtnOpCodeSupported(static_cast<ID3D11DeviceContext*>(&context), 1U, &supportedForContext) == NVAPI_OK);
        REQUIRE(supportedForDevice == false);
        REQUIRE(supportedForContext == false);
    }

    SECTION("SetDepthBoundsTests returns OK") {
        auto enable = true;
        auto min = 0.4f;
        auto max = 0.7f;
        REQUIRE_CALL(context, SetDepthBoundsTest(enable, min, max))
            .TIMES(3);

        // Test device call twice to ensure correct reference counting when hitting the device cache
        REQUIRE(NvAPI_D3D11_SetDepthBoundsTest(static_cast<ID3D11Device*>(&device), enable, min, max) == NVAPI_OK);
        REQUIRE(NvAPI_D3D11_SetDepthBoundsTest(static_cast<ID3D11Device*>(&device), enable, min, max) == NVAPI_OK);
        REQUIRE(NvAPI_D3D11_SetDepthBoundsTest(static_cast<ID3D11DeviceContext*>(&context), enable, min, max) == NVAPI_OK);
        REQUIRE(deviceRefCount == 0);
        REQUIRE(contextRefCount == 0);
    }

    SECTION("BeginUAVOverlap returns OK") {
        REQUIRE_CALL(context, SetBarrierControl(D3D11_VK_BARRIER_CONTROL_IGNORE_WRITE_AFTER_WRITE))
            .TIMES(3);

        // Test device call twice to ensure correct reference counting when hitting the device cache
        REQUIRE(NvAPI_D3D11_BeginUAVOverlap(static_cast<ID3D11Device*>(&device)) == NVAPI_OK);
        REQUIRE(NvAPI_D3D11_BeginUAVOverlap(static_cast<ID3D11Device*>(&device)) == NVAPI_OK);
        REQUIRE(NvAPI_D3D11_BeginUAVOverlap(static_cast<ID3D11DeviceContext*>(&context)) == NVAPI_OK);
        REQUIRE(deviceRefCount == 0);
        REQUIRE(contextRefCount == 0);
    }

    SECTION("EndUAVOverlap returns OK") {
        REQUIRE_CALL(context, SetBarrierControl(0U))
            .TIMES(3);

        // Test device call twice to ensure correct reference counting when hitting the device cache
        REQUIRE(NvAPI_D3D11_EndUAVOverlap(static_cast<ID3D11Device*>(&device)) == NVAPI_OK);
        REQUIRE(NvAPI_D3D11_EndUAVOverlap(static_cast<ID3D11Device*>(&device)) == NVAPI_OK);
        REQUIRE(NvAPI_D3D11_EndUAVOverlap(static_cast<ID3D11DeviceContext*>(&context)) == NVAPI_OK);
        REQUIRE(deviceRefCount == 0);
        REQUIRE(contextRefCount == 0);
    }

    SECTION("MultiDrawInstancedIndirect/MultiDrawIndexedInstancedIndirect returns OK") {
        auto drawCount = 4U;
        D3D11BufferMock buffer;
        auto offsetForArgs = 8U;
        auto strideForArgs = 16U;
        REQUIRE_CALL(context, MultiDrawIndirect(drawCount, &buffer, offsetForArgs, strideForArgs))
            .TIMES(1);
        REQUIRE_CALL(context, MultiDrawIndexedIndirect(drawCount, &buffer, offsetForArgs, strideForArgs))
            .TIMES(1);

        REQUIRE(NvAPI_D3D11_MultiDrawInstancedIndirect(static_cast<ID3D11DeviceContext*>(&context), drawCount, &buffer, offsetForArgs, strideForArgs) == NVAPI_OK);
        REQUIRE(NvAPI_D3D11_MultiDrawIndexedInstancedIndirect(static_cast<ID3D11DeviceContext*>(&context), drawCount, &buffer, offsetForArgs, strideForArgs) == NVAPI_OK);
        REQUIRE(deviceRefCount == 0);
        REQUIRE(contextRefCount == 0);
    }
}
