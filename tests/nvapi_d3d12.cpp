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
        REQUIRE(NvAPI_D3D12_EnumerateMetaCommands(&device, &count, &descs) == NVAPI_OK);
        REQUIRE(count == 0);
    }

    SECTION("GetGraphicsCapabilities succeeds") {
        auto dxgiFactory = std::make_unique<DXGIFactory1Mock>();
        auto vulkan = std::make_unique<VulkanMock>();
        auto nvml = std::make_unique<NvmlMock>();
        auto lfx = std::make_unique<LfxMock>();
        DXGIDxvkAdapterMock adapter;
        DXGIOutputMock output;
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
