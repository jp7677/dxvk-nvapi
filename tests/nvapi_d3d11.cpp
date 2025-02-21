#include "nvapi_tests_private.h"
#include "mocks/d3d_mocks.h"
#include "mocks/d3d11_mocks.h"
#include "nvapi/resource_factory_util.h"

using namespace trompeloeil;

TEST_CASE("D3D11 methods succeed", "[.d3d11]") {
    D3D11DxvkDeviceMock device;
    D3D11DxvkDeviceContextMock context;
    auto deviceRefCount = 0;
    auto contextRefCount = 0;

    ALLOW_CALL(device, QueryInterface(__uuidof(ID3D11Device), _))
        .LR_SIDE_EFFECT(*_2 = static_cast<ID3D11Device*>(&device))
        .LR_SIDE_EFFECT(deviceRefCount++)
        .RETURN(S_OK);
    ALLOW_CALL(device, QueryInterface(__uuidof(ID3D11VkExtDevice), _))
        .LR_SIDE_EFFECT(*_2 = static_cast<ID3D11VkExtDevice*>(&device))
        .LR_SIDE_EFFECT(deviceRefCount++)
        .RETURN(S_OK);
    ALLOW_CALL(device, QueryInterface(__uuidof(ID3D11VkExtDevice1), _))
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

    ALLOW_CALL(context, QueryInterface(__uuidof(ID3D11Device), _))
        .RETURN(E_NOINTERFACE);
    ALLOW_CALL(context, QueryInterface(__uuidof(ID3D11DeviceContext), _))
        .LR_SIDE_EFFECT(*_2 = static_cast<ID3D11DeviceContext*>(&context))
        .LR_SIDE_EFFECT(contextRefCount++)
        .RETURN(S_OK);
    ALLOW_CALL(context, QueryInterface(__uuidof(ID3D11VkExtContext), _))
        .LR_SIDE_EFFECT(*_2 = static_cast<ID3D11VkExtContext*>(&context))
        .LR_SIDE_EFFECT(contextRefCount++)
        .RETURN(S_OK);
    ALLOW_CALL(context, QueryInterface(__uuidof(ID3D11VkExtContext1), _))
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
        ALLOW_CALL(device, QueryInterface(__uuidof(ID3D11VkExtDevice), _))
            .RETURN(E_NOINTERFACE);
        ALLOW_CALL(device, QueryInterface(__uuidof(ID3D11VkExtDevice1), _))
            .RETURN(E_NOINTERFACE);
        ALLOW_CALL(context, QueryInterface(__uuidof(ID3D11VkExtContext), _))
            .RETURN(E_NOINTERFACE);
        ALLOW_CALL(context, QueryInterface(__uuidof(ID3D11VkExtContext1), _))
            .RETURN(E_NOINTERFACE);
        FORBID_CALL(context, SetDepthBoundsTest(_, _, _));
        FORBID_CALL(context, SetBarrierControl(_));
        FORBID_CALL(context, MultiDrawIndirect(_, _, _, _));
        FORBID_CALL(context, MultiDrawIndexedIndirect(_, _, _, _));

        REQUIRE(NvAPI_D3D11_SetDepthBoundsTest(static_cast<ID3D11Device*>(&device), true, 0.5f, 0.8f) == NVAPI_NO_IMPLEMENTATION);
        REQUIRE(NvAPI_D3D11_SetDepthBoundsTest(static_cast<ID3D11DeviceContext*>(&context), true, 0.5f, 0.8f) == NVAPI_NO_IMPLEMENTATION);
        REQUIRE(NvAPI_D3D11_BeginUAVOverlap(static_cast<ID3D11Device*>(&device)) == NVAPI_NO_IMPLEMENTATION);
        REQUIRE(NvAPI_D3D11_BeginUAVOverlap(static_cast<ID3D11DeviceContext*>(&context)) == NVAPI_NO_IMPLEMENTATION);
        REQUIRE(NvAPI_D3D11_EndUAVOverlap(static_cast<ID3D11Device*>(&device)) == NVAPI_NO_IMPLEMENTATION);
        REQUIRE(NvAPI_D3D11_EndUAVOverlap(static_cast<ID3D11DeviceContext*>(&context)) == NVAPI_NO_IMPLEMENTATION);
        D3D11BufferMock buffer;
        REQUIRE(NvAPI_D3D11_MultiDrawInstancedIndirect(static_cast<ID3D11DeviceContext*>(&context), 4U, &buffer, 8U, 16U) == NVAPI_NO_IMPLEMENTATION);
        REQUIRE(NvAPI_D3D11_MultiDrawIndexedInstancedIndirect(static_cast<ID3D11DeviceContext*>(&context), 6U, &buffer, 12U, 20U) == NVAPI_NO_IMPLEMENTATION);
    }

    SECTION("SetDepthBoundsTests without DXVK extension support returns error") {
        ALLOW_CALL(device, GetExtensionSupport(D3D11_VK_EXT_DEPTH_BOUNDS))
            .RETURN(false);
        FORBID_CALL(context, SetDepthBoundsTest(_, _, _));

        REQUIRE(NvAPI_D3D11_SetDepthBoundsTest(static_cast<ID3D11Device*>(&device), true, 0.4f, 0.7f) == NVAPI_NO_IMPLEMENTATION);
        REQUIRE(NvAPI_D3D11_SetDepthBoundsTest(static_cast<ID3D11DeviceContext*>(&context), true, 0.5f, 0.8f) == NVAPI_NO_IMPLEMENTATION);
    }

    SECTION("BeginUAVOverlap/EndUAVOverlap without DXVK extension support returns error") {
        ALLOW_CALL(device, GetExtensionSupport(D3D11_VK_EXT_BARRIER_CONTROL))
            .RETURN(false);
        FORBID_CALL(context, SetBarrierControl(_));

        REQUIRE(NvAPI_D3D11_BeginUAVOverlap(static_cast<ID3D11Device*>(&device)) == NVAPI_NO_IMPLEMENTATION);
        REQUIRE(NvAPI_D3D11_EndUAVOverlap(static_cast<ID3D11Device*>(&device)) == NVAPI_NO_IMPLEMENTATION);
        REQUIRE(NvAPI_D3D11_BeginUAVOverlap(static_cast<ID3D11DeviceContext*>(&context)) == NVAPI_NO_IMPLEMENTATION);
        REQUIRE(NvAPI_D3D11_EndUAVOverlap(static_cast<ID3D11DeviceContext*>(&context)) == NVAPI_NO_IMPLEMENTATION);
    }

    SECTION("MultiDrawInstancedIndirect/MultiDrawIndexedInstancedIndirect without DXVK extension support returns error") {
        ALLOW_CALL(device, GetExtensionSupport(D3D11_VK_EXT_MULTI_DRAW_INDIRECT))
            .RETURN(false);
        FORBID_CALL(context, MultiDrawIndirect(_, _, _, _));
        FORBID_CALL(context, MultiDrawIndexedIndirect(_, _, _, _));

        D3D11BufferMock buffer;
        REQUIRE(NvAPI_D3D11_MultiDrawInstancedIndirect(static_cast<ID3D11DeviceContext*>(&context), 4U, &buffer, 8U, 16U) == NVAPI_NO_IMPLEMENTATION);
        REQUIRE(NvAPI_D3D11_MultiDrawIndexedInstancedIndirect(static_cast<ID3D11DeviceContext*>(&context), 6U, &buffer, 12U, 20U) == NVAPI_NO_IMPLEMENTATION);
    }

    SECTION("LaunchCubinShader without DXVK extension support returns error") {
        ALLOW_CALL(device, GetExtensionSupport(D3D11_VK_NVX_BINARY_IMPORT))
            .RETURN(false);
        ALLOW_CALL(device, GetExtensionSupport(D3D11_VK_NVX_IMAGE_VIEW_HANDLE))
            .RETURN(false);
        FORBID_CALL(context, LaunchCubinShaderNVX(_, _, _, _, _, _, _, _, _, _));

        REQUIRE(NvAPI_D3D11_LaunchCubinShader(static_cast<ID3D11DeviceContext*>(&context), NVDX_ObjectHandle(), 0, 0, 0, nullptr, 0, nullptr, 0, nullptr, 0) == NVAPI_NO_IMPLEMENTATION);
    }

    SECTION("IsFatbinPTXSupported without DXVK extension returns OK but reports unsupported") {
        ALLOW_CALL(device, GetExtensionSupport(D3D11_VK_NVX_BINARY_IMPORT))
            .RETURN(false);
        ALLOW_CALL(device, GetExtensionSupport(D3D11_VK_NVX_IMAGE_VIEW_HANDLE))
            .RETURN(false);

        bool supported = true;
        REQUIRE(NvAPI_D3D11_IsFatbinPTXSupported(static_cast<ID3D11Device*>(&device), &supported) == NVAPI_OK);
        REQUIRE(supported == false);
    }

    SECTION("CreateCubinComputeShader/CreateCubinComputeShaderWithName/LaunchCubinShader without extended DXVK interface returns error") {
        ALLOW_CALL(device, QueryInterface(__uuidof(ID3D11VkExtDevice1), _))
            .RETURN(E_NOINTERFACE);
        ALLOW_CALL(context, QueryInterface(__uuidof(ID3D11VkExtContext1), _))
            .RETURN(E_NOINTERFACE);
        FORBID_CALL(device, CreateCubinComputeShaderWithNameNVX(_, _, _, _, _, _, _));
        FORBID_CALL(context, LaunchCubinShaderNVX(_, _, _, _, _, _, _, _, _, _));

        NVDX_ObjectHandle objhandle;
        REQUIRE(NvAPI_D3D11_CreateCubinComputeShaderWithName(static_cast<ID3D11Device*>(&device), "X", 1U, 0U, 0U, 0U, "foo", &objhandle) == NVAPI_NO_IMPLEMENTATION);
        REQUIRE(NvAPI_D3D11_CreateCubinComputeShader(static_cast<ID3D11Device*>(&device), "X", 1U, 0U, 0U, 0U, &objhandle) == NVAPI_NO_IMPLEMENTATION);
        REQUIRE(NvAPI_D3D11_LaunchCubinShader(static_cast<ID3D11DeviceContext*>(&context), NVDX_ObjectHandle(), 0, 0, 0, nullptr, 0, nullptr, 0, nullptr, 0) == NVAPI_NO_IMPLEMENTATION);
    }

    SECTION("IsFatbinPTXSupported without extended DXVK interface returns OK but reports unsupported") {
        ALLOW_CALL(device, QueryInterface(__uuidof(ID3D11VkExtDevice1), _))
            .RETURN(E_NOINTERFACE);
        ALLOW_CALL(context, QueryInterface(__uuidof(ID3D11VkExtContext1), _))
            .RETURN(E_NOINTERFACE);

        bool supported = true;
        REQUIRE(NvAPI_D3D11_IsFatbinPTXSupported(static_cast<ID3D11Device*>(&device), &supported) == NVAPI_OK);
        REQUIRE(supported == false);
    }

    SECTION("CreateUnorderedAccessView/CreateShaderResourceView/GetResourceGPUVirtualAddress/GetResourceGPUVirtualAddressEx/GetCudaTextureObject/CreateSamplerState without extended DXVK interface returns error") {
        ALLOW_CALL(device, QueryInterface(__uuidof(ID3D11VkExtDevice1), _))
            .RETURN(E_NOINTERFACE);
        ALLOW_CALL(context, QueryInterface(__uuidof(ID3D11VkExtContext1), _))
            .RETURN(E_NOINTERFACE);
        FORBID_CALL(device, CreateUnorderedAccessViewAndGetDriverHandleNVX(_, _, _, _));
        FORBID_CALL(device, CreateShaderResourceViewAndGetDriverHandleNVX(_, _, _, _));
        FORBID_CALL(device, GetResourceHandleGPUVirtualAddressAndSizeNVX(_, _, _));
        FORBID_CALL(device, GetCudaTextureObjectNVX(_, _, _));
        FORBID_CALL(device, CreateSamplerStateAndGetDriverHandleNVX(_, _, _));

        D3D11BufferMock resource;
        NvU32 handle;
        D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
        ID3D11UnorderedAccessView* pUAV;
        REQUIRE(NvAPI_D3D11_CreateUnorderedAccessView(static_cast<ID3D11Device*>(&device), &resource, &uavDesc, &pUAV, &handle) == NVAPI_NO_IMPLEMENTATION);
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
        ID3D11ShaderResourceView* srv;
        REQUIRE(NvAPI_D3D11_CreateShaderResourceView(static_cast<ID3D11Device*>(&device), &resource, &srvDesc, &srv, &handle) == NVAPI_NO_IMPLEMENTATION);
        NvU64 gpuVAStart;
        REQUIRE(NvAPI_D3D11_GetResourceGPUVirtualAddress(static_cast<ID3D11Device*>(&device), NVDX_ObjectHandle(1), &gpuVAStart) == NVAPI_NO_IMPLEMENTATION);
        NV_GET_GPU_VIRTUAL_ADDRESS gva;
        gva.version = NV_GET_GPU_VIRTUAL_ADDRESS_VER1;
        gva.hResource = reinterpret_cast<NVDX_ObjectHandle>(1);
        REQUIRE(NvAPI_D3D11_GetResourceGPUVirtualAddressEx(static_cast<ID3D11Device*>(&device), &gva) == NVAPI_NO_IMPLEMENTATION);
        D3D11_SAMPLER_DESC samplerDesc;
        ID3D11SamplerState* samplerState;
        REQUIRE(NvAPI_D3D11_CreateSamplerState(static_cast<ID3D11Device*>(&device), &samplerDesc, &samplerState, &handle) == NVAPI_NO_IMPLEMENTATION);
        REQUIRE(NvAPI_D3D11_GetCudaTextureObject(static_cast<ID3D11Device*>(&device), 0x1, 0x2, &handle) == NVAPI_NO_IMPLEMENTATION);
    }

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
    }

    SECTION("BeginUAVOverlap returns OK") {
        REQUIRE_CALL(context, SetBarrierControl(D3D11_VK_BARRIER_CONTROL_IGNORE_WRITE_AFTER_WRITE))
            .TIMES(3);

        // Test device call twice to ensure correct reference counting when hitting the device cache
        REQUIRE(NvAPI_D3D11_BeginUAVOverlap(static_cast<ID3D11Device*>(&device)) == NVAPI_OK);
        REQUIRE(NvAPI_D3D11_BeginUAVOverlap(static_cast<ID3D11Device*>(&device)) == NVAPI_OK);
        REQUIRE(NvAPI_D3D11_BeginUAVOverlap(static_cast<ID3D11DeviceContext*>(&context)) == NVAPI_OK);
    }

    SECTION("EndUAVOverlap returns OK") {
        REQUIRE_CALL(context, SetBarrierControl(0U))
            .TIMES(3);

        // Test device call twice to ensure correct reference counting when hitting the device cache
        REQUIRE(NvAPI_D3D11_EndUAVOverlap(static_cast<ID3D11Device*>(&device)) == NVAPI_OK);
        REQUIRE(NvAPI_D3D11_EndUAVOverlap(static_cast<ID3D11Device*>(&device)) == NVAPI_OK);
        REQUIRE(NvAPI_D3D11_EndUAVOverlap(static_cast<ID3D11DeviceContext*>(&context)) == NVAPI_OK);
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
    }

    SECTION("LaunchCubinShader/CreateCubinComputeShader/CreateCubinComputeShaderWithName returns OK") {
        auto shader = "X";
        NVDX_ObjectHandle objhandle{};
        REQUIRE_CALL(device, CreateCubinComputeShaderWithNameNVX(reinterpret_cast<void*>(&shader), 1U, 2U, 3U, 4U, _, reinterpret_cast<IUnknown**>(&objhandle)))
            .WITH(_6 == std::string(""))
            .TIMES(1)
            .RETURN(true);
        REQUIRE_CALL(device, CreateCubinComputeShaderWithNameNVX(reinterpret_cast<void*>(&shader), 1U, 2U, 3U, 4U, _, reinterpret_cast<IUnknown**>(&objhandle)))
            .WITH(_6 == std::string("shader"))
            .TIMES(1)
            .RETURN(true);
        REQUIRE_CALL(context, LaunchCubinShaderNVX(reinterpret_cast<IUnknown*>(objhandle), 1U, 2U, 3U, nullptr, 0U, nullptr, 0U, nullptr, 0U))
            .TIMES(1)
            .RETURN(true);

        REQUIRE(NvAPI_D3D11_CreateCubinComputeShader(static_cast<ID3D11Device*>(&device), reinterpret_cast<void*>(&shader), 1U, 2U, 3U, 4U, &objhandle) == NVAPI_OK);
        REQUIRE(NvAPI_D3D11_CreateCubinComputeShaderWithName(static_cast<ID3D11Device*>(&device), reinterpret_cast<void*>(&shader), 1U, 2U, 3U, 4U, "shader", &objhandle) == NVAPI_OK);
        REQUIRE(NvAPI_D3D11_LaunchCubinShader(static_cast<ID3D11DeviceContext*>(&context), objhandle, 1U, 2U, 3U, nullptr, 0U, nullptr, 0U, nullptr, 0U) == NVAPI_OK);
    }

    SECTION("CreateCubinComputeShader/CreateCubinComputeShaderWithName with failure inside DXVK returns error") {
        ALLOW_CALL(device, CreateCubinComputeShaderWithNameNVX(_, _, _, _, _, _, _))
            .RETURN(false);

        NVDX_ObjectHandle objhandle;
        REQUIRE(NvAPI_D3D11_CreateCubinComputeShaderWithName(static_cast<ID3D11Device*>(&device), "X", 1U, 0U, 0U, 0U, "foo", &objhandle) == NVAPI_ERROR);
        REQUIRE(NvAPI_D3D11_CreateCubinComputeShader(static_cast<ID3D11Device*>(&device), "X", 1U, 0U, 0U, 0U, &objhandle) == NVAPI_ERROR);
    }

    SECTION("DestroyCubinComputeShader returns OK and treats handle as a COM object and releases it") {
        // it's part of our contract with DXVK that cuda shader handles are really COM objects
        UnknownMock unknown;
        REQUIRE_CALL(unknown, Release())
            .TIMES(1)
            .RETURN(42);

        auto handle = reinterpret_cast<NVDX_ObjectHandle>(&unknown);
        REQUIRE(NvAPI_D3D11_DestroyCubinComputeShader(static_cast<ID3D11Device*>(&device), handle) == NVAPI_OK);
    }

    SECTION("DestroyCubinComputeShader returns error on a NULL or NVDX_OBJECT_NONE handle") {
        // this also checks the assumption that NVDX_OBJECT_NONE casts <-> NULL, which should forever be true but will break some stuff in subtle ways if not, so ¯\_(ツ)_/¯
        REQUIRE(NVDX_OBJECT_NONE == reinterpret_cast<NVDX_ObjectHandle>(NULL /* not nullptr which is cast-proofed */));
        REQUIRE(reinterpret_cast<void*>(NVDX_OBJECT_NONE) == nullptr);
        REQUIRE(NvAPI_D3D11_DestroyCubinComputeShader(static_cast<ID3D11Device*>(&device), nullptr) == NVAPI_ERROR);
        REQUIRE(NvAPI_D3D11_DestroyCubinComputeShader(static_cast<ID3D11Device*>(&device), NVDX_OBJECT_NONE) == NVAPI_ERROR);
    }

    SECTION("IsFatbinPTXSupported returns OK") {
        bool supported = false;
        REQUIRE(NvAPI_D3D11_IsFatbinPTXSupported(static_cast<ID3D11Device*>(&device), &supported) == NVAPI_OK);
        REQUIRE(supported == true);
    }

    SECTION("CreateUnorderedAccessView/CreateShaderResourceView returns OK") {
        D3D11BufferMock resource;
        D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
        ID3D11UnorderedAccessView* pUAV;
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
        ID3D11ShaderResourceView* srv;
        NvU32 handle;
        REQUIRE_CALL(device, CreateUnorderedAccessViewAndGetDriverHandleNVX(&resource, &uavDesc, &pUAV, reinterpret_cast<uint32_t*>(&handle)))
            .TIMES(1)
            .RETURN(true);
        REQUIRE_CALL(device, CreateShaderResourceViewAndGetDriverHandleNVX(&resource, &srvDesc, &srv, reinterpret_cast<uint32_t*>(&handle)))
            .TIMES(1)
            .RETURN(true);

        REQUIRE(NvAPI_D3D11_CreateUnorderedAccessView(static_cast<ID3D11Device*>(&device), &resource, &uavDesc, &pUAV, &handle) == NVAPI_OK);
        REQUIRE(NvAPI_D3D11_CreateShaderResourceView(static_cast<ID3D11Device*>(&device), &resource, &srvDesc, &srv, &handle) == NVAPI_OK);
    }

    SECTION("GetResourceHandle returns OK") {
        D3D11BufferMock resource;
        NVDX_ObjectHandle handle;
        // Test device call twice to ensure correct reference counting when hitting the device cache
        REQUIRE(NvAPI_D3D11_GetResourceHandle(static_cast<ID3D11Device*>(&device), &resource, &handle) == NVAPI_OK);
        REQUIRE(NvAPI_D3D11_GetResourceHandle(static_cast<ID3D11Device*>(&device), &resource, &handle) == NVAPI_OK);
        // While the handles returned by NVAPI are opaque with unspecified values, our interaction with DXVK *requires* that we implement them as a simple recast
        REQUIRE(reinterpret_cast<void*>(handle) == reinterpret_cast<void*>(&resource));
    }

    SECTION("GetResourceHandle with NULL argument returns InvalidArgument") {
        D3D11BufferMock resource;
        NVDX_ObjectHandle handle;
        REQUIRE(NvAPI_D3D11_GetResourceHandle(nullptr, &resource, &handle) == NVAPI_INVALID_ARGUMENT);
        REQUIRE(NvAPI_D3D11_GetResourceHandle(static_cast<ID3D11Device*>(&device), nullptr, &handle) == NVAPI_INVALID_ARGUMENT);
        REQUIRE(NvAPI_D3D11_GetResourceHandle(static_cast<ID3D11Device*>(&device), &resource, nullptr) == NVAPI_INVALID_ARGUMENT);
    }

    SECTION("GetResourceGPUVirtualAddress/GetResourceGPUVirtualAddressEx returns OK") {
        D3D11BufferMock resource;
        auto handle = reinterpret_cast<NVDX_ObjectHandle>(&resource);
        NvU64 vaStart;
        NV_GET_GPU_VIRTUAL_ADDRESS_V1 params{};
        params.version = NV_GET_GPU_VIRTUAL_ADDRESS_VER1;
        params.hResource = handle;
        REQUIRE_CALL(device, GetResourceHandleGPUVirtualAddressAndSizeNVX(static_cast<void*>(handle), _, _))
            .SIDE_EFFECT({
                *_2 = 64UL;
                *_3 = 128UL;
            })
            .TIMES(2)
            .RETURN(true);

        REQUIRE(NvAPI_D3D11_GetResourceGPUVirtualAddress(static_cast<ID3D11Device*>(&device), handle, &vaStart) == NVAPI_OK);
        REQUIRE(vaStart == 64UL);
        REQUIRE(NvAPI_D3D11_GetResourceGPUVirtualAddressEx(static_cast<ID3D11Device*>(&device), &params) == NVAPI_OK);
        REQUIRE(params.gpuVAStart == 64UL);
        REQUIRE(params.gpuVASize == 128UL);
    }

    SECTION("GetResourceGPUVirtualAddressEx with unknown struct version returns incompatible-struct-version") {
        NV_GET_GPU_VIRTUAL_ADDRESS params{};
        params.version = NV_GET_GPU_VIRTUAL_ADDRESS_VER1 + 1;
        REQUIRE(NvAPI_D3D11_GetResourceGPUVirtualAddressEx(static_cast<ID3D11Device*>(&device), &params) == NVAPI_INCOMPATIBLE_STRUCT_VERSION);
    }

    SECTION("GetResourceGPUVirtualAddressEx with current struct version returns not incompatible-struct-version") {
        // This test should fail when a header update provides a newer not yet implemented struct version
        NV_GET_GPU_VIRTUAL_ADDRESS params{};
        params.version = NV_GET_GPU_VIRTUAL_ADDRESS_VER;
        REQUIRE(NvAPI_D3D11_GetResourceGPUVirtualAddressEx(static_cast<ID3D11Device*>(&device), &params) != NVAPI_INCOMPATIBLE_STRUCT_VERSION);
    }

    SECTION("CreateSamplerState returns OK") {
        NvU32 handle;
        D3D11_SAMPLER_DESC samplerDesc{};
        ID3D11SamplerState* samplerState;
        REQUIRE_CALL(device, CreateSamplerStateAndGetDriverHandleNVX(&samplerDesc, &samplerState, reinterpret_cast<uint32_t*>(&handle)))
            .TIMES(1)
            .RETURN(true);

        REQUIRE(NvAPI_D3D11_CreateSamplerState(static_cast<ID3D11Device*>(&device), &samplerDesc, &samplerState, &handle) == NVAPI_OK);
    }

    SECTION("GetCudaTextureObject returns OK") {
        NvU32 handle;
        REQUIRE_CALL(device, GetCudaTextureObjectNVX(0x1U, 0x2U, reinterpret_cast<uint32_t*>(&handle)))
            .TIMES(1)
            .RETURN(true);

        REQUIRE(NvAPI_D3D11_GetCudaTextureObject(static_cast<ID3D11Device*>(&device), 0x1U, 0x2U, &handle) == NVAPI_OK);
    }

    CHECK(deviceRefCount == 0);
    CHECK(deviceRefCount == 0);
}

