#include "nvapi_tests_private.h"
#include "mocks/d3d_mocks.h"
#include "mocks/d3d12_mocks.h"
#include "nvapi/resource_factory_util.h"

using namespace trompeloeil;

// previous version of this structure before the union gained a new dmmTriangles member in R535
typedef struct _NVAPI_D3D12_RAYTRACING_GEOMETRY_DESC_EX_R520 {
    NVAPI_D3D12_RAYTRACING_GEOMETRY_TYPE_EX type; //!< The type of geometry stored in the union of this structure.
    D3D12_RAYTRACING_GEOMETRY_FLAGS flags;        //!< Flags affecting how this geometry is processed by the raytracing pipeline.
    union {
        D3D12_RAYTRACING_GEOMETRY_TRIANGLES_DESC triangles;              //!< Describes triangle geometry if \c type is #NVAPI_D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES_EX.
                                                                         //!< Otherwise, this parameter is unused (space repurposed in a union).
        D3D12_RAYTRACING_GEOMETRY_AABBS_DESC aabbs;                      //!< Describes AABB geometry if \c type is #NVAPI_D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS_EX.
                                                                         //!< Otherwise, this parameter is unused (space repurposed in a union).
        NVAPI_D3D12_RAYTRACING_GEOMETRY_OMM_TRIANGLES_DESC ommTriangles; //!< Describes triangle geometry which may optionally use Opacity Micromaps, if \c type is #NVAPI_D3D12_RAYTRACING_GEOMETRY_TYPE_OMM_TRIANGLES_EX.
                                                                         //!< Otherwise, this parameter is unused (space repurposed in a union).
    };
} NVAPI_D3D12_RAYTRACING_GEOMETRY_DESC_EX_R520;

