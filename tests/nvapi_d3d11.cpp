#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"

#include "../inc/catch.hpp"
#include "../inc/catch2/trompeloeil.hpp"
#include "../src/nvapi_d3d11.cpp"
#include "nvapi_d3d11_mocks.cpp"

using namespace trompeloeil;

class UnknownMock : public mock_interface<IUnknown> {
    MAKE_MOCK2 (QueryInterface, HRESULT(REFIID, void * *), override);
    MAKE_MOCK0 (AddRef, ULONG(), override);
    MAKE_MOCK0 (Release, ULONG(), override);
};

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
    ALLOW_CALL(device, QueryInterface(ID3D11VkExtDevice1::guid, _))
        .LR_SIDE_EFFECT(*_2 = static_cast<ID3D11VkExtDevice1*>(&device))
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
    ALLOW_CALL(context, QueryInterface(ID3D11VkExtContext1::guid, _))
        .LR_SIDE_EFFECT(*_2 = static_cast<ID3D11VkExtContext1*>(&context))
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
        ALLOW_CALL(device, QueryInterface(ID3D11VkExtDevice1::guid, _))
            .RETURN(E_NOINTERFACE);
        ALLOW_CALL(context, QueryInterface(ID3D11VkExtContext::guid, _))
            .RETURN(E_NOINTERFACE);
        ALLOW_CALL(context, QueryInterface(ID3D11VkExtContext1::guid, _))
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

    SECTION("LaunchCubinShader without DXVK extension support returns error") {
        ALLOW_CALL(device, GetExtensionSupport(D3D11_VK_NVX_BINARY_IMPORT))
            .RETURN(false);
        FORBID_CALL(context, LaunchCubinShaderNVX(_, _, _, _, _, _, _, _, _, _));

        REQUIRE(NvAPI_D3D11_LaunchCubinShader(static_cast<ID3D11DeviceContext*>(&context), NVDX_ObjectHandle(), 0,0,0, nullptr, 0, nullptr, 0, nullptr, 0) == NVAPI_ERROR);
        REQUIRE(deviceRefCount == 0);
        REQUIRE(contextRefCount == 0);
    }

    SECTION("Failing CreateCubinComputeShader/CreateCubinComputeShaderWithName inside DXVK returns error") {
        ALLOW_CALL(device, CreateCubinComputeShaderWithNameNVX(_, _, _, _, _, _, _))
            .RETURN(false);

        NVDX_ObjectHandle objhandle;
        REQUIRE(NvAPI_D3D11_CreateCubinComputeShaderWithName(static_cast<ID3D11Device*>(&device), "X", 1U, 0U,0U,0U, "foo", &objhandle) == NVAPI_ERROR);
        REQUIRE(NvAPI_D3D11_CreateCubinComputeShader(static_cast<ID3D11Device*>(&device), "X", 1U, 0U,0U,0U, &objhandle) == NVAPI_ERROR);
        REQUIRE(deviceRefCount == 0);
        REQUIRE(contextRefCount == 0);
    }

    SECTION("LaunchCubinShader/CreateCubinComputeShader/CreateCubinComputeShaderWithName without extended DXVK interface returns error") {
        ALLOW_CALL(device, QueryInterface(ID3D11VkExtDevice1::guid, _))
            .RETURN(E_NOINTERFACE);
        ALLOW_CALL(context, QueryInterface(ID3D11VkExtContext1::guid, _))
            .RETURN(E_NOINTERFACE);
        FORBID_CALL(context, LaunchCubinShaderNVX(_, _, _, _, _, _, _, _, _, _));
        FORBID_CALL(device, CreateCubinComputeShaderWithNameNVX(_, _, _, _, _, _, _));

        REQUIRE(NvAPI_D3D11_LaunchCubinShader(static_cast<ID3D11DeviceContext*>(&context), NVDX_ObjectHandle(), 0,0,0, nullptr, 0, nullptr, 0, nullptr, 0) == NVAPI_ERROR);
        NVDX_ObjectHandle objhandle;
        REQUIRE(NvAPI_D3D11_CreateCubinComputeShaderWithName(static_cast<ID3D11Device*>(&device), "X", 1U, 0U,0U,0U, "foo", &objhandle) == NVAPI_ERROR);
        REQUIRE(NvAPI_D3D11_CreateCubinComputeShader(static_cast<ID3D11Device*>(&device), "X", 1U, 0U,0U,0U, &objhandle) == NVAPI_ERROR);
        REQUIRE(deviceRefCount == 0);
        REQUIRE(contextRefCount == 0);
    }

    SECTION("IsFatbinPTXSupported without DXVK extension succeeds but reports unsupported") {
        ALLOW_CALL(device, GetExtensionSupport(D3D11_VK_NVX_BINARY_IMPORT))
            .RETURN(false);

        bool sup = true;
        REQUIRE(NvAPI_D3D11_IsFatbinPTXSupported(static_cast<ID3D11Device*>(&device), &sup) == NVAPI_OK);
        REQUIRE(sup == false);
        REQUIRE(deviceRefCount == 0);
    }

    SECTION("IsFatbinPTXSupported without DXVK extension succeeds but reports unsupported") {
        ALLOW_CALL(device, GetExtensionSupport(D3D11_VK_NVX_BINARY_IMPORT))
            .RETURN(false);

        bool sup = true;
        REQUIRE(NvAPI_D3D11_IsFatbinPTXSupported(static_cast<ID3D11Device*>(&device), &sup) == NVAPI_OK);
        REQUIRE(sup == false);
        REQUIRE(deviceRefCount == 0);
    }

    SECTION("(NvAPI_D3D11_)CreateUnorderedAccessView/CreateShaderResourceView/GetResourceGPUVirtualAddress/GetResourceGPUVirtualAddressEx/GetCudaTextureObject/CreateSamplerState without extended DXVK interface returns error") {
        ALLOW_CALL(device, QueryInterface(ID3D11VkExtDevice1::guid, _))
            .RETURN(E_NOINTERFACE);
        ALLOW_CALL(context, QueryInterface(ID3D11VkExtContext1::guid, _))
            .RETURN(E_NOINTERFACE);
        FORBID_CALL(device, CreateUnorderedAccessViewAndGetDriverHandleNVX(_, _, _, _));
        FORBID_CALL(device, CreateShaderResourceViewAndGetDriverHandleNVX(_, _, _, _));
        FORBID_CALL(device, GetResourceHandleGPUVirtualAddressAndSizeNVX(_, _, _));
        FORBID_CALL(device, GetCudaTextureObjectNVX(_, _, _));
        FORBID_CALL(device, CreateSamplerStateAndGetDriverHandleNVX(_, _, _));

        D3D11BufferMock res;
        NvU32 driverhandle;
        D3D11_UNORDERED_ACCESS_VIEW_DESC uavdesc;
        ID3D11UnorderedAccessView* pUAV;
        REQUIRE(NvAPI_D3D11_CreateUnorderedAccessView(static_cast<ID3D11Device*>(&device), &res, &uavdesc, &pUAV, &driverhandle) == NVAPI_ERROR);
        D3D11_SHADER_RESOURCE_VIEW_DESC srvdesc;
        ID3D11ShaderResourceView* pSRV;
        REQUIRE(NvAPI_D3D11_CreateShaderResourceView(static_cast<ID3D11Device*>(&device), &res, &srvdesc, &pSRV, &driverhandle) == NVAPI_ERROR);
        NvU64 gpuVAStart;
        REQUIRE(NvAPI_D3D11_GetResourceGPUVirtualAddress(static_cast<ID3D11Device*>(&device), NVDX_ObjectHandle(1), &gpuVAStart) == NVAPI_ERROR);
        NV_GET_GPU_VIRTUAL_ADDRESS gva;
        gva.version = NV_GET_GPU_VIRTUAL_ADDRESS_VER1;
        gva.hResource = NVDX_ObjectHandle(1);
        REQUIRE(NvAPI_D3D11_GetResourceGPUVirtualAddressEx(static_cast<ID3D11Device*>(&device), &gva) == NVAPI_ERROR);
        D3D11_SAMPLER_DESC samplerdesc;
        ID3D11SamplerState* pSamplerState;
        REQUIRE(NvAPI_D3D11_CreateSamplerState(static_cast<ID3D11Device*>(&device), &samplerdesc, &pSamplerState, &driverhandle) == NVAPI_ERROR);
        REQUIRE(NvAPI_D3D11_GetCudaTextureObject(static_cast<ID3D11Device*>(&device), 0x1, 0x2, &driverhandle) == NVAPI_ERROR);
        REQUIRE(deviceRefCount == 0);
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

    SECTION("DestroyCubinComputeShader treats handle as a COM object and releases it") {
        // it's part of our contract with DXVK that cuda shader handles are really COM objects
        UnknownMock comobj;
        REQUIRE_CALL(comobj, Release())
            .TIMES(1)
            .RETURN(42);

        NVDX_ObjectHandle handle = reinterpret_cast<NVDX_ObjectHandle>(&comobj);
        REQUIRE(NvAPI_D3D11_DestroyCubinComputeShader(static_cast<ID3D11Device*>(&device), handle) == NVAPI_OK);
        REQUIRE(deviceRefCount == 0);
        REQUIRE(contextRefCount == 0);
    }

    SECTION("DestroyCubinComputeShader does not succeed (or crash) on a NULL or NVDX_OBJECT_NONE handle") {
        // this also checks the assumption that NVDX_OBJECT_NONE casts <-> NULL, which should forever be true but will break some stuff in subtle ways if not, so ¯\_(ツ)_/¯
        REQUIRE(NVDX_OBJECT_NONE == reinterpret_cast<NVDX_ObjectHandle>(NULL /* not nullptr which is castproofed */));
        REQUIRE(reinterpret_cast<void*>(NVDX_OBJECT_NONE) == nullptr);
        REQUIRE(NvAPI_D3D11_DestroyCubinComputeShader(static_cast<ID3D11Device*>(&device), nullptr) != NVAPI_OK);
        REQUIRE(NvAPI_D3D11_DestroyCubinComputeShader(static_cast<ID3D11Device*>(&device), NVDX_OBJECT_NONE) != NVAPI_OK);
        REQUIRE(deviceRefCount == 0);
        REQUIRE(contextRefCount == 0);
    }

    SECTION("GetResourceHandle returns OK") {
        // Test device call twice to ensure correct reference counting when hitting the device cache
        D3D11BufferMock res;
        NVDX_ObjectHandle handle = reinterpret_cast<NVDX_ObjectHandle>(&res);
        REQUIRE(NvAPI_D3D11_GetResourceHandle(static_cast<ID3D11Device*>(&device), &res, &handle) == NVAPI_OK);
        REQUIRE(NvAPI_D3D11_GetResourceHandle(static_cast<ID3D11Device*>(&device), &res, &handle) == NVAPI_OK);
        REQUIRE(deviceRefCount == 0);
        REQUIRE(contextRefCount == 0);
    }

    SECTION("GetResourceHandle with NULL argument returns InvalidArgument") {
        D3D11BufferMock res;
        NVDX_ObjectHandle handle;
        REQUIRE(NvAPI_D3D11_GetResourceHandle(nullptr, &res, &handle) == NVAPI_INVALID_ARGUMENT);
        REQUIRE(NvAPI_D3D11_GetResourceHandle(static_cast<ID3D11Device*>(&device), nullptr, &handle) == NVAPI_INVALID_ARGUMENT);
        REQUIRE(NvAPI_D3D11_GetResourceHandle(static_cast<ID3D11Device*>(&device), &res, nullptr) == NVAPI_INVALID_ARGUMENT);
        REQUIRE(deviceRefCount == 0);
        REQUIRE(contextRefCount == 0);
    }

    SECTION("GetResourceHandle output handle is a simple recast of the input resource pointer") {
        // While the handles returned by NvAPI are opaque with unspecified values, our interaction with DXVK *requires* that we implement them as a simple recast
        D3D11BufferMock res;
        NVDX_ObjectHandle handle;
        REQUIRE(NvAPI_D3D11_GetResourceHandle(static_cast<ID3D11Device*>(&device), &res, &handle) == NVAPI_OK);
        REQUIRE(reinterpret_cast<void*>(handle) == reinterpret_cast<void*>(&res));
    }
}
