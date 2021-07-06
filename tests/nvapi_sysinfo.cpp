#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"

#include "../inc/catch.hpp"
#include "../inc/catch2/trompeloeil.hpp"
#include "../src/sysinfo/resource_factory.h"
#include "../src/dxvk/dxvk_interfaces.h"
#include "../src/nvapi.cpp"
#include "../src/nvapi_sys.cpp"
#include "../src/nvapi_gpu.cpp"
#include "../src/nvapi_disp.cpp"
#include "../src/nvapi_mosaic.cpp"
#include "nvapi_sysinfo_mocks.cpp"
#include "nvapi_sysinfo_util.cpp"

using namespace trompeloeil;

TEST_CASE("GetInterfaceVersionString returns OK", "[sysinfo]") {
    NvAPI_ShortString desc;
    REQUIRE(NvAPI_GetInterfaceVersionString(desc) == NVAPI_OK);
    REQUIRE(strcmp(desc, "R470") == 0);
}

TEST_CASE("GetErrorMessage returns OK", "[sysinfo]") {
    NvAPI_ShortString desc;
    REQUIRE(NvAPI_GetErrorMessage(NVAPI_NVIDIA_DEVICE_NOT_FOUND, desc) == NVAPI_OK);
    REQUIRE(strcmp(desc, "NVAPI_NVIDIA_DEVICE_NOT_FOUND") == 0);
}

TEST_CASE("Initialize returns device-not-found when DXVK reports no adapters", "[sysinfo]") {
    auto dxgiFactory = std::make_unique<DXGIFactoryMock>();
    auto vulkan = std::make_unique<VulkanMock>();
    auto nvml = std::make_unique<NvmlMock>();

    ALLOW_CALL(*dxgiFactory, AddRef())
        .RETURN(1);
    ALLOW_CALL(*dxgiFactory, Release())
        .RETURN(0);
    ALLOW_CALL(*dxgiFactory, EnumAdapters(_, _))
        .RETURN(DXGI_ERROR_NOT_FOUND);

    ALLOW_CALL(*vulkan, IsAvailable())
        .RETURN(true);

    ALLOW_CALL(*nvml, IsAvailable())
        .RETURN(false);

    SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml));
    REQUIRE(NvAPI_Initialize() == NVAPI_NVIDIA_DEVICE_NOT_FOUND);
    REQUIRE(NvAPI_Unload() == NVAPI_API_NOT_INITIALIZED);
}