TEST_CASE("D3D11 MultiGPU methods succeed", "[.d3d11]") {
    auto dxgiFactory = std::make_unique<DXGIDxvkFactoryMock>();
    auto vk = std::make_unique<VkMock>();
    auto nvml = std::make_unique<NvmlMock>();
    DXGIDxvkAdapterMock* adapter = CreateDXGIDxvkAdapterMock();
    DXGIOutput6Mock* output = CreateDXGIOutput6Mock();

    auto e = ConfigureDefaultTestEnvironment(*dxgiFactory, *vk, *nvml, *adapter, *output);

    SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));
    REQUIRE(NvAPI_Initialize() == NVAPI_OK);

    SECTION("MultiGPU_GetCaps (V1) returns OK") {
        NV_MULTIGPU_CAPS_V1 multiGPUCaps{};

        REQUIRE(NvAPI_D3D11_MultiGPU_GetCaps(reinterpret_cast<PNV_MULTIGPU_CAPS>(&multiGPUCaps)) == NVAPI_OK);
        REQUIRE(multiGPUCaps.multiGPUVersion == 3);
        REQUIRE(multiGPUCaps.nTotalGPUs == 1);
        REQUIRE(multiGPUCaps.nSLIGPUs == 0);
    }

    SECTION("MultiGPU_GetCaps (V2) returns OK") {
        NV_MULTIGPU_CAPS multiGPUCaps{};
        multiGPUCaps.version = NV_MULTIGPU_CAPS_VER2;

        REQUIRE(NvAPI_D3D11_MultiGPU_GetCaps(&multiGPUCaps) == NVAPI_OK);
        REQUIRE(multiGPUCaps.multiGPUVersion == 3);
        REQUIRE(multiGPUCaps.nTotalGPUs == 1);
        REQUIRE(multiGPUCaps.nSLIGPUs == 0);
    }

    SECTION("MultiGPU_Init returns OK") {
        REQUIRE(NvAPI_D3D11_MultiGPU_Init(true) == NVAPI_OK);
    }
}
