#include "nvapi_tests_private.h"
#include "resource_factory_util.h"
#include "nvapi_sysinfo_mocks.h"
#include "nvapi_d3d12_mocks.h"

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

    ALLOW_CALL(commandList, QueryInterface(__uuidof(ID3D12GraphicsCommandList1), _))
        .LR_SIDE_EFFECT(*_2 = static_cast<ID3D12GraphicsCommandList1*>(&commandList))
        .LR_SIDE_EFFECT(commandListRefCount++)
        .RETURN(S_OK);
    ALLOW_CALL(commandList, QueryInterface(ID3D12GraphicsCommandListExt::guid, _))
        .LR_SIDE_EFFECT(*_2 = static_cast<ID3D12GraphicsCommandListExt*>(&commandList))
        .LR_SIDE_EFFECT(commandListRefCount++)
        .RETURN(S_OK);
    ALLOW_CALL(commandList, QueryInterface(ID3D12GraphicsCommandListExt1::guid, _))
        .LR_SIDE_EFFECT(*_2 = static_cast<ID3D12GraphicsCommandListExt*>(&commandList))
        .LR_SIDE_EFFECT(commandListRefCount++)
        .RETURN(S_OK);
    ALLOW_CALL(commandList, AddRef())
        .LR_SIDE_EFFECT(commandListRefCount++)
        .RETURN(commandListRefCount);
    ALLOW_CALL(commandList, Release())
        .LR_SIDE_EFFECT(commandListRefCount--)
        .RETURN(commandListRefCount);

    SECTION("CreateGraphicsPipelineState for other than SetDepthBounds returns not-supported") {
        FORBID_CALL(device, CreateGraphicsPipelineState(_, _, _));

        // See https://developer.nvidia.com/unlocking-gpu-intrinsics-hlsl how to use NvAPI_D3D12_CreateGraphicsPipelineState
        auto desc = D3D12_GRAPHICS_PIPELINE_STATE_DESC{};
        NVAPI_D3D12_PSO_SET_SHADER_EXTENSION_SLOT_DESC extensionDesc;
        extensionDesc.baseVersion = NV_PSO_EXTENSION_DESC_VER;
        extensionDesc.psoExtension = NV_PSO_SET_SHADER_EXTNENSION_SLOT_AND_SPACE;
        extensionDesc.version = NV_SET_SHADER_EXTENSION_SLOT_DESC_VER;
        const NVAPI_D3D12_PSO_EXTENSION_DESC* extensions[] = {&extensionDesc};
        ID3D12PipelineState* pipelineState = nullptr;
        REQUIRE(NvAPI_D3D12_CreateGraphicsPipelineState(&device, &desc, 1, extensions, &pipelineState) == NVAPI_NOT_SUPPORTED);
    }

    SECTION("D3D12 methods without VKD3D-Proton return error") {
        ALLOW_CALL(device, QueryInterface(ID3D12DeviceExt::guid, _))
            .RETURN(E_NOINTERFACE);
        ALLOW_CALL(commandList, QueryInterface(ID3D12GraphicsCommandListExt::guid, _))
            .RETURN(E_NOINTERFACE);
        ALLOW_CALL(commandList, QueryInterface(ID3D12GraphicsCommandListExt1::guid, _))
            .RETURN(E_NOINTERFACE);

        FORBID_CALL(device, CreateCubinComputeShaderWithName(_, _, _, _, _, _, _));
        FORBID_CALL(device, DestroyCubinComputeShader(_));
        FORBID_CALL(device, GetCudaTextureObject(_, _, _));
        FORBID_CALL(device, GetCudaSurfaceObject(_, _));
        FORBID_CALL(device, CaptureUAVInfo(_));
        FORBID_CALL(commandList, LaunchCubinShader(_, _, _, _, _, _));
        FORBID_CALL(commandList, LaunchCubinShaderEx(_, _, _, _, _, _, _, _, _));

        REQUIRE(NvAPI_D3D12_CreateCubinComputeShaderWithName(static_cast<ID3D12Device*>(&device), nullptr, 0, 0, 0, 0, "shader_name", nullptr) == NVAPI_ERROR);
        REQUIRE(NvAPI_D3D12_CreateCubinComputeShader(static_cast<ID3D12Device*>(&device), nullptr, 0, 0, 0, 0, nullptr) == NVAPI_ERROR);
        REQUIRE(NvAPI_D3D12_DestroyCubinComputeShader(static_cast<ID3D12Device*>(&device), nullptr) == NVAPI_ERROR);

        D3D12_CPU_DESCRIPTOR_HANDLE srvHandle;
        REQUIRE(NvAPI_D3D12_GetCudaTextureObject(static_cast<ID3D12Device*>(&device), srvHandle, srvHandle, nullptr) == NVAPI_ERROR);
        REQUIRE(NvAPI_D3D12_GetCudaSurfaceObject(static_cast<ID3D12Device*>(&device), srvHandle, nullptr) == NVAPI_ERROR);

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
        REQUIRE(NvAPI_D3D12_GetCudaTextureObject(static_cast<ID3D12Device*>(&device), srvHandle, srvHandle, nullptr) == NVAPI_ERROR);
        REQUIRE(NvAPI_D3D12_GetCudaSurfaceObject(static_cast<ID3D12Device*>(&device), srvHandle, nullptr) == NVAPI_ERROR);

        REQUIRE(NvAPI_D3D12_CaptureUAVInfo(static_cast<ID3D12Device*>(&device), nullptr) == NVAPI_ERROR);
        bool isPTXSupported;
        REQUIRE(NvAPI_D3D12_IsFatbinPTXSupported(static_cast<ID3D12Device*>(&device), &isPTXSupported) == NVAPI_ERROR);
        REQUIRE(deviceRefCount == 0);
        REQUIRE(commandListRefCount == 0);
    }

    // Test failing scenarios first because caches won't be reset between tests (we don't cache negatives)

    SECTION("IsNvShaderExtnOpCodeSupported returns OK") {
        auto supported = true;
        REQUIRE(NvAPI_D3D12_IsNvShaderExtnOpCodeSupported(&device, 1U, &supported) == NVAPI_OK);
        REQUIRE(supported == false);
    }

    SECTION("EnumerateMetaCommands returns OK") {
        NvU32 count = 0U;
        NVAPI_META_COMMAND_DESC descs{};
        REQUIRE(NvAPI_D3D12_EnumerateMetaCommands(&device, &count, &descs) == NVAPI_NOT_SUPPORTED);
    }

    SECTION("GetGraphicsCapabilities succeeds") {
        auto dxgiFactory = std::make_unique<DXGIFactory1Mock>();
        auto vulkan = std::make_unique<VulkanMock>();
        auto nvml = std::make_unique<NvmlMock>();
        auto lfx = std::make_unique<LfxMock>();
        DXGIDxvkAdapterMock adapter;
        DXGIOutput6Mock output;
        auto luid = new LUID{};

        auto e = ConfigureDefaultTestEnvironment(*dxgiFactory, *vulkan, *nvml, *lfx, adapter, output);

        ALLOW_CALL(device, QueryInterface(__uuidof(ID3D12Device), _))
            .LR_SIDE_EFFECT(*_2 = static_cast<ID3D12Device*>(&device))
            .LR_SIDE_EFFECT(deviceRefCount++)
            .RETURN(S_OK);
        ALLOW_CALL(device, AddRef())
            .LR_SIDE_EFFECT(deviceRefCount++)
            .RETURN(deviceRefCount);
        ALLOW_CALL(device, Release())
            .LR_SIDE_EFFECT(deviceRefCount--)
            .RETURN(deviceRefCount);

        ALLOW_CALL(device, GetAdapterLuid(_))
            .LR_RETURN(luid);

        SECTION("GetGraphicsCapabilities without matching adapter returns OK with sm_0") {
            SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml), std::move(lfx));
            REQUIRE(NvAPI_Initialize() == NVAPI_OK);

            NV_D3D12_GRAPHICS_CAPS graphicsCaps{};
            REQUIRE(NvAPI_D3D12_GetGraphicsCapabilities(static_cast<ID3D12Device*>(&device), NV_D3D12_GRAPHICS_CAPS_VER1, &graphicsCaps) == NVAPI_OK);
            REQUIRE(graphicsCaps.bFastUAVClearSupported == true);
            REQUIRE(graphicsCaps.majorSMVersion == 0);
            REQUIRE(graphicsCaps.majorSMVersion == 0);
            REQUIRE(deviceRefCount == 0);
        }

        SECTION("GetGraphicsCapabilities returns OK with valid SM") {
            struct Data {
                VkDriverId driverId;
                uint32_t deviceId;
                std::string extensionName;
                uint16_t expectedMajorSMVersion;
                uint16_t expectedMinorSMVersion;
            };
            auto args = GENERATE(
                Data{VK_DRIVER_ID_NVIDIA_PROPRIETARY, 0x2600, VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME, 8, 9},
                Data{VK_DRIVER_ID_NVIDIA_PROPRIETARY, 0x2000, VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME, 8, 6},
                Data{VK_DRIVER_ID_NVIDIA_PROPRIETARY, 0x2000, VK_NV_SHADING_RATE_IMAGE_EXTENSION_NAME, 7, 5},
                Data{VK_DRIVER_ID_NVIDIA_PROPRIETARY, 0x2000, VK_NVX_IMAGE_VIEW_HANDLE_EXTENSION_NAME, 7, 0},
                Data{VK_DRIVER_ID_NVIDIA_PROPRIETARY, 0x2000, VK_NV_CLIP_SPACE_W_SCALING_EXTENSION_NAME, 6, 0},
                Data{VK_DRIVER_ID_NVIDIA_PROPRIETARY, 0x2000, VK_NV_VIEWPORT_ARRAY2_EXTENSION_NAME, 5, 0},
                Data{VK_DRIVER_ID_AMD_OPEN_SOURCE, 0x2000, VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME, 0, 0},
                Data{VK_DRIVER_ID_NVIDIA_PROPRIETARY, 0x2000, "ext", 0, 0});

            ::SetEnvironmentVariableA("DXVK_NVAPI_ALLOW_OTHER_DRIVERS", "1");

            luid->HighPart = 0x00000002;
            luid->LowPart = 0x00000001;

            ALLOW_CALL(*vulkan, GetDeviceExtensions(_, _)) // NOLINT(bugprone-use-after-move)
                .RETURN(std::set<std::string>{VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME, args.extensionName});
            ALLOW_CALL(*vulkan, GetPhysicalDeviceProperties2(_, _, _)) // NOLINT(bugprone-use-after-move)
                .SIDE_EFFECT(
                    ConfigureGetPhysicalDeviceProperties2(_3,
                        [&args](auto props, auto idProps, auto pciBusInfoProps, auto driverProps, auto fragmentShadingRateProps) {
                            auto luid = LUID{0x00000001, 0x00000002};
                            memcpy(&idProps->deviceLUID, &luid, sizeof(luid));
                            idProps->deviceLUIDValid = VK_TRUE;
                            driverProps->driverID = args.driverId;
                            props->deviceID = args.deviceId;
                            if (args.extensionName == VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME)
                                fragmentShadingRateProps->primitiveFragmentShadingRateWithMultipleViewports = VK_TRUE;
                        }));

            SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml), std::move(lfx));
            REQUIRE(NvAPI_Initialize() == NVAPI_OK);

            NV_D3D12_GRAPHICS_CAPS graphicsCaps;
            REQUIRE(NvAPI_D3D12_GetGraphicsCapabilities(static_cast<ID3D12Device*>(&device), NV_D3D12_GRAPHICS_CAPS_VER1, &graphicsCaps) == NVAPI_OK);
            REQUIRE(graphicsCaps.bFastUAVClearSupported == true);
            REQUIRE(graphicsCaps.majorSMVersion == args.expectedMajorSMVersion);
            REQUIRE(graphicsCaps.minorSMVersion == args.expectedMinorSMVersion);
            REQUIRE(deviceRefCount == 0);

            ::SetEnvironmentVariableA("DXVK_NVAPI_ALLOW_OTHER_DRIVERS", "");
        }

        SECTION("GetGraphicsCapabilities with unknown struct version returns incompatible-struct-version") {
            SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml), std::move(lfx));
            REQUIRE(NvAPI_Initialize() == NVAPI_OK);

            NV_D3D12_GRAPHICS_CAPS graphicsCaps{};
            REQUIRE(NvAPI_D3D12_GetGraphicsCapabilities(static_cast<ID3D12Device*>(&device), NV_D3D12_GRAPHICS_CAPS_VER1 + 1, &graphicsCaps) == NVAPI_INCOMPATIBLE_STRUCT_VERSION);
        }

        SECTION("GetGraphicsCapabilities with current struct version returns not incompatible-struct-version") {
            // This test should fail when a header update provides a newer not yet implemented struct version
            SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml), std::move(lfx));
            REQUIRE(NvAPI_Initialize() == NVAPI_OK);

            NV_D3D12_GRAPHICS_CAPS graphicsCaps{};
            REQUIRE(NvAPI_D3D12_GetGraphicsCapabilities(static_cast<ID3D12Device*>(&device), NV_D3D12_GRAPHICS_CAPS_VER, &graphicsCaps) != NVAPI_INCOMPATIBLE_STRUCT_VERSION);
        }

        delete luid;
    }

    SECTION("CreateGraphicsPipelineState succeeds") {
        SECTION("CreateGraphicsPipelineState for SetDepthBounds returns OK") {
            // See https://developer.nvidia.com/unlocking-gpu-intrinsics-hlsl how to use NvAPI_D3D12_CreateGraphicsPipelineState
            auto desc = D3D12_GRAPHICS_PIPELINE_STATE_DESC{};
            NVAPI_D3D12_PSO_ENABLE_DEPTH_BOUND_TEST_DESC_V1 extensionDesc{};
            extensionDesc.baseVersion = NV_PSO_EXTENSION_DESC_VER_1;
            extensionDesc.psoExtension = NV_PSO_ENABLE_DEPTH_BOUND_TEST_EXTENSION;
            extensionDesc.version = NV_ENABLE_DEPTH_BOUND_TEST_PSO_EXTENSION_DESC_VER;
            extensionDesc.EnableDBT = true;
            const NVAPI_D3D12_PSO_EXTENSION_DESC* extensions[] = {&extensionDesc};
            ID3D12PipelineState* pipelineState = nullptr;
            REQUIRE_CALL(device, CreateGraphicsPipelineState(&desc, __uuidof(ID3D12PipelineState), reinterpret_cast<void**>(&pipelineState)))
                .RETURN(S_OK);

            REQUIRE(NvAPI_D3D12_CreateGraphicsPipelineState(&device, &desc, 1, extensions, &pipelineState) == NVAPI_OK);
            REQUIRE(deviceRefCount == 0);
            REQUIRE(commandListRefCount == 0);
        }

        SECTION("CreateGraphicsPipelineState with unknown struct version returns incompatible-struct-version") {
            auto desc = D3D12_GRAPHICS_PIPELINE_STATE_DESC{};
            NVAPI_D3D12_PSO_ENABLE_DEPTH_BOUND_TEST_DESC extensionDesc;
            extensionDesc.baseVersion = NV_PSO_EXTENSION_DESC_VER_1 + 1;
            const NVAPI_D3D12_PSO_EXTENSION_DESC* extensions[] = {&extensionDesc};
            ID3D12PipelineState* pipelineState = nullptr;
            REQUIRE(NvAPI_D3D12_CreateGraphicsPipelineState(&device, &desc, 1, extensions, &pipelineState) == NVAPI_INCOMPATIBLE_STRUCT_VERSION);
        }

        SECTION("CreateGraphicsPipelineState with current struct version returns not incompatible-struct-version") {
            // This test should fail when a header update provides a newer not yet implemented struct version
            ALLOW_CALL(device, CreateGraphicsPipelineState(_, _, _))
                .RETURN(S_OK);

            auto desc = D3D12_GRAPHICS_PIPELINE_STATE_DESC{};
            NVAPI_D3D12_PSO_ENABLE_DEPTH_BOUND_TEST_DESC extensionDesc;
            extensionDesc.baseVersion = NV_PSO_EXTENSION_DESC_VER;
            const NVAPI_D3D12_PSO_EXTENSION_DESC* extensions[] = {&extensionDesc};
            ID3D12PipelineState* pipelineState = nullptr;
            REQUIRE(NvAPI_D3D12_CreateGraphicsPipelineState(&device, &desc, 1, extensions, &pipelineState) != NVAPI_INCOMPATIBLE_STRUCT_VERSION);
        }
    }

    SECTION("SetDepthBoundsTestValues returns OK") {
        auto min = 0.4f;
        auto max = 0.7f;
        REQUIRE_CALL(commandList, OMSetDepthBounds(min, max));

        REQUIRE(NvAPI_D3D12_SetDepthBoundsTestValues(&commandList, min, max) == NVAPI_OK);
        REQUIRE(deviceRefCount == 0);
        REQUIRE(commandListRefCount == 0);
    }

    SECTION("CreateCubinComputeShader returns OK") {
        const void* cubinData = nullptr;
        auto cubinSize = 2U;
        auto blockX = 3U;
        auto blockY = 4U;
        auto blockZ = 5U;
        auto shaderHandle = reinterpret_cast<D3D12_CUBIN_DATA_HANDLE*>(0x912122);
        auto handle = &shaderHandle;
        REQUIRE_CALL(device, CreateCubinComputeShaderWithName(cubinData, cubinSize, blockX, blockY, blockZ, _, handle))
            .WITH(_6 == std::string(""))
            .RETURN(S_OK)
            .TIMES(1);

        REQUIRE(NvAPI_D3D12_CreateCubinComputeShader(static_cast<ID3D12Device*>(&device), cubinData, cubinSize, blockX, blockY, blockZ, reinterpret_cast<NVDX_ObjectHandle*>(handle)) == NVAPI_OK);
        REQUIRE(deviceRefCount == 0);
        REQUIRE(commandListRefCount == 0);
    }

    SECTION("CreateCubinComputeShaderWithName returns OK") {
        const void* cubinData = nullptr;
        auto cubinSize = 2U;
        auto blockX = 3U;
        auto blockY = 4U;
        auto blockZ = 5U;
        auto shaderName = "shader";
        auto shaderHandle = reinterpret_cast<D3D12_CUBIN_DATA_HANDLE*>(0x912122);
        auto handle = &shaderHandle;
        REQUIRE_CALL(device, CreateCubinComputeShaderWithName(cubinData, cubinSize, blockX, blockY, blockZ, shaderName, handle))
            .RETURN(S_OK)
            .TIMES(1);

        REQUIRE(NvAPI_D3D12_CreateCubinComputeShaderWithName(static_cast<ID3D12Device*>(&device), cubinData, cubinSize, blockX, blockY, blockZ, shaderName, reinterpret_cast<NVDX_ObjectHandle*>(handle)) == NVAPI_OK);
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

        REQUIRE(NvAPI_D3D12_GetCudaTextureObject(static_cast<ID3D12Device*>(&device), srvHandle, samplerHandle, nullptr) == NVAPI_OK);
        REQUIRE(NvAPI_D3D12_GetCudaSurfaceObject(static_cast<ID3D12Device*>(&device), srvHandle, nullptr) == NVAPI_OK);
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
        auto smemSize = 0U;
        const void* params = nullptr;
        auto paramSize = 4U;
        const void* rawParam = nullptr;
        auto rawParamCount = 0U;
        REQUIRE_CALL(commandList, LaunchCubinShaderEx(reinterpret_cast<D3D12_CUBIN_DATA_HANDLE*>(pShader), blockX, blockY, blockZ, smemSize, params, paramSize, rawParam, rawParamCount))
            .RETURN(S_OK)
            .TIMES(1);

        REQUIRE(NvAPI_D3D12_LaunchCubinShader(static_cast<ID3D12GraphicsCommandList*>(&commandList), pShader, blockX, blockY, blockZ, params, paramSize) == NVAPI_OK);
        REQUIRE(deviceRefCount == 0);
        REQUIRE(commandListRefCount == 0);
    }

    SECTION("Create and Launch CuBIN with SMEM returns OK") {
        const void* cubinData = nullptr;
        auto cubinSize = 2U;
        auto blockX = 3U;
        auto blockY = 4U;
        auto blockZ = 5U;
        auto smemSize = 6U;
        auto shaderName = "shader";
        auto shaderHandle = reinterpret_cast<D3D12_CUBIN_DATA_HANDLE*>(0x912122);
        auto handle = &shaderHandle;
        const void* params = nullptr;
        auto paramSize = 7U;
        const void* rawParam = nullptr;
        auto rawParamCount = 0U;
        REQUIRE_CALL(device, CreateCubinComputeShaderWithName(cubinData, cubinSize, blockX, blockY, blockZ, shaderName, handle))
            .RETURN(S_OK)
            .TIMES(1);
        REQUIRE_CALL(commandList, LaunchCubinShaderEx(shaderHandle, blockX, blockY, blockZ, smemSize, params, paramSize, rawParam, rawParamCount))
            .RETURN(S_OK)
            .TIMES(1);

        REQUIRE(NvAPI_D3D12_CreateCubinComputeShaderEx(static_cast<ID3D12Device*>(&device), cubinData, cubinSize, blockX, blockY, blockZ, smemSize, shaderName, reinterpret_cast<NVDX_ObjectHandle*>(handle)) == NVAPI_OK);
        REQUIRE(NvAPI_D3D12_LaunchCubinShader(static_cast<ID3D12GraphicsCommandList*>(&commandList), reinterpret_cast<NVDX_ObjectHandle>(shaderHandle), blockX, blockY, blockZ, params, paramSize) == NVAPI_OK);
        REQUIRE(deviceRefCount == 0);
        REQUIRE(commandListRefCount == 0);
    }

    SECTION("Launch CuBIN without ID3D12GraphicsCommandListExt1 returns OK") {
        D3D12Vkd3dGraphicsCommandListMock commandList0;
        ALLOW_CALL(commandList0, QueryInterface(__uuidof(ID3D12GraphicsCommandList1), _))
            .LR_SIDE_EFFECT(*_2 = static_cast<ID3D12GraphicsCommandList1*>(&commandList0))
            .LR_SIDE_EFFECT(commandListRefCount++)
            .RETURN(S_OK);
        ALLOW_CALL(commandList0, QueryInterface(ID3D12GraphicsCommandListExt::guid, _))
            .LR_SIDE_EFFECT(*_2 = static_cast<ID3D12GraphicsCommandListExt*>(&commandList0))
            .LR_SIDE_EFFECT(commandListRefCount++)
            .RETURN(S_OK);
        ALLOW_CALL(commandList0, QueryInterface(ID3D12GraphicsCommandListExt1::guid, _))
            .RETURN(E_NOINTERFACE);
        ALLOW_CALL(commandList0, AddRef())
            .LR_SIDE_EFFECT(commandListRefCount++)
            .RETURN(commandListRefCount);
        ALLOW_CALL(commandList0, Release())
            .LR_SIDE_EFFECT(commandListRefCount--)
            .RETURN(commandListRefCount);

        auto shaderHandle = reinterpret_cast<D3D12_CUBIN_DATA_HANDLE*>(0xbadcf00d);
        auto blockX = 1U;
        auto blockY = 2U;
        auto blockZ = 3U;
        const void* params = nullptr;
        auto paramSize = 4U;

        REQUIRE_CALL(commandList0, LaunchCubinShader(shaderHandle, blockX, blockY, blockZ, params, paramSize))
            .RETURN(S_OK)
            .TIMES(1);
        FORBID_CALL(commandList0, LaunchCubinShaderEx(_, _, _, _, _, _, _, _, _));

        REQUIRE(NvAPI_D3D12_LaunchCubinShader(static_cast<ID3D12GraphicsCommandList*>(&commandList0), reinterpret_cast<NVDX_ObjectHandle>(shaderHandle), blockX, blockY, blockZ, params, paramSize) == NVAPI_OK);
        REQUIRE(deviceRefCount == 0);
        REQUIRE(commandListRefCount == 0);
    }

    SECTION("GetRaytracingCaps returns OK and claims that thread reordering is not supported") {
        NVAPI_D3D12_RAYTRACING_THREAD_REORDERING_CAPS caps;
        REQUIRE(NvAPI_D3D12_GetRaytracingCaps(static_cast<ID3D12Device*>(&device), NVAPI_D3D12_RAYTRACING_CAPS_TYPE_THREAD_REORDERING, &caps, sizeof(caps)) == NVAPI_OK);
        REQUIRE(caps == NVAPI_D3D12_RAYTRACING_THREAD_REORDERING_CAP_NONE);
    }

    SECTION("GetRaytracingCaps returns OK and claims that Opacity Micromap is not supported") {
        NVAPI_D3D12_RAYTRACING_OPACITY_MICROMAP_CAPS caps;
        REQUIRE(NvAPI_D3D12_GetRaytracingCaps(static_cast<ID3D12Device*>(&device), NVAPI_D3D12_RAYTRACING_CAPS_TYPE_OPACITY_MICROMAP, &caps, sizeof(caps)) == NVAPI_OK);
        REQUIRE(caps == NVAPI_D3D12_RAYTRACING_OPACITY_MICROMAP_CAP_NONE);
    }

    SECTION("GetRaytracingAccelerationStructurePrebuildInfoEx succeeds") {
        SECTION("GetRaytracingAccelerationStructurePrebuildInfoEx returns OK") {
            NVAPI_D3D12_RAYTRACING_GEOMETRY_DESC_EX geometryDescEx{};
            NVAPI_D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS_EX desc{};
            D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info{};
            NVAPI_GET_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO_EX_PARAMS params{};
            params.version = NVAPI_GET_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO_EX_PARAMS_VER1;
            params.pDesc = &desc;
            params.pInfo = &info;

            D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS d3d12Desc{};
            ALLOW_CALL(device, GetRaytracingAccelerationStructurePrebuildInfo(_, &info))
                .LR_SIDE_EFFECT(d3d12Desc = *_1);

            SECTION("GetRaytracingAccelerationStructurePrebuildInfoEx with TLAS") {
                desc.type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
                desc.instanceDescs = D3D12_GPU_VIRTUAL_ADDRESS{};

                REQUIRE(NvAPI_D3D12_GetRaytracingAccelerationStructurePrebuildInfoEx(static_cast<ID3D12Device5*>(&device), &params) == NVAPI_OK);
                REQUIRE(d3d12Desc.InstanceDescs == desc.instanceDescs);
            }

            SECTION("GetRaytracingAccelerationStructurePrebuildInfoEx with BLAS for pointer array") {
                NVAPI_D3D12_RAYTRACING_GEOMETRY_DESC_EX* geometryDescExArray[] = {};
                desc.type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
                desc.descsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY_OF_POINTERS;
                desc.ppGeometryDescs = geometryDescExArray;

                REQUIRE(NvAPI_D3D12_GetRaytracingAccelerationStructurePrebuildInfoEx(static_cast<ID3D12Device5*>(&device), &params) == NVAPI_OK);
                REQUIRE(d3d12Desc.ppGeometryDescs == reinterpret_cast<const D3D12_RAYTRACING_GEOMETRY_DESC* const*>(desc.ppGeometryDescs));
            }

            SECTION("GetRaytracingAccelerationStructurePrebuildInfoEx with BLAS for array") {
                desc.type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
                desc.descsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
                desc.numDescs = 1;
                desc.pGeometryDescs = &geometryDescEx;
                desc.geometryDescStrideInBytes = sizeof(geometryDescEx);

                SECTION("GetRaytracingAccelerationStructurePrebuildInfoEx for triangles geometry") {
                    geometryDescEx.type = NVAPI_D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES_EX;
                    geometryDescEx.triangles.IndexBuffer = D3D12_GPU_VIRTUAL_ADDRESS{};

                    REQUIRE(NvAPI_D3D12_GetRaytracingAccelerationStructurePrebuildInfoEx(static_cast<ID3D12Device5*>(&device), &params) == NVAPI_OK);
                    REQUIRE(d3d12Desc.NumDescs == desc.numDescs);
                    REQUIRE(d3d12Desc.DescsLayout == desc.descsLayout);
                    REQUIRE(d3d12Desc.pGeometryDescs->Type == D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES);
                    REQUIRE(d3d12Desc.pGeometryDescs->Triangles.IndexBuffer == geometryDescEx.triangles.IndexBuffer);
                }

                SECTION("GetRaytracingAccelerationStructurePrebuildInfoEx with AABBs geometry") {
                    geometryDescEx.type = NVAPI_D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS_EX;
                    geometryDescEx.aabbs.AABBCount = 3;

                    REQUIRE(NvAPI_D3D12_GetRaytracingAccelerationStructurePrebuildInfoEx(static_cast<ID3D12Device5*>(&device), &params) == NVAPI_OK);
                    REQUIRE(d3d12Desc.NumDescs == desc.numDescs);
                    REQUIRE(d3d12Desc.DescsLayout == desc.descsLayout);
                    REQUIRE(d3d12Desc.pGeometryDescs->Type == D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS);
                    REQUIRE(d3d12Desc.pGeometryDescs->AABBs.AABBCount == geometryDescEx.aabbs.AABBCount);
                }
            }
        }

        SECTION("GetRaytracingAccelerationStructurePrebuildInfoEx with unknown struct version returns incompatible-struct-version") {
            NVAPI_GET_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO_EX_PARAMS params{};
            params.version = NVAPI_GET_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO_EX_PARAMS_VER1 + 1;
            REQUIRE(NvAPI_D3D12_GetRaytracingAccelerationStructurePrebuildInfoEx(static_cast<ID3D12Device5*>(&device), &params) == NVAPI_INCOMPATIBLE_STRUCT_VERSION);
        }

        SECTION("GetRaytracingAccelerationStructurePrebuildInfoEx with current struct version returns not incompatible-struct-version") {
            // This test should fail when a header update provides a newer not yet implemented struct version
            NVAPI_GET_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO_EX_PARAMS params{};
            params.version = NVAPI_GET_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO_EX_PARAMS_VER;
            REQUIRE(NvAPI_D3D12_GetRaytracingAccelerationStructurePrebuildInfoEx(static_cast<ID3D12Device5*>(&device), &params) != NVAPI_INCOMPATIBLE_STRUCT_VERSION);
        }
    }

    SECTION("BuildRaytracingAccelerationStructureEx succeeds") {
        SECTION("BuildRaytracingAccelerationStructureEx returns OK") {
            NVAPI_D3D12_RAYTRACING_GEOMETRY_DESC_EX geometryDescEx{};
            NVAPI_D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC_EX desc{};
            NVAPI_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_EX_PARAMS params{};
            params.version = NVAPI_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_EX_PARAMS_VER1;
            params.pDesc = &desc;

            D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC d3d12Desc{};
            ALLOW_CALL(commandList, BuildRaytracingAccelerationStructure(_, params.numPostbuildInfoDescs, params.pPostbuildInfoDescs))
                .LR_SIDE_EFFECT(d3d12Desc = *_1);

            SECTION("BuildRaytracingAccelerationStructureEx with TLAS") {
                desc.inputs.type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
                desc.inputs.instanceDescs = D3D12_GPU_VIRTUAL_ADDRESS{};

                REQUIRE(NvAPI_D3D12_BuildRaytracingAccelerationStructureEx(static_cast<ID3D12GraphicsCommandList4*>(&commandList), &params) == NVAPI_OK);
                REQUIRE(d3d12Desc.DestAccelerationStructureData == desc.destAccelerationStructureData);
                REQUIRE(d3d12Desc.Inputs.InstanceDescs == desc.inputs.instanceDescs);
                REQUIRE(d3d12Desc.SourceAccelerationStructureData == desc.sourceAccelerationStructureData);
                REQUIRE(d3d12Desc.ScratchAccelerationStructureData == desc.scratchAccelerationStructureData);
            }

            SECTION("BuildRaytracingAccelerationStructureEx with BLAS for pointer array") {
                NVAPI_D3D12_RAYTRACING_GEOMETRY_DESC_EX* geometryDescExArray[] = {};
                desc.inputs.type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
                desc.inputs.descsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY_OF_POINTERS;
                desc.inputs.ppGeometryDescs = geometryDescExArray;

                REQUIRE(NvAPI_D3D12_BuildRaytracingAccelerationStructureEx(static_cast<ID3D12GraphicsCommandList4*>(&commandList), &params) == NVAPI_OK);
                REQUIRE(d3d12Desc.DestAccelerationStructureData == desc.destAccelerationStructureData);
                REQUIRE(d3d12Desc.Inputs.ppGeometryDescs == reinterpret_cast<const D3D12_RAYTRACING_GEOMETRY_DESC* const*>(desc.inputs.ppGeometryDescs));
                REQUIRE(d3d12Desc.SourceAccelerationStructureData == desc.sourceAccelerationStructureData);
                REQUIRE(d3d12Desc.ScratchAccelerationStructureData == desc.scratchAccelerationStructureData);
            }

            SECTION("BuildRaytracingAccelerationStructureEx with BLAS for array") {
                desc.inputs.type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
                desc.inputs.descsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
                desc.inputs.numDescs = 1;
                desc.inputs.pGeometryDescs = &geometryDescEx;
                desc.inputs.geometryDescStrideInBytes = sizeof(geometryDescEx);

                SECTION("BuildRaytracingAccelerationStructureEx for triangles geometry") {
                    geometryDescEx.type = NVAPI_D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES_EX;
                    geometryDescEx.triangles.IndexBuffer = D3D12_GPU_VIRTUAL_ADDRESS{};

                    REQUIRE(NvAPI_D3D12_BuildRaytracingAccelerationStructureEx(static_cast<ID3D12GraphicsCommandList4*>(&commandList), &params) == NVAPI_OK);
                    REQUIRE(d3d12Desc.DestAccelerationStructureData == desc.destAccelerationStructureData);
                    REQUIRE(d3d12Desc.Inputs.NumDescs == desc.inputs.numDescs);
                    REQUIRE(d3d12Desc.Inputs.DescsLayout == desc.inputs.descsLayout);
                    REQUIRE(d3d12Desc.Inputs.pGeometryDescs->Type == D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES);
                    REQUIRE(d3d12Desc.Inputs.pGeometryDescs->Triangles.IndexBuffer == geometryDescEx.triangles.IndexBuffer);
                    REQUIRE(d3d12Desc.SourceAccelerationStructureData == desc.sourceAccelerationStructureData);
                    REQUIRE(d3d12Desc.ScratchAccelerationStructureData == desc.scratchAccelerationStructureData);
                }

                SECTION("BuildRaytracingAccelerationStructureEx for AABBs geometry") {
                    geometryDescEx.type = NVAPI_D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS_EX;
                    geometryDescEx.aabbs.AABBCount = 3;

                    REQUIRE(NvAPI_D3D12_BuildRaytracingAccelerationStructureEx(static_cast<ID3D12GraphicsCommandList4*>(&commandList), &params) == NVAPI_OK);
                    REQUIRE(d3d12Desc.DestAccelerationStructureData == desc.destAccelerationStructureData);
                    REQUIRE(d3d12Desc.Inputs.NumDescs == desc.inputs.numDescs);
                    REQUIRE(d3d12Desc.Inputs.DescsLayout == desc.inputs.descsLayout);
                    REQUIRE(d3d12Desc.Inputs.pGeometryDescs->Type == D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS);
                    REQUIRE(d3d12Desc.Inputs.pGeometryDescs->AABBs.AABBCount == geometryDescEx.aabbs.AABBCount);
                    REQUIRE(d3d12Desc.SourceAccelerationStructureData == desc.sourceAccelerationStructureData);
                    REQUIRE(d3d12Desc.ScratchAccelerationStructureData == desc.scratchAccelerationStructureData);
                }
            }
        }

        SECTION("BuildRaytracingAccelerationStructureEx with unknown struct version returns incompatible-struct-version") {
            NVAPI_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_EX_PARAMS params{};
            params.version = NVAPI_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_EX_PARAMS_VER1 + 1;
            REQUIRE(NvAPI_D3D12_BuildRaytracingAccelerationStructureEx(static_cast<ID3D12GraphicsCommandList4*>(&commandList), &params) == NVAPI_INCOMPATIBLE_STRUCT_VERSION);
        }

        SECTION("BuildRaytracingAccelerationStructureEx with current struct version returns not incompatible-struct-version") {
            // This test should fail when a header update provides a newer not yet implemented struct version
            NVAPI_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_EX_PARAMS params{};
            params.version = NVAPI_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_EX_PARAMS_VER;
            REQUIRE(NvAPI_D3D12_BuildRaytracingAccelerationStructureEx(static_cast<ID3D12GraphicsCommandList4*>(&commandList), &params) != NVAPI_INCOMPATIBLE_STRUCT_VERSION);
        }
    }
}