TEST_CASE("D3D12 methods succeed", "[.d3d12]") {
    D3D12Vkd3dDeviceMock device;
    D3D12Vkd3dCommandQueueMock commandQueue;
    D3DLowLatencyDeviceMock lowLatencyDevice;
    D3D12Vkd3dGraphicsCommandListMock commandList;
    auto deviceRefCount = 0;
    auto commandListRefCount = 0;
    auto commandQueueRefCount = 0;
    auto lowLatencyDeviceRefCount = 0;

    ALLOW_CALL(device, QueryInterface(__uuidof(ID3D12Device), _))
        .LR_SIDE_EFFECT(*_2 = static_cast<ID3D12Device*>(&device))
        .LR_SIDE_EFFECT(deviceRefCount++)
        .RETURN(S_OK);
    ALLOW_CALL(device, QueryInterface(__uuidof(ID3D12DeviceExt), _))
        .LR_SIDE_EFFECT(*_2 = static_cast<ID3D12DeviceExt*>(&device))
        .LR_SIDE_EFFECT(deviceRefCount++)
        .RETURN(S_OK);
    ALLOW_CALL(device, QueryInterface(__uuidof(ID3D12DeviceExt2), _))
        .RETURN(E_NOINTERFACE);
    ALLOW_CALL(device, AddRef())
        .LR_SIDE_EFFECT(deviceRefCount++)
        .RETURN(deviceRefCount);
    ALLOW_CALL(device, Release())
        .LR_SIDE_EFFECT(deviceRefCount--)
        .RETURN(deviceRefCount);

    ALLOW_CALL(device, QueryInterface(__uuidof(ID3DLowLatencyDevice), _))
        .RETURN(E_NOINTERFACE);

    ALLOW_CALL(device, GetExtensionSupport(_))
        .RETURN(true);

    ALLOW_CALL(commandList, QueryInterface(__uuidof(ID3D12GraphicsCommandList1), _))
        .LR_SIDE_EFFECT(*_2 = static_cast<ID3D12GraphicsCommandList1*>(&commandList))
        .LR_SIDE_EFFECT(commandListRefCount++)
        .RETURN(S_OK);
    ALLOW_CALL(commandList, QueryInterface(__uuidof(ID3D12GraphicsCommandListExt), _))
        .LR_SIDE_EFFECT(*_2 = static_cast<ID3D12GraphicsCommandListExt*>(&commandList))
        .LR_SIDE_EFFECT(commandListRefCount++)
        .RETURN(S_OK);
    ALLOW_CALL(commandList, QueryInterface(__uuidof(ID3D12GraphicsCommandListExt1), _))
        .LR_SIDE_EFFECT(*_2 = static_cast<ID3D12GraphicsCommandListExt*>(&commandList))
        .LR_SIDE_EFFECT(commandListRefCount++)
        .RETURN(S_OK);
    ALLOW_CALL(commandList, AddRef())
        .LR_SIDE_EFFECT(commandListRefCount++)
        .RETURN(commandListRefCount);
    ALLOW_CALL(commandList, Release())
        .LR_SIDE_EFFECT(commandListRefCount--)
        .RETURN(commandListRefCount);

    ALLOW_CALL(commandQueue, QueryInterface(__uuidof(ID3D12CommandQueue), _))
        .LR_SIDE_EFFECT(*_2 = static_cast<ID3D12CommandQueue*>(&commandQueue))
        .LR_SIDE_EFFECT(commandQueueRefCount++)
        .RETURN(S_OK);
    ALLOW_CALL(commandQueue, QueryInterface(__uuidof(ID3D12CommandQueueExt), _))
        .LR_SIDE_EFFECT(*_2 = static_cast<ID3D12CommandQueueExt*>(&commandQueue))
        .LR_SIDE_EFFECT(commandQueueRefCount++)
        .RETURN(S_OK);
    ALLOW_CALL(commandQueue, QueryInterface(__uuidof(ID3D11DeviceChild), _))
        .RETURN(E_NOINTERFACE);
    ALLOW_CALL(commandQueue, AddRef())
        .LR_SIDE_EFFECT(commandQueueRefCount++)
        .RETURN(commandQueueRefCount);
    ALLOW_CALL(commandQueue, Release())
        .LR_SIDE_EFFECT(commandQueueRefCount--)
        .RETURN(commandQueueRefCount);
    ALLOW_CALL(commandQueue, GetDevice(__uuidof(ID3D12Device), _))
        .LR_SIDE_EFFECT(*_2 = static_cast<ID3D12Device*>(&device))
        .LR_SIDE_EFFECT(deviceRefCount++)
        .RETURN(S_OK);

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
        ALLOW_CALL(device, QueryInterface(__uuidof(ID3D12DeviceExt), _))
            .RETURN(E_NOINTERFACE);
        ALLOW_CALL(commandList, QueryInterface(__uuidof(ID3D12GraphicsCommandListExt), _))
            .RETURN(E_NOINTERFACE);
        ALLOW_CALL(commandList, QueryInterface(__uuidof(ID3D12GraphicsCommandListExt1), _))
            .RETURN(E_NOINTERFACE);

        FORBID_CALL(device, CreateCubinComputeShaderWithName(_, _, _, _, _, _, _));
        FORBID_CALL(device, DestroyCubinComputeShader(_));
        FORBID_CALL(device, GetCudaTextureObject(_, _, _));
        FORBID_CALL(device, GetCudaSurfaceObject(_, _));
        FORBID_CALL(device, CaptureUAVInfo(_));
        FORBID_CALL(commandList, LaunchCubinShader(_, _, _, _, _, _));
        FORBID_CALL(commandList, LaunchCubinShaderEx(_, _, _, _, _, _, _, _, _));

        const void* cubinData = nullptr;
        NVDX_ObjectHandle handle{};
        REQUIRE(NvAPI_D3D12_CreateCubinComputeShaderWithName(static_cast<ID3D12Device*>(&device), cubinData, 0, 0, 0, 0, "shader_name", &handle) == NVAPI_NO_IMPLEMENTATION);
        REQUIRE(NvAPI_D3D12_CreateCubinComputeShader(static_cast<ID3D12Device*>(&device), cubinData, 0, 0, 0, 0, &handle) == NVAPI_NO_IMPLEMENTATION);
        REQUIRE(NvAPI_D3D12_CreateCubinComputeShaderEx(static_cast<ID3D12Device*>(&device), cubinData, 0, 0, 0, 0, 0, "shader_name", &handle) == NVAPI_NO_IMPLEMENTATION);
        REQUIRE(NvAPI_D3D12_DestroyCubinComputeShader(static_cast<ID3D12Device*>(&device), handle) == NVAPI_NO_IMPLEMENTATION);

        D3D12_CPU_DESCRIPTOR_HANDLE srvHandle;
        NvU32 textureHandle;
        NvU32 surfaceHandle;
        REQUIRE(NvAPI_D3D12_GetCudaTextureObject(static_cast<ID3D12Device*>(&device), srvHandle, srvHandle, &textureHandle) == NVAPI_NO_IMPLEMENTATION);
        REQUIRE(NvAPI_D3D12_GetCudaSurfaceObject(static_cast<ID3D12Device*>(&device), srvHandle, &surfaceHandle) == NVAPI_NO_IMPLEMENTATION);

        NVAPI_UAV_INFO info{};
        REQUIRE(NvAPI_D3D12_CaptureUAVInfo(static_cast<ID3D12Device*>(&device), &info) == NVAPI_NO_IMPLEMENTATION);
        bool isPTXSupported;
        REQUIRE(NvAPI_D3D12_IsFatbinPTXSupported(static_cast<ID3D12Device*>(&device), &isPTXSupported) == NVAPI_NO_IMPLEMENTATION);

        REQUIRE(NvAPI_D3D12_LaunchCubinShader(static_cast<ID3D12GraphicsCommandList*>(&commandList), handle, 0, 0, 0, nullptr, 0) == NVAPI_NO_IMPLEMENTATION);
    }

    SECTION("D3D12 methods without cubin extension return error") {
        ALLOW_CALL(device, GetExtensionSupport(D3D12_VK_NVX_BINARY_IMPORT))
            .RETURN(false);
        ALLOW_CALL(device, GetExtensionSupport(D3D12_VK_NVX_IMAGE_VIEW_HANDLE))
            .RETURN(false);

        FORBID_CALL(device, CreateCubinComputeShaderWithName(_, _, _, _, _, _, _));
        FORBID_CALL(device, DestroyCubinComputeShader(_));
        FORBID_CALL(device, GetCudaTextureObject(_, _, _));
        FORBID_CALL(device, GetCudaSurfaceObject(_, _));
        FORBID_CALL(device, CaptureUAVInfo(_));

        const void* cubinData = nullptr;
        NVDX_ObjectHandle handle{};
        REQUIRE(NvAPI_D3D12_CreateCubinComputeShaderWithName(static_cast<ID3D12Device*>(&device), cubinData, 0, 0, 0, 0, "shader_name", &handle) == NVAPI_NO_IMPLEMENTATION);
        REQUIRE(NvAPI_D3D12_CreateCubinComputeShader(static_cast<ID3D12Device*>(&device), cubinData, 0, 0, 0, 0, &handle) == NVAPI_NO_IMPLEMENTATION);
        REQUIRE(NvAPI_D3D12_CreateCubinComputeShaderEx(static_cast<ID3D12Device*>(&device), cubinData, 0, 0, 0, 0, 0, "shader_name", &handle) == NVAPI_NO_IMPLEMENTATION);
        REQUIRE(NvAPI_D3D12_DestroyCubinComputeShader(static_cast<ID3D12Device*>(&device), handle) == NVAPI_NO_IMPLEMENTATION);

        D3D12_CPU_DESCRIPTOR_HANDLE srvHandle;
        NvU32 textureHandle;
        NvU32 surfaceHandle;
        REQUIRE(NvAPI_D3D12_GetCudaTextureObject(static_cast<ID3D12Device*>(&device), srvHandle, srvHandle, &textureHandle) == NVAPI_NO_IMPLEMENTATION);
        REQUIRE(NvAPI_D3D12_GetCudaSurfaceObject(static_cast<ID3D12Device*>(&device), srvHandle, &surfaceHandle) == NVAPI_NO_IMPLEMENTATION);

        NVAPI_UAV_INFO info{};
        REQUIRE(NvAPI_D3D12_CaptureUAVInfo(static_cast<ID3D12Device*>(&device), &info) == NVAPI_NO_IMPLEMENTATION);
        bool isPTXSupported;
        REQUIRE(NvAPI_D3D12_IsFatbinPTXSupported(static_cast<ID3D12Device*>(&device), &isPTXSupported) == NVAPI_ERROR);
    }

    SECTION("IsNvShaderExtnOpCodeSupported returns OK") {
        auto supported = true;
        REQUIRE(NvAPI_D3D12_IsNvShaderExtnOpCodeSupported(&device, 1U, &supported) == NVAPI_OK);
        REQUIRE(supported == false);
    }

    SECTION("EnumerateMetaCommands returns OK") {
        NvU32 count = 1U;
        NVAPI_META_COMMAND_DESC descs{};
        REQUIRE(NvAPI_D3D12_EnumerateMetaCommands(&device, &count, &descs) == NVAPI_OK);
        REQUIRE(count == 0);
    }

    SECTION("GetGraphicsCapabilities succeeds") {
        auto dxgiFactory = std::make_unique<DXGIDxvkFactoryMock>();
        auto vk = std::make_unique<VkMock>();
        auto nvml = std::make_unique<NvmlMock>();
        DXGIDxvkAdapterMock* adapter = CreateDXGIDxvkAdapterMock();
        DXGIOutput6Mock* output = CreateDXGIOutput6Mock();
        LUID luid{};

        auto e = ConfigureDefaultTestEnvironment(*dxgiFactory, *vk, *nvml, *adapter, *output);

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

#if defined(WIDL_EXPLICIT_AGGREGATE_RETURNS)
        ALLOW_CALL(device, GetAdapterLuid(_))
            .LR_SIDE_EFFECT(*_1 = luid)
            .LR_RETURN(_1);
#else
        ALLOW_CALL(device, GetAdapterLuid())
            .LR_RETURN(luid);
#endif

        SECTION("GetGraphicsCapabilities without matching adapter returns OK with sm_0") {
            SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));
            REQUIRE(NvAPI_Initialize() == NVAPI_OK);

            NV_D3D12_GRAPHICS_CAPS graphicsCaps{};
            REQUIRE(NvAPI_D3D12_GetGraphicsCapabilities(static_cast<ID3D12Device*>(&device), NV_D3D12_GRAPHICS_CAPS_VER1, &graphicsCaps) == NVAPI_OK);
            REQUIRE(graphicsCaps.bFastUAVClearSupported == true);
            REQUIRE(graphicsCaps.majorSMVersion == 0);
            REQUIRE(graphicsCaps.majorSMVersion == 0);
        }

        SECTION("GetGraphicsCapabilities returns OK with valid SM") {
            struct Data {
                VkDriverId driverId;
                uint32_t deviceId;
                std::set<std::string> extensionNames;
                uint16_t expectedMajorSMVersion;
                uint16_t expectedMinorSMVersion;
                bool variablePixelRateShadingSupported;
            };
            auto args = GENERATE(
                Data{VK_DRIVER_ID_NVIDIA_PROPRIETARY, 0x2000, {VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME, VK_NV_CUDA_KERNEL_LAUNCH_EXTENSION_NAME}, 8, 6, true},
                Data{VK_DRIVER_ID_NVIDIA_PROPRIETARY, 0x2000, {VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME}, 0, 0, true},
                Data{VK_DRIVER_ID_NVIDIA_PROPRIETARY, 0x2000, {}, 0, 0, false},
                Data{VK_DRIVER_ID_MESA_NVK, 0x2600, {VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME}, 0, 0, true},
                Data{VK_DRIVER_ID_MESA_NVK, 0x2600, {}, 0, 0, false},
                Data{VK_DRIVER_ID_AMD_OPEN_SOURCE, 0x2000, {}, 0, 0, false},
                Data{VK_DRIVER_ID_NVIDIA_PROPRIETARY, 0x2000, {}, 0, 0, false});

            ::SetEnvironmentVariableA("DXVK_NVAPI_ALLOW_OTHER_DRIVERS", "1");

            auto extensionNames = std::set<std::string>{VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME};
            extensionNames.insert(args.extensionNames.begin(), args.extensionNames.end());

            luid.HighPart = 0x00000002;
            luid.LowPart = 0x00000001;

            ALLOW_CALL(*vk, GetDeviceExtensions(_, _))
                .RETURN(extensionNames);
            ALLOW_CALL(*vk, GetPhysicalDeviceProperties2(_, _, _))
                .SIDE_EFFECT(
                    ConfigureGetPhysicalDeviceProperties2(_3,
                        [&args, &luid](auto vkProps) {
                            memcpy(&vkProps.idProps->deviceLUID, &luid, sizeof(luid));
                            vkProps.idProps->deviceLUIDValid = VK_TRUE;
                            vkProps.driverProps->driverID = args.driverId;
                            vkProps.props->deviceID = args.deviceId;
                            if (args.extensionNames.contains(VK_NV_CUDA_KERNEL_LAUNCH_EXTENSION_NAME)) {
                                vkProps.cudaKernelLaunchProperties->computeCapabilityMajor = 8;
                                vkProps.cudaKernelLaunchProperties->computeCapabilityMinor = 6;
                            }
                        }));

            SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));
            REQUIRE(NvAPI_Initialize() == NVAPI_OK);

            NV_D3D12_GRAPHICS_CAPS graphicsCaps;
            REQUIRE(NvAPI_D3D12_GetGraphicsCapabilities(static_cast<ID3D12Device*>(&device), NV_D3D12_GRAPHICS_CAPS_VER1, &graphicsCaps) == NVAPI_OK);
            REQUIRE(graphicsCaps.majorSMVersion == args.expectedMajorSMVersion);
            REQUIRE(graphicsCaps.minorSMVersion == args.expectedMinorSMVersion);
            REQUIRE(graphicsCaps.bFastUAVClearSupported == true);
            REQUIRE(graphicsCaps.bExclusiveScissorRectsSupported == false);
            REQUIRE(graphicsCaps.bVariablePixelRateShadingSupported == args.variablePixelRateShadingSupported);
        }

        SECTION("GetGraphicsCapabilities with unknown struct version returns incompatible-struct-version") {
            SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));
            REQUIRE(NvAPI_Initialize() == NVAPI_OK);

            NV_D3D12_GRAPHICS_CAPS graphicsCaps{};
            REQUIRE(NvAPI_D3D12_GetGraphicsCapabilities(static_cast<ID3D12Device*>(&device), NV_D3D12_GRAPHICS_CAPS_VER1 + 1, &graphicsCaps) == NVAPI_INCOMPATIBLE_STRUCT_VERSION);
        }

        SECTION("GetGraphicsCapabilities with current struct version returns not incompatible-struct-version") {
            // This test should fail when a header update provides a newer not yet implemented struct version
            SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));
            REQUIRE(NvAPI_Initialize() == NVAPI_OK);

            NV_D3D12_GRAPHICS_CAPS graphicsCaps{};
            REQUIRE(NvAPI_D3D12_GetGraphicsCapabilities(static_cast<ID3D12Device*>(&device), NV_D3D12_GRAPHICS_CAPS_VER, &graphicsCaps) != NVAPI_INCOMPATIBLE_STRUCT_VERSION);
        }
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
            .WITH(_6 == std::string("shader"))
            .RETURN(S_OK)
            .TIMES(1);

        REQUIRE(NvAPI_D3D12_CreateCubinComputeShaderWithName(static_cast<ID3D12Device*>(&device), cubinData, cubinSize, blockX, blockY, blockZ, shaderName, reinterpret_cast<NVDX_ObjectHandle*>(handle)) == NVAPI_OK);
    }

    SECTION("DestroyCubinComputeShader returns OK") {
        auto shaderHandle = reinterpret_cast<D3D12_CUBIN_DATA_HANDLE*>(0x912122);
        REQUIRE_CALL(device, DestroyCubinComputeShader(shaderHandle))
            .RETURN(S_OK)
            .TIMES(1);

        REQUIRE(NvAPI_D3D12_DestroyCubinComputeShader(static_cast<ID3D12Device*>(&device), reinterpret_cast<NVDX_ObjectHandle>(shaderHandle)) == NVAPI_OK);
    }

    SECTION("GetCudaTextureObject/GetCudaSurfaceObject returns OK") {
        NvU32 handle;
        D3D12_CPU_DESCRIPTOR_HANDLE srvHandle = {0x123456};
        D3D12_CPU_DESCRIPTOR_HANDLE samplerHandle = {0x654321};
        REQUIRE_CALL(device, GetCudaTextureObject(_, _, _))
            .LR_WITH(_1.ptr == srvHandle.ptr && _2.ptr == samplerHandle.ptr)
            .RETURN(S_OK)
            .TIMES(1);
        REQUIRE_CALL(device, GetCudaSurfaceObject(_, _))
            .LR_WITH(_1.ptr == srvHandle.ptr)
            .RETURN(S_OK)
            .TIMES(1);

        REQUIRE(NvAPI_D3D12_GetCudaTextureObject(static_cast<ID3D12Device*>(&device), srvHandle, samplerHandle, &handle) == NVAPI_OK);
        REQUIRE(NvAPI_D3D12_GetCudaSurfaceObject(static_cast<ID3D12Device*>(&device), srvHandle, &handle) == NVAPI_OK);
    }

    SECTION("CaptureUAVInfo returns OK") {
        REQUIRE_CALL(device, CaptureUAVInfo(_))
            .RETURN(S_OK)
            .TIMES(1);

        NVAPI_UAV_INFO info{};
        REQUIRE(NvAPI_D3D12_CaptureUAVInfo(static_cast<ID3D12Device*>(&device), &info) == NVAPI_OK);
    }

    SECTION("IsFatbinPTXSupported returns OK") {
        auto isPTXSupported = false;
        REQUIRE(NvAPI_D3D12_IsFatbinPTXSupported(static_cast<ID3D12Device*>(&device), &isPTXSupported) == NVAPI_OK);
        REQUIRE(isPTXSupported == true);
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
    }

    SECTION("Launch CuBIN without ID3D12GraphicsCommandListExt1 returns OK") {
        ALLOW_CALL(commandList, QueryInterface(__uuidof(ID3D12GraphicsCommandListExt1), _))
            .RETURN(E_NOINTERFACE);

        auto shaderHandle = reinterpret_cast<D3D12_CUBIN_DATA_HANDLE*>(0xbadcf00d);
        auto blockX = 1U;
        auto blockY = 2U;
        auto blockZ = 3U;
        const void* params = nullptr;
        auto paramSize = 4U;
        REQUIRE_CALL(commandList, LaunchCubinShader(shaderHandle, blockX, blockY, blockZ, params, paramSize))
            .RETURN(S_OK)
            .TIMES(1);
        FORBID_CALL(commandList, LaunchCubinShaderEx(_, _, _, _, _, _, _, _, _));

        REQUIRE(NvAPI_D3D12_LaunchCubinShader(static_cast<ID3D12GraphicsCommandList*>(&commandList), reinterpret_cast<NVDX_ObjectHandle>(shaderHandle), blockX, blockY, blockZ, params, paramSize) == NVAPI_OK);
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

    SECTION("GetRaytracingCaps returns OK and claims that Displacement Micromap is not supported") {
        NVAPI_D3D12_RAYTRACING_DISPLACEMENT_MICROMAP_CAPS caps;
        REQUIRE(NvAPI_D3D12_GetRaytracingCaps(static_cast<ID3D12Device*>(&device), NVAPI_D3D12_RAYTRACING_CAPS_TYPE_DISPLACEMENT_MICROMAP, &caps, sizeof(caps)) == NVAPI_OK);
        REQUIRE(caps == NVAPI_D3D12_RAYTRACING_DISPLACEMENT_MICROMAP_CAP_NONE);
    }

    SECTION("GetRaytracingAccelerationStructurePrebuildInfoEx succeeds") {
        SECTION("GetRaytracingAccelerationStructurePrebuildInfoEx returns OK") {
            NVAPI_D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS_EX desc{};
            D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info{};
            NVAPI_GET_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO_EX_PARAMS params{};
            params.version = NVAPI_GET_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO_EX_PARAMS_VER1;
            params.pDesc = &desc;
            params.pInfo = &info;

            D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS d3d12Desc{};
            std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> geometryDescs{};
            ALLOW_CALL(device, GetRaytracingAccelerationStructurePrebuildInfo(_, &info))
                .LR_SIDE_EFFECT({
                    d3d12Desc = *_1;

                    // we know that our implementation passes pointer to data in stack-allocated vector in pGeometryDescs
                    // so we need to copy it before the function returns and the vector is destroyed
                    if (_1->Type == D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL && _1->DescsLayout == D3D12_ELEMENTS_LAYOUT_ARRAY)
                        geometryDescs.assign(_1->pGeometryDescs, _1->pGeometryDescs + _1->NumDescs);
                });

            SECTION("GetRaytracingAccelerationStructurePrebuildInfoEx with TLAS") {
                desc.type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
                desc.instanceDescs = D3D12_GPU_VIRTUAL_ADDRESS{};

                REQUIRE(NvAPI_D3D12_GetRaytracingAccelerationStructurePrebuildInfoEx(static_cast<ID3D12Device5*>(&device), &params) == NVAPI_OK);
                REQUIRE(d3d12Desc.InstanceDescs == desc.instanceDescs);
            }

            SECTION("GetRaytracingAccelerationStructurePrebuildInfoEx with BLAS for pointer array") {
                NVAPI_D3D12_RAYTRACING_GEOMETRY_DESC_EX** geometryDescExArray = nullptr;
                desc.type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
                desc.descsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY_OF_POINTERS;
                desc.ppGeometryDescs = geometryDescExArray;

                REQUIRE(NvAPI_D3D12_GetRaytracingAccelerationStructurePrebuildInfoEx(static_cast<ID3D12Device5*>(&device), &params) == NVAPI_OK);
                REQUIRE(d3d12Desc.ppGeometryDescs == reinterpret_cast<const D3D12_RAYTRACING_GEOMETRY_DESC* const*>(desc.ppGeometryDescs));
            }

            SECTION("GetRaytracingAccelerationStructurePrebuildInfoEx with BLAS for array") {
                NVAPI_D3D12_RAYTRACING_GEOMETRY_DESC_EX geometryDescEx[2];
                desc.type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
                desc.descsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
                desc.numDescs = 2;
                desc.pGeometryDescs = geometryDescEx;
                desc.geometryDescStrideInBytes = sizeof(NVAPI_D3D12_RAYTRACING_GEOMETRY_DESC_EX);

                SECTION("GetRaytracingAccelerationStructurePrebuildInfoEx for triangles geometry") {
                    geometryDescEx[0].type = NVAPI_D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES_EX;
                    geometryDescEx[0].triangles.IndexBuffer = 3;
                    geometryDescEx[1].type = NVAPI_D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES_EX;
                    geometryDescEx[1].triangles.IndexBuffer = 4;

                    REQUIRE(NvAPI_D3D12_GetRaytracingAccelerationStructurePrebuildInfoEx(static_cast<ID3D12Device5*>(&device), &params) == NVAPI_OK);
                    REQUIRE(d3d12Desc.NumDescs == desc.numDescs);
                    REQUIRE(d3d12Desc.DescsLayout == desc.descsLayout);
                    REQUIRE(geometryDescs[0].Type == D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES);
                    REQUIRE(geometryDescs[0].Triangles.IndexBuffer == geometryDescEx[0].triangles.IndexBuffer);
                    REQUIRE(geometryDescs[1].Type == D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES);
                    REQUIRE(geometryDescs[1].Triangles.IndexBuffer == geometryDescEx[1].triangles.IndexBuffer);
                }

                SECTION("GetRaytracingAccelerationStructurePrebuildInfoEx with AABBs geometry") {
                    geometryDescEx[0].type = NVAPI_D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS_EX;
                    geometryDescEx[0].aabbs.AABBCount = 3;
                    geometryDescEx[1].type = NVAPI_D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS_EX;
                    geometryDescEx[1].aabbs.AABBCount = 4;

                    REQUIRE(NvAPI_D3D12_GetRaytracingAccelerationStructurePrebuildInfoEx(static_cast<ID3D12Device5*>(&device), &params) == NVAPI_OK);
                    REQUIRE(d3d12Desc.NumDescs == desc.numDescs);
                    REQUIRE(d3d12Desc.DescsLayout == desc.descsLayout);
                    REQUIRE(geometryDescs[0].Type == D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS);
                    REQUIRE(geometryDescs[0].AABBs.AABBCount == geometryDescEx[0].aabbs.AABBCount);
                    REQUIRE(geometryDescs[1].Type == D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS);
                    REQUIRE(geometryDescs[1].AABBs.AABBCount == geometryDescEx[1].aabbs.AABBCount);
                }
            }

            SECTION("GetRaytracingAccelerationStructurePrebuildInfoEx with BLAS for array of R520 structures") {
                NVAPI_D3D12_RAYTRACING_GEOMETRY_DESC_EX_R520 geometryDescExR520[2];
                desc.type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
                desc.descsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
                desc.numDescs = 2;
                desc.pGeometryDescs = reinterpret_cast<NVAPI_D3D12_RAYTRACING_GEOMETRY_DESC_EX*>(geometryDescExR520);
                desc.geometryDescStrideInBytes = sizeof(NVAPI_D3D12_RAYTRACING_GEOMETRY_DESC_EX_R520);

                SECTION("GetRaytracingAccelerationStructurePrebuildInfoEx for triangles geometry") {
                    geometryDescExR520[0].type = NVAPI_D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES_EX;
                    geometryDescExR520[0].triangles.IndexBuffer = 3;
                    geometryDescExR520[1].type = NVAPI_D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES_EX;
                    geometryDescExR520[1].triangles.IndexBuffer = 4;

                    REQUIRE(NvAPI_D3D12_GetRaytracingAccelerationStructurePrebuildInfoEx(static_cast<ID3D12Device5*>(&device), &params) == NVAPI_OK);
                    REQUIRE(d3d12Desc.NumDescs == desc.numDescs);
                    REQUIRE(d3d12Desc.DescsLayout == desc.descsLayout);
                    REQUIRE(geometryDescs[0].Type == D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES);
                    REQUIRE(geometryDescs[0].Triangles.IndexBuffer == geometryDescExR520[0].triangles.IndexBuffer);
                    REQUIRE(geometryDescs[1].Type == D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES);
                    REQUIRE(geometryDescs[1].Triangles.IndexBuffer == geometryDescExR520[1].triangles.IndexBuffer);
                }

                SECTION("GetRaytracingAccelerationStructurePrebuildInfoEx with AABBs geometry") {
                    geometryDescExR520[0].type = NVAPI_D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS_EX;
                    geometryDescExR520[0].aabbs.AABBCount = 3;
                    geometryDescExR520[1].type = NVAPI_D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS_EX;
                    geometryDescExR520[1].aabbs.AABBCount = 4;

                    REQUIRE(NvAPI_D3D12_GetRaytracingAccelerationStructurePrebuildInfoEx(static_cast<ID3D12Device5*>(&device), &params) == NVAPI_OK);
                    REQUIRE(d3d12Desc.NumDescs == desc.numDescs);
                    REQUIRE(d3d12Desc.DescsLayout == desc.descsLayout);
                    REQUIRE(geometryDescs[0].Type == D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS);
                    REQUIRE(geometryDescs[0].AABBs.AABBCount == geometryDescExR520[0].aabbs.AABBCount);
                    REQUIRE(geometryDescs[1].Type == D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS);
                    REQUIRE(geometryDescs[1].AABBs.AABBCount == geometryDescExR520[1].aabbs.AABBCount);
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
            NVAPI_D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC_EX desc{};
            NVAPI_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_EX_PARAMS params{};
            params.version = NVAPI_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_EX_PARAMS_VER1;
            params.pDesc = &desc;

            D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC d3d12Desc{};
            std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> geometryDescs{};
            ALLOW_CALL(commandList, BuildRaytracingAccelerationStructure(_, params.numPostbuildInfoDescs, params.pPostbuildInfoDescs))
                .LR_SIDE_EFFECT({
                    d3d12Desc = *_1;

                    // we know that our implementation passes pointer to data in stack-allocated vector in pGeometryDescs
                    // so we need to copy it before the function returns and the vector is destroyed
                    if (_1->Inputs.Type == D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL && _1->Inputs.DescsLayout == D3D12_ELEMENTS_LAYOUT_ARRAY)
                        geometryDescs.assign(_1->Inputs.pGeometryDescs, _1->Inputs.pGeometryDescs + _1->Inputs.NumDescs);
                });

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
                NVAPI_D3D12_RAYTRACING_GEOMETRY_DESC_EX** geometryDescExArray = nullptr;
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
                NVAPI_D3D12_RAYTRACING_GEOMETRY_DESC_EX geometryDescEx[2];
                desc.inputs.type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
                desc.inputs.descsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
                desc.inputs.numDescs = 2;
                desc.inputs.pGeometryDescs = geometryDescEx;
                desc.inputs.geometryDescStrideInBytes = sizeof(NVAPI_D3D12_RAYTRACING_GEOMETRY_DESC_EX);

                SECTION("BuildRaytracingAccelerationStructureEx for triangles geometry") {
                    geometryDescEx[0].type = NVAPI_D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES_EX;
                    geometryDescEx[0].triangles.IndexBuffer = 3;
                    geometryDescEx[1].type = NVAPI_D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES_EX;
                    geometryDescEx[1].triangles.IndexBuffer = 4;

                    REQUIRE(NvAPI_D3D12_BuildRaytracingAccelerationStructureEx(static_cast<ID3D12GraphicsCommandList4*>(&commandList), &params) == NVAPI_OK);
                    REQUIRE(d3d12Desc.DestAccelerationStructureData == desc.destAccelerationStructureData);
                    REQUIRE(d3d12Desc.Inputs.NumDescs == desc.inputs.numDescs);
                    REQUIRE(d3d12Desc.Inputs.DescsLayout == desc.inputs.descsLayout);
                    REQUIRE(geometryDescs[0].Type == D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES);
                    REQUIRE(geometryDescs[0].Triangles.IndexBuffer == geometryDescEx[0].triangles.IndexBuffer);
                    REQUIRE(geometryDescs[1].Type == D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES);
                    REQUIRE(geometryDescs[1].Triangles.IndexBuffer == geometryDescEx[1].triangles.IndexBuffer);
                    REQUIRE(d3d12Desc.SourceAccelerationStructureData == desc.sourceAccelerationStructureData);
                    REQUIRE(d3d12Desc.ScratchAccelerationStructureData == desc.scratchAccelerationStructureData);
                }

                SECTION("BuildRaytracingAccelerationStructureEx for AABBs geometry") {
                    geometryDescEx[0].type = NVAPI_D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS_EX;
                    geometryDescEx[0].aabbs.AABBCount = 3;
                    geometryDescEx[1].type = NVAPI_D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS_EX;
                    geometryDescEx[1].aabbs.AABBCount = 4;

                    REQUIRE(NvAPI_D3D12_BuildRaytracingAccelerationStructureEx(static_cast<ID3D12GraphicsCommandList4*>(&commandList), &params) == NVAPI_OK);
                    REQUIRE(d3d12Desc.DestAccelerationStructureData == desc.destAccelerationStructureData);
                    REQUIRE(d3d12Desc.Inputs.NumDescs == desc.inputs.numDescs);
                    REQUIRE(d3d12Desc.Inputs.DescsLayout == desc.inputs.descsLayout);
                    REQUIRE(geometryDescs[0].Type == D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS);
                    REQUIRE(geometryDescs[0].AABBs.AABBCount == geometryDescEx[0].aabbs.AABBCount);
                    REQUIRE(geometryDescs[1].Type == D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS);
                    REQUIRE(geometryDescs[1].AABBs.AABBCount == geometryDescEx[1].aabbs.AABBCount);
                    REQUIRE(d3d12Desc.SourceAccelerationStructureData == desc.sourceAccelerationStructureData);
                    REQUIRE(d3d12Desc.ScratchAccelerationStructureData == desc.scratchAccelerationStructureData);
                }
            }

            SECTION("BuildRaytracingAccelerationStructureEx with BLAS for array of R520 structures") {
                NVAPI_D3D12_RAYTRACING_GEOMETRY_DESC_EX_R520 geometryDescExR520[2];
                desc.inputs.type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
                desc.inputs.descsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
                desc.inputs.numDescs = 2;
                desc.inputs.pGeometryDescs = reinterpret_cast<NVAPI_D3D12_RAYTRACING_GEOMETRY_DESC_EX*>(geometryDescExR520);
                desc.inputs.geometryDescStrideInBytes = sizeof(NVAPI_D3D12_RAYTRACING_GEOMETRY_DESC_EX_R520);

                SECTION("BuildRaytracingAccelerationStructureEx for triangles geometry") {
                    geometryDescExR520[0].type = NVAPI_D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES_EX;
                    geometryDescExR520[0].triangles.IndexBuffer = 3;
                    geometryDescExR520[1].type = NVAPI_D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES_EX;
                    geometryDescExR520[1].triangles.IndexBuffer = 4;

                    REQUIRE(NvAPI_D3D12_BuildRaytracingAccelerationStructureEx(static_cast<ID3D12GraphicsCommandList4*>(&commandList), &params) == NVAPI_OK);
                    REQUIRE(d3d12Desc.DestAccelerationStructureData == desc.destAccelerationStructureData);
                    REQUIRE(d3d12Desc.Inputs.NumDescs == desc.inputs.numDescs);
                    REQUIRE(d3d12Desc.Inputs.DescsLayout == desc.inputs.descsLayout);
                    REQUIRE(geometryDescs[0].Type == D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES);
                    REQUIRE(geometryDescs[0].Triangles.IndexBuffer == geometryDescExR520[0].triangles.IndexBuffer);
                    REQUIRE(geometryDescs[1].Type == D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES);
                    REQUIRE(geometryDescs[1].Triangles.IndexBuffer == geometryDescExR520[1].triangles.IndexBuffer);
                    REQUIRE(d3d12Desc.SourceAccelerationStructureData == desc.sourceAccelerationStructureData);
                    REQUIRE(d3d12Desc.ScratchAccelerationStructureData == desc.scratchAccelerationStructureData);
                }

                SECTION("BuildRaytracingAccelerationStructureEx for AABBs geometry") {
                    geometryDescExR520[0].type = NVAPI_D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS_EX;
                    geometryDescExR520[0].aabbs.AABBCount = 3;
                    geometryDescExR520[1].type = NVAPI_D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS_EX;
                    geometryDescExR520[1].aabbs.AABBCount = 4;

                    REQUIRE(NvAPI_D3D12_BuildRaytracingAccelerationStructureEx(static_cast<ID3D12GraphicsCommandList4*>(&commandList), &params) == NVAPI_OK);
                    REQUIRE(d3d12Desc.DestAccelerationStructureData == desc.destAccelerationStructureData);
                    REQUIRE(d3d12Desc.Inputs.NumDescs == desc.inputs.numDescs);
                    REQUIRE(d3d12Desc.Inputs.DescsLayout == desc.inputs.descsLayout);
                    REQUIRE(geometryDescs[0].Type == D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS);
                    REQUIRE(geometryDescs[0].AABBs.AABBCount == geometryDescExR520[0].aabbs.AABBCount);
                    REQUIRE(geometryDescs[1].Type == D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS);
                    REQUIRE(geometryDescs[1].AABBs.AABBCount == geometryDescExR520[1].aabbs.AABBCount);
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

    SECTION("D3DLowLatencyDevice methods succeed") {
        auto dxgiFactory = std::make_unique<DXGIDxvkFactoryMock>();
        auto vk = std::make_unique<VkMock>();
        auto nvml = std::make_unique<NvmlMock>();
        DXGIDxvkAdapterMock* adapter = CreateDXGIDxvkAdapterMock();
        DXGIOutput6Mock* output = CreateDXGIOutput6Mock();

        auto e = ConfigureDefaultTestEnvironment(*dxgiFactory, *vk, *nvml, *adapter, *output);

        ALLOW_CALL(commandQueue, GetDevice(__uuidof(ID3DLowLatencyDevice), _))
            .LR_SIDE_EFFECT(*_2 = static_cast<ID3DLowLatencyDevice*>(&lowLatencyDevice))
            .LR_SIDE_EFFECT(lowLatencyDeviceRefCount++)
            .RETURN(S_OK);
        ALLOW_CALL(commandQueue, QueryInterface(__uuidof(ID3D12DeviceChild), _))
            .LR_SIDE_EFFECT(*_2 = static_cast<ID3D12DeviceChild*>(&commandQueue))
            .LR_SIDE_EFFECT(commandQueueRefCount++)
            .RETURN(S_OK);
        ALLOW_CALL(commandQueue, QueryInterface(__uuidof(ID3DLowLatencyDevice), _))
            .RETURN(E_NOINTERFACE);
        ALLOW_CALL(device, QueryInterface(__uuidof(ID3DLowLatencyDevice), _))
            .LR_SIDE_EFFECT(*_2 = static_cast<ID3DLowLatencyDevice*>(&lowLatencyDevice))
            .LR_SIDE_EFFECT(lowLatencyDeviceRefCount++)
            .RETURN(S_OK);
        ALLOW_CALL(lowLatencyDevice, AddRef())
            .LR_SIDE_EFFECT(lowLatencyDeviceRefCount++)
            .RETURN(lowLatencyDeviceRefCount);
        ALLOW_CALL(lowLatencyDevice, Release())
            .LR_SIDE_EFFECT(lowLatencyDeviceRefCount--)
            .RETURN(lowLatencyDeviceRefCount);
        ALLOW_CALL(lowLatencyDevice, SupportsLowLatency())
            .RETURN(true);

        SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));

        SECTION("NotifyOutOfBandCommandQueue succeeds") {
            SECTION("NotifyOutOfBandCommandQueue returns OK") {
                REQUIRE_CALL(commandQueue, NotifyOutOfBandCommandQueue(static_cast<D3D12_OUT_OF_BAND_CQ_TYPE>(OUT_OF_BAND_RENDER)))
                    .RETURN(S_OK);

                REQUIRE(NvAPI_Initialize() == NVAPI_OK);
                REQUIRE(NvAPI_D3D12_NotifyOutOfBandCommandQueue(&commandQueue, OUT_OF_BAND_RENDER) == NVAPI_OK);
            }

            SECTION("NotifyOutOfBandCommandQueue with null command queue returns invalid-pointer") {
                REQUIRE(NvAPI_Initialize() == NVAPI_OK);
                REQUIRE(NvAPI_D3D12_NotifyOutOfBandCommandQueue(nullptr, OUT_OF_BAND_RENDER) == NVAPI_INVALID_POINTER);
            }
        }

        SECTION("SetAsyncFrameMarker succeeds") {
            SECTION("SetAsyncFrameMarker returns OK") {
                REQUIRE_CALL(lowLatencyDevice, SetLatencyMarker(1ULL, VK_LATENCY_MARKER_OUT_OF_BAND_RENDERSUBMIT_START_NV))
                    .RETURN(S_OK);

                REQUIRE(NvAPI_Initialize() == NVAPI_OK);

                NV_ASYNC_FRAME_MARKER_PARAMS params{};
                params.version = NV_ASYNC_FRAME_MARKER_PARAMS_VER1;
                params.frameID = 123ULL;
                params.markerType = OUT_OF_BAND_RENDERSUBMIT_START;
                REQUIRE(NvAPI_D3D12_SetAsyncFrameMarker(&commandQueue, &params) == NVAPI_OK);
            }

            SECTION("SetAsyncFrameMarker drops PC_LATENCY_PING and returns OK") {
                FORBID_CALL(lowLatencyDevice, SetLatencyMarker(_, _));

                REQUIRE(NvAPI_Initialize() == NVAPI_OK);

                NV_ASYNC_FRAME_MARKER_PARAMS params{};
                params.version = NV_ASYNC_FRAME_MARKER_PARAMS_VER1;
                params.frameID = 123ULL;
                params.markerType = PC_LATENCY_PING;
                REQUIRE(NvAPI_D3D12_SetAsyncFrameMarker(&commandQueue, &params) == NVAPI_OK);
            }

            SECTION("SetAsyncFrameMarker with second command queue from the same device uses the same D3DLowLatencyDevice") {
                D3D12Vkd3dCommandQueueMock otherCommandQueue;
                ALLOW_CALL(otherCommandQueue, QueryInterface(__uuidof(ID3D12CommandQueue), _))
                    .LR_SIDE_EFFECT(*_2 = static_cast<ID3D12CommandQueue*>(&otherCommandQueue))
                    .RETURN(S_OK);
                ALLOW_CALL(otherCommandQueue, QueryInterface(__uuidof(ID3D12DeviceChild), _))
                    .LR_SIDE_EFFECT(*_2 = static_cast<ID3D12DeviceChild*>(&otherCommandQueue))
                    .RETURN(S_OK);
                ALLOW_CALL(otherCommandQueue, QueryInterface(__uuidof(ID3D11DeviceChild), _))
                    .RETURN(E_NOINTERFACE);
                ALLOW_CALL(otherCommandQueue, AddRef())
                    .RETURN(1);
                ALLOW_CALL(otherCommandQueue, Release())
                    .RETURN(1);
                ALLOW_CALL(otherCommandQueue, GetDevice(__uuidof(ID3DLowLatencyDevice), _))
                    .LR_SIDE_EFFECT(*_2 = static_cast<ID3DLowLatencyDevice*>(&lowLatencyDevice))
                    .LR_SIDE_EFFECT(lowLatencyDeviceRefCount++)
                    .RETURN(S_OK);
                ALLOW_CALL(otherCommandQueue, QueryInterface(__uuidof(ID3DLowLatencyDevice), _))
                    .RETURN(E_NOINTERFACE);

                REQUIRE_CALL(lowLatencyDevice, SetLatencyMarker(_, VK_LATENCY_MARKER_OUT_OF_BAND_RENDERSUBMIT_START_NV))
                    .RETURN(S_OK)
                    .TIMES(4);

                REQUIRE(NvAPI_Initialize() == NVAPI_OK);

                NV_ASYNC_FRAME_MARKER_PARAMS params{};
                params.version = NV_ASYNC_FRAME_MARKER_PARAMS_VER1;
                params.frameID = 123ULL;
                params.markerType = OUT_OF_BAND_RENDERSUBMIT_START;
                REQUIRE(NvAPI_D3D12_SetAsyncFrameMarker(&commandQueue, &params) == NVAPI_OK);
                params.frameID = 124ULL;
                REQUIRE(NvAPI_D3D12_SetAsyncFrameMarker(&otherCommandQueue, &params) == NVAPI_OK);
                params.frameID = 125ULL;
                REQUIRE(NvAPI_D3D12_SetAsyncFrameMarker(&commandQueue, &params) == NVAPI_OK);
                params.frameID = 126ULL;
                REQUIRE(NvAPI_D3D12_SetAsyncFrameMarker(&otherCommandQueue, &params) == NVAPI_OK);
            }

            SECTION("SetAsyncFrameMarker with unknown struct version returns incompatible-struct-version") {
                REQUIRE(NvAPI_Initialize() == NVAPI_OK);

                NV_ASYNC_FRAME_MARKER_PARAMS params{};
                params.version = NV_ASYNC_FRAME_MARKER_PARAMS_VER1 + 1;
                REQUIRE(NvAPI_D3D12_SetAsyncFrameMarker(&commandQueue, &params) == NVAPI_INCOMPATIBLE_STRUCT_VERSION);
            }

            SECTION("SetAsyncFrameMarker with current struct version returns not incompatible-struct-version") {
                ALLOW_CALL(lowLatencyDevice, SetLatencyMarker(_, _))
                    .RETURN(S_OK);

                REQUIRE(NvAPI_Initialize() == NVAPI_OK);

                NV_ASYNC_FRAME_MARKER_PARAMS params{};
                params.version = NV_ASYNC_FRAME_MARKER_PARAMS_VER;
                REQUIRE(NvAPI_D3D12_SetAsyncFrameMarker(&commandQueue, &params) != NVAPI_INCOMPATIBLE_STRUCT_VERSION);
            }

            SECTION("SetAsyncFrameMarker with null command queue returns invalid-pointer") {
                REQUIRE(NvAPI_Initialize() == NVAPI_OK);

                NV_ASYNC_FRAME_MARKER_PARAMS params{};
                params.version = NV_ASYNC_FRAME_MARKER_PARAMS_VER;
                REQUIRE(NvAPI_D3D12_SetAsyncFrameMarker(nullptr, &params) == NVAPI_INVALID_POINTER);
            }
        }
    }

    CHECK(deviceRefCount == 0);
    CHECK(commandListRefCount == 0);
    CHECK(commandQueueRefCount == 0);
    CHECK(lowLatencyDeviceRefCount == 0);
}