TEST_CASE("Sysinfo methods succeed", "[sysinfo]") {
    auto dxgiFactory = std::make_unique<DXGIFactoryMock>();
    DXGIDxvkAdapterMock adapter;
    DXGIOutputMock output;
    auto vulkan = std::make_unique<VulkanMock>();
    auto nvml = std::make_unique<NvmlMock>();

    ALLOW_CALL(*dxgiFactory, AddRef())
        .RETURN(1);
    ALLOW_CALL(*dxgiFactory, Release())
        .RETURN(0);
    ALLOW_CALL(*dxgiFactory, EnumAdapters(0U, _))
        .LR_SIDE_EFFECT(*_2 = static_cast<IDXGIAdapter*>(&adapter))
        .RETURN(S_OK);
    ALLOW_CALL(*dxgiFactory, EnumAdapters(1U, _))
        .RETURN(DXGI_ERROR_NOT_FOUND);

    ALLOW_CALL(adapter, QueryInterface(IDXGIVkInteropAdapter::guid, _))
        .LR_SIDE_EFFECT(*_2 = static_cast<IDXGIVkInteropAdapter*>(&adapter))
        .RETURN(S_OK);
    ALLOW_CALL(adapter, Release())
        .RETURN(0);
    ALLOW_CALL(adapter, EnumOutputs(0U, _))
        .LR_SIDE_EFFECT(*_2 = static_cast<IDXGIOutput*>(&output))
        .RETURN(S_OK);
    ALLOW_CALL(adapter, EnumOutputs(1U, _))
        .RETURN(DXGI_ERROR_NOT_FOUND);
    ALLOW_CALL(adapter, GetVulkanHandles(_, _));

    ALLOW_CALL(output, Release())
        .RETURN(0);
    ALLOW_CALL(output, GetDesc(_))
        .SIDE_EFFECT(*_1 = DXGI_OUTPUT_DESC{L"Output1", {0,0,0,0}, 1, DXGI_MODE_ROTATION_UNSPECIFIED, nullptr})
        .RETURN(S_OK);

    ALLOW_CALL(*vulkan, IsAvailable())
        .RETURN(true);
    ALLOW_CALL(*vulkan, GetDeviceExtensions(_, _))
        .RETURN(std::set<std::string>{"ext"});
    ALLOW_CALL(*vulkan, GetPhysicalDeviceProperties2(_, _, _))
        .SIDE_EFFECT(strcpy(_3->properties.deviceName, "Device1"));
    ALLOW_CALL(*vulkan, GetPhysicalDeviceMemoryProperties2(_, _, _));

    ALLOW_CALL(*nvml, IsAvailable())
        .RETURN(false);

    SECTION("Initialize and unloads return OK") {
        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);
        REQUIRE(NvAPI_Unload() == NVAPI_OK);
    }

    SECTION("GetDisplayDriverVersion returns OK", "[sysinfo]") {
        struct Data {VkDriverId driverId; u_int major; u_int minor; u_int patch; u_int expectedVersion;};
        auto args = GENERATE(
            Data{VK_DRIVER_ID_NVIDIA_PROPRIETARY, 0x1d6, 0x2d, 0x01, 47045},
            Data{VK_DRIVER_ID_NVIDIA_PROPRIETARY, 0x1d6, 0x66, 0x01, 47099},
            Data{VK_DRIVER_ID_AMD_OPEN_SOURCE, 0x15, 0x02, 0x03, 2102});

        ALLOW_CALL(*vulkan, GetDeviceExtensions(_, _)) // NOLINT(bugprone-use-after-move)
            .RETURN(std::set<std::string>{VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME});
        ALLOW_CALL(*vulkan, GetPhysicalDeviceProperties2(_, _, _))
            .SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [&args](auto props, auto idProps, auto pciBusInfoProps, auto driverProps, auto fragmentShadingRateProps) {
                        driverProps->driverID = args.driverId;
                        strcpy(props->properties.deviceName, "GPU0");
                        if (args.driverId == VK_DRIVER_ID_NVIDIA_PROPRIETARY)
                            props->properties.driverVersion = (args.major << 22) | (args.minor << 14) | (args.patch << 6);
                        else
                            props->properties.driverVersion = (args.major << 22) | (args.minor << 12) | args.patch;
                    })
            );

        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvDisplayHandle handle;
        REQUIRE(NvAPI_EnumNvidiaDisplayHandle(0, &handle) == NVAPI_OK);

        NV_DISPLAY_DRIVER_VERSION version;
        version.version = NV_DISPLAY_DRIVER_VERSION_VER;
        REQUIRE(NvAPI_GetDisplayDriverVersion(handle, &version) == NVAPI_OK);
        REQUIRE(version.drvVersion == args.expectedVersion);
        REQUIRE(strcmp(version.szAdapterString, "GPU0") == 0);
        REQUIRE(std::string(version.szBuildBranchString).length() > 0);
    }

    SECTION("GetGPUType returns OK", "[sysinfo]") {
        ALLOW_CALL(*vulkan, GetPhysicalDeviceProperties2(_, _, _)) // NOLINT(bugprone-use-after-move)
            .SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [](auto props, auto idProps, auto pciBusInfoProps, auto driverProps, auto fragmentShadingRateProps) {
                        props->properties.deviceType = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
                    })
                );

        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(0, &handle) == NVAPI_OK);

        NV_GPU_TYPE type;
        REQUIRE(NvAPI_GPU_GetGPUType(handle, &type) == NVAPI_OK);
        REQUIRE(type == NV_SYSTEM_TYPE_DGPU);
    }

    SECTION("GetPCIIdentifiers returns OK", "[sysinfo]") {
        ALLOW_CALL(*vulkan, GetPhysicalDeviceProperties2(_, _, _)) // NOLINT(bugprone-use-after-move)
            .SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [](auto props, auto idProps, auto pciBusInfoProps, auto driverProps, auto fragmentShadingRateProps) {
                        props->properties.vendorID = 0x10de;
                        props->properties.deviceID = 0x1234;
                    })
                );

        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(0, &handle) == NVAPI_OK);

        NvU32 deviceId, subSystemId, revisionId, extDeviceId;
        REQUIRE(NvAPI_GPU_GetPCIIdentifiers(handle, &deviceId, &subSystemId, &revisionId, &extDeviceId) == NVAPI_OK);
        REQUIRE(deviceId == 0x123410de);
        REQUIRE(subSystemId == 0);
        REQUIRE(revisionId == 0);
        REQUIRE(extDeviceId == 0);
    }

    SECTION("GetFullName returns OK", "[sysinfo]") {
        ALLOW_CALL(*vulkan, GetPhysicalDeviceProperties2(_, _, _)) // NOLINT(bugprone-use-after-move)
            .SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [](auto props, auto idProps, auto pciBusInfoProps, auto driverProps, auto fragmentShadingRateProps) {
                        strcpy(props->properties.deviceName, "High-End GPU01");
                    })
                );

        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(0, &handle) == NVAPI_OK);

        NvAPI_ShortString name;
        REQUIRE(NvAPI_GPU_GetFullName(handle, name) == NVAPI_OK);
        REQUIRE(strcmp(name, "High-End GPU01") == 0);
    }

    SECTION("GetBusId returns OK", "[sysinfo]") {
        ALLOW_CALL(*vulkan, GetDeviceExtensions(_, _)) // NOLINT(bugprone-use-after-move)
            .RETURN(std::set<std::string>{VK_EXT_PCI_BUS_INFO_EXTENSION_NAME});
        ALLOW_CALL(*vulkan, GetPhysicalDeviceProperties2(_, _, _))
            .SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [](auto props, auto idProps, auto pciBusInfoProps, auto driverProps, auto fragmentShadingRateProps) {
                        pciBusInfoProps->pciBus = 2U;
                    })
                );

        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(0, &handle) == NVAPI_OK);

        NvU32 busId;
        REQUIRE(NvAPI_GPU_GetBusId(handle, &busId) == NVAPI_OK);
        REQUIRE(busId == 2);
    }

    SECTION("GetPhysicalFrameBufferSize returns OK", "[sysinfo]") {
        ALLOW_CALL(*vulkan, GetPhysicalDeviceMemoryProperties2(_, _, _)) // NOLINT(bugprone-use-after-move)
            .SIDE_EFFECT({
                _3->memoryProperties.memoryHeapCount = 1;
                _3->memoryProperties.memoryHeaps[0].flags = VK_MEMORY_HEAP_DEVICE_LOCAL_BIT;
                _3->memoryProperties.memoryHeaps[0].size = 8191 * 1024;
            });

        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(0, &handle) == NVAPI_OK);

        NvU32 size;
        REQUIRE(NvAPI_GPU_GetPhysicalFrameBufferSize(handle, &size) == NVAPI_OK);
        REQUIRE(size == 8191);
    }

    SECTION("GetAdapterIdFromPhysicalGpu returns OK") {
        ALLOW_CALL(*vulkan, GetPhysicalDeviceProperties2(_, _, _)) // NOLINT(bugprone-use-after-move)
            .SIDE_EFFECT(
                 ConfigureGetPhysicalDeviceProperties2(_3,
                     [](auto props, auto idProps, auto pciBusInfoProps, auto driverProps, auto fragmentShadingRateProps) {
                         idProps->deviceLUIDValid = VK_TRUE;
                         for (auto i = 0U; i < VK_LUID_SIZE; i++)
                             idProps->deviceLUID[i] = i + 1;
                    })
            );

        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(0, &handle) == NVAPI_OK);

        LUID luid;
        REQUIRE(NvAPI_GPU_GetAdapterIdFromPhysicalGpu(handle, static_cast<void*>(&luid)) == NVAPI_OK);
        REQUIRE(luid.LowPart  == 0x04030201);
        REQUIRE(luid.HighPart == 0x08070605);
    }

    SECTION("GetArchInfo returns OK") {
        struct Data {std::string extensionName; NV_GPU_ARCHITECTURE_ID expectedArchId; NV_GPU_ARCH_IMPLEMENTATION_ID expectedImplId;};
        auto args = GENERATE(
                Data{VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME, NV_GPU_ARCHITECTURE_GA100, NV_GPU_ARCH_IMPLEMENTATION_GA102},
                Data{VK_NV_SHADING_RATE_IMAGE_EXTENSION_NAME, NV_GPU_ARCHITECTURE_TU100, NV_GPU_ARCH_IMPLEMENTATION_TU102},
                Data{VK_NVX_IMAGE_VIEW_HANDLE_EXTENSION_NAME, NV_GPU_ARCHITECTURE_GV100, NV_GPU_ARCH_IMPLEMENTATION_GV100},
                Data{VK_NV_CLIP_SPACE_W_SCALING_EXTENSION_NAME, NV_GPU_ARCHITECTURE_GP100, NV_GPU_ARCH_IMPLEMENTATION_GP102},
                Data{VK_NV_VIEWPORT_ARRAY2_EXTENSION_NAME, NV_GPU_ARCHITECTURE_GM200, NV_GPU_ARCH_IMPLEMENTATION_GM204},
                Data{"ext", NV_GPU_ARCHITECTURE_GK100, NV_GPU_ARCH_IMPLEMENTATION_GK104});

        ALLOW_CALL(*vulkan, GetDeviceExtensions(_, _)) // NOLINT(bugprone-use-after-move)
            .RETURN(std::set<std::string>{
                VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME,
                args.extensionName});
        ALLOW_CALL(*vulkan, GetPhysicalDeviceProperties2(_, _, _))
            .SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [&args](auto props, auto idProps, auto pciBusInfoProps, auto driverProps, auto fragmentShadingRateProps) {
                        driverProps->driverID = VK_DRIVER_ID_NVIDIA_PROPRIETARY;
                        if (args.extensionName == VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME)
                            fragmentShadingRateProps->primitiveFragmentShadingRateWithMultipleViewports = VK_TRUE;
                    })
            );

        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(0, &handle) == NVAPI_OK);

        NV_GPU_ARCH_INFO archInfo;
        archInfo.version = NV_GPU_ARCH_INFO_VER_2;
        REQUIRE(NvAPI_GPU_GetArchInfo(handle, &archInfo) == NVAPI_OK);
        REQUIRE(archInfo.architecture_id == args.expectedArchId);
        REQUIRE(archInfo.implementation_id == args.expectedImplId);
        REQUIRE(archInfo.revision_id == NV_GPU_CHIP_REV_A01);
    }

    SECTION("GetArchInfo returns device-not-found when no NVIDIA device is present") {
        ALLOW_CALL(*vulkan, GetDeviceExtensions(_, _)) // NOLINT(bugprone-use-after-move)
            .RETURN(std::set<std::string>{
                VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME});
        ALLOW_CALL(*vulkan, GetPhysicalDeviceProperties2(_, _, _))
            .SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [](auto props, auto idProps, auto pciBusInfoProps, auto driverProps, auto fragmentShadingRateProps) {
                        driverProps->driverID = VK_DRIVER_ID_MESA_RADV;
                    })
            );

        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(0, &handle) == NVAPI_OK);

        NV_GPU_ARCH_INFO archInfo;
        archInfo.version = NV_GPU_ARCH_INFO_VER_2;
        REQUIRE(NvAPI_GPU_GetArchInfo(handle, &archInfo) == NVAPI_NVIDIA_DEVICE_NOT_FOUND);
    }
}

TEST_CASE("GetHdrCapabilities returns OK", "[sysinfo]") {
    NV_HDR_CAPABILITIES capabilities;
    capabilities.version = NV_HDR_CAPABILITIES_VER2;
    REQUIRE(NvAPI_Disp_GetHdrCapabilities(0, &capabilities) == NVAPI_OK);
    REQUIRE(capabilities.isST2084EotfSupported == false);
    REQUIRE(capabilities.isTraditionalHdrGammaSupported == false);
    REQUIRE(capabilities.isEdrSupported == false);
    REQUIRE(capabilities.driverExpandDefaultHdrParameters == false);
    REQUIRE(capabilities.isTraditionalSdrGammaSupported == false);
    REQUIRE(capabilities.isDolbyVisionSupported == false);
}

TEST_CASE("GetDisplayViewportsByResolution returns mosaic-ot-active", "[sysinfo]") {
    NvU8 corrected;
    NV_RECT rect[NV_MOSAIC_MAX_DISPLAYS];
    REQUIRE(NvAPI_Mosaic_GetDisplayViewportsByResolution(0, 0, 0, rect, &corrected) == NVAPI_MOSAIC_NOT_ACTIVE);
}
