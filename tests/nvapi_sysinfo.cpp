#include "nvapi_tests_private.h"
#include "nvapi/resource_factory_util.h"

using namespace trompeloeil;
using namespace Catch::Matchers;

TEST_CASE("GetInterfaceVersionString returns OK", "[.sysinfo]") {
    NvAPI_ShortString desc;
    REQUIRE(NvAPI_GetInterfaceVersionString(desc) == NVAPI_OK);
    REQUIRE_THAT(desc, Equals("NVAPI Open Source Interface (DXVK-NVAPI)"));
}

TEST_CASE("GetErrorMessage returns OK", "[.sysinfo]") {
    NvAPI_ShortString desc;
    REQUIRE(NvAPI_GetErrorMessage(NVAPI_NVIDIA_DEVICE_NOT_FOUND, desc) == NVAPI_OK);
    REQUIRE_THAT(desc, Equals("NVAPI_NVIDIA_DEVICE_NOT_FOUND"));
}

TEST_CASE("Initialize succeeds", "[.sysinfo]") {
    auto dxgiFactory = std::make_unique<DXGIDxvkFactoryMock>();
    auto vk = std::make_unique<VkMock>();
    auto nvml = std::make_unique<NvmlMock>();
    DXGIDxvkAdapterMock* adapter = CreateDXGIDxvkAdapterMock();
    DXGIOutput6Mock* output = CreateDXGIOutput6Mock();

    auto e = ConfigureDefaultTestEnvironment(*dxgiFactory, *vk, *nvml, *adapter, *output);

    SECTION("Initialize returns OK") {
        SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);
        REQUIRE(NvAPI_Unload() == NVAPI_OK);
    }

    SECTION("Initialize returns device-not-found when DXVK reports no adapters") {
        ALLOW_CALL(*dxgiFactory, EnumAdapters1(_, _))
            .RETURN(DXGI_ERROR_NOT_FOUND);

        SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_NVIDIA_DEVICE_NOT_FOUND);
        REQUIRE(NvAPI_Unload() == NVAPI_API_NOT_INITIALIZED);
    }

    SECTION("Initialize returns device-not-found when DXVK NVAPI hack is enabled") {
        ::SetEnvironmentVariableA("DXVK_ENABLE_NVAPI", "0");

        ALLOW_CALL(*adapter, GetDesc1(_))
            .SIDE_EFFECT(_1->VendorId = 0x1002)
            .RETURN(S_OK);

        SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_NVIDIA_DEVICE_NOT_FOUND);
        REQUIRE(NvAPI_Unload() == NVAPI_API_NOT_INITIALIZED);
    }

    SECTION("Initialize returns OK when DXVK reports other vendor but DXVK_ENABLE_NVAPI is set") {
        struct Data {
            VkDriverId driverId;
        };
        auto args = GENERATE(
            Data{VK_DRIVER_ID_NVIDIA_PROPRIETARY},
            Data{VK_DRIVER_ID_MESA_NVK});

        ::SetEnvironmentVariableA("DXVK_ENABLE_NVAPI", "1");

        ALLOW_CALL(*adapter, GetDesc1(_))
            .SIDE_EFFECT(_1->VendorId = 0x1002)
            .RETURN(S_OK);
        ALLOW_CALL(*vk, GetPhysicalDeviceProperties2(_, _, _))
            .SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [&args](auto vkProps) {
                        vkProps.driverProps->driverID = args.driverId;
                    }));

        SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));

        REQUIRE(NvAPI_Initialize() == NVAPI_OK);
        REQUIRE(NvAPI_Unload() == NVAPI_OK);
    }

    SECTION("Initialize returns Ok when adapter with Mesa NVK driver ID has been found") {
        ALLOW_CALL(*vk, GetPhysicalDeviceProperties2(_, _, _))
            .SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [](auto vkProps) {
                        vkProps.driverProps->driverID = VK_DRIVER_ID_MESA_NVK;
                    }));

        SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);
        REQUIRE(NvAPI_Unload() == NVAPI_OK);
    }

    SECTION("Initialize returns device-not-found when adapter with non NVIDIA driver ID has been found") {
        ALLOW_CALL(*vk, GetPhysicalDeviceProperties2(_, _, _))
            .SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [](auto vkProps) {
                        vkProps.driverProps->driverID = VK_DRIVER_ID_MESA_RADV;
                    }));

        SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_NVIDIA_DEVICE_NOT_FOUND);
        REQUIRE(NvAPI_Unload() == NVAPI_API_NOT_INITIALIZED);
    }
}

TEST_CASE("Sysinfo methods succeed", "[.sysinfo]") {
    auto dxgiFactory = std::make_unique<DXGIDxvkFactoryMock>();
    auto vk = std::make_unique<VkMock>();
    auto nvml = std::make_unique<NvmlMock>();
    DXGIDxvkAdapterMock* adapter = CreateDXGIDxvkAdapterMock();
    DXGIOutput6Mock* output = CreateDXGIOutput6Mock();

    auto e = ConfigureDefaultTestEnvironment(*dxgiFactory, *vk, *nvml, *adapter, *output);
    auto primaryDisplayId = 0x00010001;

    SECTION("Initialize and unloads return OK") {
        SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);
        REQUIRE(NvAPI_Unload() == NVAPI_OK);
    }

    SECTION("GetDriverAndBranchVersion returns OK") {
        ALLOW_CALL(*vk, GetPhysicalDeviceProperties2(_, _, _))
            .SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [](auto vkProps) {
                        vkProps.driverProps->driverID = VK_DRIVER_ID_NVIDIA_PROPRIETARY;
                        vkProps.props->driverVersion = (470 << 22) | (35 << 14) | 1 << 6;
                    }));

        SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvU32 version;
        NvAPI_ShortString branch;
        REQUIRE(NvAPI_SYS_GetDriverAndBranchVersion(&version, branch) == NVAPI_OK);
        REQUIRE(version == 47035);
        REQUIRE_THAT(branch, StartsWith("r"));
    }

    SECTION("GetDisplayDriverInfo succeeds") {
        ALLOW_CALL(*vk, GetPhysicalDeviceProperties2(_, _, _))
            .SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [](auto vkProps) {
                        vkProps.driverProps->driverID = VK_DRIVER_ID_NVIDIA_PROPRIETARY;
                        vkProps.props->driverVersion = (470 << 22) | (35 << 14) | 1 << 6;
                    }));

        SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        SECTION("GetDisplayDriverInfo (V1) returns OK") {
            NV_DISPLAY_DRIVER_INFO_V1 info;
            info.version = NV_DISPLAY_DRIVER_INFO_VER1;
            REQUIRE(NvAPI_SYS_GetDisplayDriverInfo(reinterpret_cast<NV_DISPLAY_DRIVER_INFO*>(&info)) == NVAPI_OK);
            REQUIRE(info.driverVersion == 47035);
            REQUIRE(info.bIsDCHDriver == 1);
            REQUIRE(info.bIsNVIDIAGameReadyPackage == 1);
            REQUIRE_THAT(info.szBuildBranch, StartsWith("r"));
        }

        SECTION("GetDisplayDriverInfo (V2) returns OK") {
            NV_DISPLAY_DRIVER_INFO_V2 info;
            info.version = NV_DISPLAY_DRIVER_INFO_VER2;
            REQUIRE(NvAPI_SYS_GetDisplayDriverInfo(&info) == NVAPI_OK);
            REQUIRE(info.driverVersion == 47035);
            REQUIRE(info.bIsDCHDriver == 1);
            REQUIRE(info.bIsNVIDIAGameReadyPackage == 1);
            REQUIRE_THAT(info.szBuildBranch, StartsWith("r"));
            REQUIRE_THAT(info.szBuildBaseBranch, StartsWith("r"));
        }

        SECTION("GetDisplayDriverInfo with unknown struct version returns incompatible-struct-version") {
            NV_DISPLAY_DRIVER_INFO info;
            info.version = NV_DISPLAY_DRIVER_INFO_VER2 + 1;
            REQUIRE(NvAPI_SYS_GetDisplayDriverInfo(&info) == NVAPI_INCOMPATIBLE_STRUCT_VERSION);
        }

        SECTION("GetDisplayDriverInfo with current struct version returns not incompatible-struct-version") {
            // This test should fail when a header update provides a newer not yet implemented struct version
            NV_DISPLAY_DRIVER_INFO info;
            info.version = NV_DISPLAY_DRIVER_INFO_VER;
            REQUIRE(NvAPI_SYS_GetDisplayDriverInfo(&info) != NVAPI_INCOMPATIBLE_STRUCT_VERSION);
        }
    }

    SECTION("GetDisplayDriverVersion returns OK") {
        struct Data {
            VkDriverId driverId;
            uint16_t major;
            uint16_t minor;
            uint16_t patch;
            uint32_t expectedVersion;
        };
        auto args = GENERATE(
            Data{VK_DRIVER_ID_NVIDIA_PROPRIETARY, 470, 45, 1, 47045},
            Data{VK_DRIVER_ID_NVIDIA_PROPRIETARY, 470, 101, 1, 47099},
            Data{VK_DRIVER_ID_AMD_OPEN_SOURCE, 21, 2, 3, 99999});

        ::SetEnvironmentVariableA("DXVK_NVAPI_ALLOW_OTHER_DRIVERS", "1");

        ALLOW_CALL(*vk, GetPhysicalDeviceProperties2(_, _, _))
            .SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [&args](auto vkProps) {
                        vkProps.driverProps->driverID = args.driverId;
                        strcpy(vkProps.props->deviceName, "GPU0");
                        if (args.driverId == VK_DRIVER_ID_NVIDIA_PROPRIETARY)
                            vkProps.props->driverVersion = (args.major << 22) | (args.minor << 14) | (args.patch << 6);
                        else
                            vkProps.props->driverVersion = (args.major << 22) | (args.minor << 12) | args.patch;
                    }));

        SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvDisplayHandle handle;
        REQUIRE(NvAPI_EnumNvidiaDisplayHandle(0, &handle) == NVAPI_OK);

        NV_DISPLAY_DRIVER_VERSION version;
        version.version = NV_DISPLAY_DRIVER_VERSION_VER;
        REQUIRE(NvAPI_GetDisplayDriverVersion(handle, &version) == NVAPI_OK);
        REQUIRE(version.drvVersion == args.expectedVersion);
        REQUIRE_THAT(version.szAdapterString, Equals("GPU0"));
        REQUIRE_THAT(version.szBuildBranchString, StartsWith("r"));
    }

    SECTION("GetDisplayDriverVersion with version override returns OK") {
        struct Data {
            std::string override;
            uint32_t expectedVersion;
        };
        auto args = GENERATE(
            Data{"", 47045},
            Data{"0", 47045},
            Data{"99", 47045},
            Data{"100000", 47045},
            Data{"AB39976", 47045},
            Data{"39976AB", 47045},
            Data{"39976", 39976});

        ::SetEnvironmentVariableA("DXVK_NVAPI_DRIVER_VERSION", args.override.c_str());

        ALLOW_CALL(*vk, GetPhysicalDeviceProperties2(_, _, _))
            .SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [](auto vkProps) {
                        vkProps.driverProps->driverID = VK_DRIVER_ID_NVIDIA_PROPRIETARY;
                        vkProps.props->driverVersion = (470 << 22) | (45 << 14) | (0 << 6);
                    }));

        SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvDisplayHandle handle;
        REQUIRE(NvAPI_EnumNvidiaDisplayHandle(0, &handle) == NVAPI_OK);

        NV_DISPLAY_DRIVER_VERSION version;
        version.version = NV_DISPLAY_DRIVER_VERSION_VER;
        REQUIRE(NvAPI_GetDisplayDriverVersion(handle, &version) == NVAPI_OK);
        REQUIRE(version.drvVersion == args.expectedVersion);
    }

    SECTION("GetGPUIDFromPhysicalGPU / GetPhysicalGPUFromGPUID succeeds") {
        ALLOW_CALL(*vk, GetDeviceExtensions(_, _))
            .RETURN(std::set<std::string>{VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME, VK_EXT_PCI_BUS_INFO_EXTENSION_NAME});
        ALLOW_CALL(*vk, GetPhysicalDeviceProperties2(_, _, _))
            .LR_SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [](auto vkProps) {
                        vkProps.driverProps->driverID = VK_DRIVER_ID_NVIDIA_PROPRIETARY;
                        vkProps.pciBusInfoProps->pciDomain = 0x01;
                        vkProps.pciBusInfoProps->pciBus = 0x02;
                        vkProps.pciBusInfoProps->pciDevice = 0x03;
                    }));

        SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(primaryDisplayId, &handle) == NVAPI_OK);

        SECTION("GetGPUIDFromPhysicalGPU return OK") {
            NvU32 gpuId;
            REQUIRE(NvAPI_GetGPUIDfromPhysicalGPU(handle, &gpuId) == NVAPI_OK);
            REQUIRE(gpuId == 0x10203);
        }

        SECTION("GetPhysicalGPUFromGPUID return OK") {
            NvPhysicalGpuHandle handleFromGpuId;
            REQUIRE(NvAPI_GetPhysicalGPUFromGPUID(0x10203, &handleFromGpuId) == NVAPI_OK);
            REQUIRE(handleFromGpuId == handle);
        }
    }

    SECTION("CudaEnumComputeCapableGpus succeeds") {
        struct Data {
            VkDriverId driverId;
            std::string extensionName;
            uint32_t cudaGpuCount;
        };
        auto args = GENERATE(
            Data{VK_DRIVER_ID_NVIDIA_PROPRIETARY, VK_NV_VIEWPORT_ARRAY2_EXTENSION_NAME, 1},
            Data{VK_DRIVER_ID_MESA_RADV, VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME, 0},
            Data{VK_DRIVER_ID_NVIDIA_PROPRIETARY, "ext", 0});

        ::SetEnvironmentVariableA("DXVK_NVAPI_ALLOW_OTHER_DRIVERS", "1");

        ALLOW_CALL(*vk, GetDeviceExtensions(_, _))
            .RETURN(std::set<std::string>{
                VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME,
                args.extensionName});
        ALLOW_CALL(*vk, GetPhysicalDeviceProperties2(_, _, _))
            .SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [args](auto vkProps) {
                        vkProps.driverProps->driverID = args.driverId;
                        if (args.extensionName == VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME)
                            vkProps.fragmentShadingRateProps->primitiveFragmentShadingRateWithMultipleViewports = VK_TRUE;
                    }));

        SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(primaryDisplayId, &handle) == NVAPI_OK);

        SECTION("CudaEnumComputeCapableGpus (V1) returns OK") {
            NV_COMPUTE_GPU_TOPOLOGY_V1 gpuTopology;
            gpuTopology.version = NV_COMPUTE_GPU_TOPOLOGY_VER1;
            REQUIRE(NvAPI_GPU_CudaEnumComputeCapableGpus(reinterpret_cast<NV_COMPUTE_GPU_TOPOLOGY*>(&gpuTopology)) == NVAPI_OK);
            REQUIRE(gpuTopology.gpuCount == args.cudaGpuCount);
            if (gpuTopology.gpuCount == 1) {
                REQUIRE(gpuTopology.computeGpus[0].hPhysicalGpu == handle);
                REQUIRE(gpuTopology.computeGpus[0].flags == 0x0b);
            }
        }

        SECTION("CudaEnumComputeCapableGpus (V2) returns incompatible-struct-version") {
            NV_COMPUTE_GPU_TOPOLOGY_V2 gpuTopology;
            gpuTopology.version = NV_COMPUTE_GPU_TOPOLOGY_VER;
            REQUIRE(NvAPI_GPU_CudaEnumComputeCapableGpus(&gpuTopology) == NVAPI_INCOMPATIBLE_STRUCT_VERSION);
        }
    }

    SECTION("GetGPUType returns OK") {
        ALLOW_CALL(*vk, GetPhysicalDeviceProperties2(_, _, _))
            .SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [](auto vkProps) {
                        vkProps.driverProps->driverID = VK_DRIVER_ID_NVIDIA_PROPRIETARY;
                        vkProps.props->deviceType = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
                    }));

        SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(primaryDisplayId, &handle) == NVAPI_OK);

        NV_GPU_TYPE type;
        REQUIRE(NvAPI_GPU_GetGPUType(handle, &type) == NVAPI_OK);
        REQUIRE(type == NV_SYSTEM_TYPE_DGPU);
    }

    SECTION("GetSystemType returns OK") {
        SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(primaryDisplayId, &handle) == NVAPI_OK);

        NV_SYSTEM_TYPE type;
        REQUIRE(NvAPI_GPU_GetSystemType(handle, &type) == NVAPI_OK);
        REQUIRE(type == NV_SYSTEM_TYPE_UNKNOWN);
    }

    SECTION("GetPCIIdentifiers returns OK") {
        ALLOW_CALL(*adapter, GetDesc1(_))
            .SIDE_EFFECT({
                _1->VendorId = 0x10de;
                _1->DeviceId = 0x1234;
            })
            .RETURN(S_OK);

        SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(primaryDisplayId, &handle) == NVAPI_OK);

        NvU32 deviceId, subSystemId, revisionId, extDeviceId;
        REQUIRE(NvAPI_GPU_GetPCIIdentifiers(handle, &deviceId, &subSystemId, &revisionId, &extDeviceId) == NVAPI_OK);
        REQUIRE(deviceId == 0x123410de);
        REQUIRE(subSystemId == 0x0);
        REQUIRE(revisionId == 0x0);
        REQUIRE(extDeviceId == 0x00001234);
    }

    SECTION("GetFullName returns OK") {
        auto name = "High-End GPU01";
        ALLOW_CALL(*vk, GetPhysicalDeviceProperties2(_, _, _))
            .LR_SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [&name](auto vkProps) {
                        vkProps.driverProps->driverID = VK_DRIVER_ID_NVIDIA_PROPRIETARY;
                        strcpy(vkProps.props->deviceName, name);
                    }));

        SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(primaryDisplayId, &handle) == NVAPI_OK);

        NvAPI_ShortString fullName;
        REQUIRE(NvAPI_GPU_GetFullName(handle, fullName) == NVAPI_OK);
        REQUIRE_THAT(fullName, Equals(name));
    }

    SECTION("GetBusId returns OK") {
        auto id = 2U;
        ALLOW_CALL(*vk, GetDeviceExtensions(_, _))
            .RETURN(std::set<std::string>{VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME, VK_EXT_PCI_BUS_INFO_EXTENSION_NAME});
        ALLOW_CALL(*vk, GetPhysicalDeviceProperties2(_, _, _))
            .LR_SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [&id](auto vkProps) {
                        vkProps.driverProps->driverID = VK_DRIVER_ID_NVIDIA_PROPRIETARY;
                        vkProps.pciBusInfoProps->pciBus = id;
                    }));

        SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(primaryDisplayId, &handle) == NVAPI_OK);

        NvU32 busId;
        REQUIRE(NvAPI_GPU_GetBusId(handle, &busId) == NVAPI_OK);
        REQUIRE(busId == id);
    }

    SECTION("GetBusSlotId returns OK") {
        auto id = 3U;
        ALLOW_CALL(*vk, GetDeviceExtensions(_, _))
            .RETURN(std::set<std::string>{VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME, VK_EXT_PCI_BUS_INFO_EXTENSION_NAME});
        ALLOW_CALL(*vk, GetPhysicalDeviceProperties2(_, _, _))
            .LR_SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [&id](auto vkProps) {
                        vkProps.driverProps->driverID = VK_DRIVER_ID_NVIDIA_PROPRIETARY;
                        vkProps.pciBusInfoProps->pciDevice = id;
                    }));

        SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(primaryDisplayId, &handle) == NVAPI_OK);

        NvU32 busSlotId;
        REQUIRE(NvAPI_GPU_GetBusSlotId(handle, &busSlotId) == NVAPI_OK);
        REQUIRE(busSlotId == id);
    }

    SECTION("GetBusType returns OK") {
        struct Data {
            std::string extensionName;
            NV_GPU_BUS_TYPE expectedBusType;
        };
        auto args = GENERATE(
            Data{VK_NV_VIEWPORT_ARRAY2_EXTENSION_NAME, NVAPI_GPU_BUS_TYPE_PCI_EXPRESS},
            Data{"ext", NVAPI_GPU_BUS_TYPE_UNDEFINED});

        ALLOW_CALL(*vk, GetDeviceExtensions(_, _))
            .RETURN(std::set<std::string>{
                VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME,
                args.extensionName});
        ALLOW_CALL(*vk, GetPhysicalDeviceProperties2(_, _, _))
            .SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [](auto vkProps) {
                        vkProps.driverProps->driverID = VK_DRIVER_ID_NVIDIA_PROPRIETARY;
                    }));

        SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(primaryDisplayId, &handle) == NVAPI_OK);

        NV_GPU_BUS_TYPE type;
        REQUIRE(NvAPI_GPU_GetBusType(handle, &type) == NVAPI_OK);
        REQUIRE(type == args.expectedBusType);
    }

    SECTION("GetPhysicalFrameBufferSize returns OK") {
        ALLOW_CALL(*adapter, GetDesc1(_))
            .SIDE_EFFECT({
                _1->VendorId = 0x10de;
                _1->DedicatedVideoMemory = 8191 * 1024;
            })
            .RETURN(S_OK);

        SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(primaryDisplayId, &handle) == NVAPI_OK);

        NvU32 size;
        REQUIRE(NvAPI_GPU_GetPhysicalFrameBufferSize(handle, &size) == NVAPI_OK);
        REQUIRE(size == 8191);
    }

    SECTION("GetVirtualFrameBufferSize returns OK") {
        ALLOW_CALL(*adapter, GetDesc1(_))
            .SIDE_EFFECT({
                _1->VendorId = 0x10de;
                _1->DedicatedVideoMemory = 8191 * 1024;
                _1->DedicatedSystemMemory = 1014 * 1024;
            })
            .RETURN(S_OK);

        SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(primaryDisplayId, &handle) == NVAPI_OK);

        NvU32 size;
        REQUIRE(NvAPI_GPU_GetVirtualFrameBufferSize(handle, &size) == NVAPI_OK);
        REQUIRE(size == 9205);
    }

    SECTION("GetMemoryInfo succeeds") {
        ALLOW_CALL(*adapter, GetDesc1(_))
            .SIDE_EFFECT({
                _1->VendorId = 0x10de;
                _1->DedicatedVideoMemory = 8191 * 1024;
                _1->DedicatedSystemMemory = 1014 * 1024;
                _1->SharedSystemMemory = 16382 * 1024;
            })
            .RETURN(S_OK);
        ALLOW_CALL(*adapter, QueryVideoMemoryInfo(_, _, _))
            .SIDE_EFFECT({
                _3->Budget = 4096 * 1024;
                _3->CurrentUsage = 1024 * 1024;
            })
            .RETURN(S_OK);

        SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(primaryDisplayId, &handle) == NVAPI_OK);

        SECTION("GetMemoryInfo (V1) returns OK") {
            NV_DISPLAY_DRIVER_MEMORY_INFO_V1 info;
            info.version = NV_DISPLAY_DRIVER_MEMORY_INFO_VER_1;
            REQUIRE(NvAPI_GPU_GetMemoryInfo(handle, reinterpret_cast<NV_DISPLAY_DRIVER_MEMORY_INFO*>(&info)) == NVAPI_OK);
            REQUIRE(info.dedicatedVideoMemory == 8191);
            REQUIRE(info.systemVideoMemory == 1014);
            REQUIRE(info.sharedSystemMemory == 16382);
            REQUIRE(info.availableDedicatedVideoMemory == 8191);
        }

        SECTION("GetMemoryInfo (V2) returns OK") {
            NV_DISPLAY_DRIVER_MEMORY_INFO_V2 info;
            info.version = NV_DISPLAY_DRIVER_MEMORY_INFO_VER_2;
            REQUIRE(NvAPI_GPU_GetMemoryInfo(handle, reinterpret_cast<NV_DISPLAY_DRIVER_MEMORY_INFO*>(&info)) == NVAPI_OK);
            REQUIRE(info.dedicatedVideoMemory == 8191);
            REQUIRE(info.systemVideoMemory == 1014);
            REQUIRE(info.sharedSystemMemory == 16382);
            REQUIRE(info.availableDedicatedVideoMemory == 8191);
            REQUIRE(info.curAvailableDedicatedVideoMemory == 4096);
        }

        SECTION("GetMemoryInfo (V3) returns OK") {
            NV_DISPLAY_DRIVER_MEMORY_INFO_V3 info;
            info.version = NV_DISPLAY_DRIVER_MEMORY_INFO_VER_3;
            REQUIRE(NvAPI_GPU_GetMemoryInfo(handle, &info) == NVAPI_OK);
            REQUIRE(info.dedicatedVideoMemory == 8191);
            REQUIRE(info.systemVideoMemory == 1014);
            REQUIRE(info.sharedSystemMemory == 16382);
            REQUIRE(info.availableDedicatedVideoMemory == 8191);
            REQUIRE(info.curAvailableDedicatedVideoMemory == 4096);
            REQUIRE(info.dedicatedVideoMemoryEvictionsSize == 0);
            REQUIRE(info.dedicatedVideoMemoryEvictionCount == 0);
        }

        SECTION("GetMemoryInfo with unknown struct version returns incompatible-struct-version") {
            NV_DISPLAY_DRIVER_MEMORY_INFO info;
            info.version = NV_DISPLAY_DRIVER_MEMORY_INFO_VER_3 + 1;
            REQUIRE(NvAPI_GPU_GetMemoryInfo(handle, &info) == NVAPI_INCOMPATIBLE_STRUCT_VERSION);
        }

        SECTION("GetMemoryInfo with current struct version returns not incompatible-struct-version") {
            NV_DISPLAY_DRIVER_MEMORY_INFO info;
            info.version = NV_DISPLAY_DRIVER_MEMORY_INFO_VER;
            REQUIRE(NvAPI_GPU_GetMemoryInfo(handle, &info) != NVAPI_INCOMPATIBLE_STRUCT_VERSION);
        }
    }

    SECTION("GetMemoryInfoEx succeeds") {
        ALLOW_CALL(*adapter, GetDesc1(_))
            .SIDE_EFFECT({
                _1->VendorId = 0x10de;
                _1->DedicatedVideoMemory = 8191 * 1024;
                _1->DedicatedSystemMemory = 1014 * 1024;
                _1->SharedSystemMemory = 16382 * 1024;
            })
            .RETURN(S_OK);
        ALLOW_CALL(*adapter, QueryVideoMemoryInfo(_, _, _))
            .SIDE_EFFECT({
                _3->Budget = 4096 * 1024;
                _3->CurrentUsage = 1024 * 1024;
            })
            .RETURN(S_OK);

        SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(primaryDisplayId, &handle) == NVAPI_OK);

        SECTION("GetMemoryInfoEx returns OK") {
            NV_GPU_MEMORY_INFO_EX_V1 info;
            info.version = NV_GPU_MEMORY_INFO_EX_VER_1;
            REQUIRE(NvAPI_GPU_GetMemoryInfoEx(handle, &info) == NVAPI_OK);
            REQUIRE(info.dedicatedVideoMemory == 8191 * 1024);
            REQUIRE(info.systemVideoMemory == 1014 * 1024);
            REQUIRE(info.sharedSystemMemory == 16382 * 1024);
            REQUIRE(info.availableDedicatedVideoMemory == 8191 * 1024);
            REQUIRE(info.curAvailableDedicatedVideoMemory == 4096 * 1024);
            REQUIRE(info.dedicatedVideoMemoryEvictionsSize == 0);
            REQUIRE(info.dedicatedVideoMemoryEvictionCount == 0);
            REQUIRE(info.dedicatedVideoMemoryPromotionsSize == 0);
            REQUIRE(info.dedicatedVideoMemoryPromotionCount == 0);
        }

        SECTION("GetMemoryInfoEx with unknown struct version returns incompatible-struct-version") {
            NV_GPU_MEMORY_INFO_EX info;
            info.version = NV_GPU_MEMORY_INFO_EX_VER_1 + 1;
            REQUIRE(NvAPI_GPU_GetMemoryInfoEx(handle, &info) == NVAPI_INCOMPATIBLE_STRUCT_VERSION);
        }

        SECTION("GetMemoryInfoEx with current struct version returns not incompatible-struct-version") {
            NV_GPU_MEMORY_INFO_EX info;
            info.version = NV_GPU_MEMORY_INFO_EX_VER;
            REQUIRE(NvAPI_GPU_GetMemoryInfoEx(handle, &info) != NVAPI_INCOMPATIBLE_STRUCT_VERSION);
        }
    }

    SECTION("GetAdapterIdFromPhysicalGpu returns OK") {
        ALLOW_CALL(*vk, GetPhysicalDeviceProperties2(_, _, _))
            .SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [](auto vkProps) {
                        auto luid = LUID{0x04030211, 0x08070655};
                        memcpy(&vkProps.idProps->deviceLUID, &luid, sizeof(luid));
                        vkProps.idProps->deviceLUIDValid = VK_TRUE;
                        vkProps.driverProps->driverID = VK_DRIVER_ID_NVIDIA_PROPRIETARY;
                    }));

        SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(primaryDisplayId, &handle) == NVAPI_OK);

        LUID luid;
        REQUIRE(NvAPI_GPU_GetAdapterIdFromPhysicalGpu(handle, static_cast<void*>(&luid)) == NVAPI_OK);
        REQUIRE(luid.HighPart == 0x08070655);
        REQUIRE(luid.LowPart == 0x04030211);
    }

    SECTION("GetLogicalGpuInfo succeeds") {
        ALLOW_CALL(*vk, GetPhysicalDeviceProperties2(_, _, _))
            .SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [](auto vkProps) {
                        auto luid = LUID{0x04030211, 0x08070655};
                        memcpy(&vkProps.idProps->deviceLUID, &luid, sizeof(luid));
                        vkProps.idProps->deviceLUIDValid = VK_TRUE;
                        vkProps.driverProps->driverID = VK_DRIVER_ID_NVIDIA_PROPRIETARY;
                    }));

        SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvU32 count;
        NvLogicalGpuHandle handles[NVAPI_MAX_LOGICAL_GPUS]{};
        REQUIRE(NvAPI_EnumLogicalGPUs(handles, &count) == NVAPI_OK);
        REQUIRE(count == 1);

        NvPhysicalGpuHandle physicalHandles[NVAPI_MAX_PHYSICAL_GPUS]{};
        REQUIRE(NvAPI_GetPhysicalGPUsFromLogicalGPU(handles[0], physicalHandles, &count) == NVAPI_OK);
        REQUIRE(count == 1);

        SECTION("GetLogicalGpuInfo returns OK") {
            LUID luid;
            NV_LOGICAL_GPU_DATA data;
            data.pOSAdapterId = static_cast<void*>(&luid);
            data.version = NV_LOGICAL_GPU_DATA_VER1;
            REQUIRE(NvAPI_GPU_GetLogicalGpuInfo(handles[0], &data) == NVAPI_OK);
            REQUIRE(data.physicalGpuCount == 1);
            REQUIRE(data.physicalGpuHandles[0] == physicalHandles[0]);
            REQUIRE(luid.HighPart == 0x08070655);
            REQUIRE(luid.LowPart == 0x04030211);
        }

        SECTION("GetLogicalGpuInfo without set os-adapter-id returns invalid-argument") {
            NV_LOGICAL_GPU_DATA data{};
            data.version = NV_LOGICAL_GPU_DATA_VER1;
            REQUIRE(NvAPI_GPU_GetLogicalGpuInfo(handles[0], &data) == NVAPI_INVALID_ARGUMENT);
        }

        SECTION("GetLogicalGpuInfo with unknown struct version returns incompatible-struct-version") {
            NV_LOGICAL_GPU_DATA data;
            data.version = NV_LOGICAL_GPU_DATA_VER1 + 1;
            REQUIRE(NvAPI_GPU_GetLogicalGpuInfo(handles[0], &data) == NVAPI_INCOMPATIBLE_STRUCT_VERSION);
        }

        SECTION("GetLogicalGpuInfo with current struct version returns not incompatible-struct-version") {
            // This test should fail when a header update provides a newer not yet implemented struct version
            LUID luid;
            NV_LOGICAL_GPU_DATA data;
            data.pOSAdapterId = static_cast<void*>(&luid);
            data.version = NV_LOGICAL_GPU_DATA_VER;
            REQUIRE(NvAPI_GPU_GetLogicalGpuInfo(handles[0], &data) != NVAPI_INCOMPATIBLE_STRUCT_VERSION);
        }
    }

    SECTION("GetArchInfo returns OK") {
        struct Data {
            VkDriverId driverId;
            uint32_t deviceId;
            std::string extensionName;
            uint32_t maxFramebufferHeight;
            NV_GPU_ARCHITECTURE_ID expectedArchId;
            NV_GPU_ARCH_IMPLEMENTATION_ID expectedImplId;
        };
        auto args = GENERATE(
            Data{VK_DRIVER_ID_NVIDIA_PROPRIETARY, 0x2600, VK_KHR_COMPUTE_SHADER_DERIVATIVES_EXTENSION_NAME, 0x8000, NV_GPU_ARCHITECTURE_GB200, NV_GPU_ARCH_IMPLEMENTATION_GB202},
            Data{VK_DRIVER_ID_NVIDIA_PROPRIETARY, 0x2600, VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME, 0x8000, NV_GPU_ARCHITECTURE_AD100, NV_GPU_ARCH_IMPLEMENTATION_AD102},
            Data{VK_DRIVER_ID_NVIDIA_PROPRIETARY, 0x2000, VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME, 0x8000, NV_GPU_ARCHITECTURE_GA100, NV_GPU_ARCH_IMPLEMENTATION_GA102},
            Data{VK_DRIVER_ID_NVIDIA_PROPRIETARY, 0x2000, VK_KHR_FRAGMENT_SHADER_BARYCENTRIC_EXTENSION_NAME, 0x8000, NV_GPU_ARCHITECTURE_TU100, NV_GPU_ARCH_IMPLEMENTATION_TU102},
            Data{VK_DRIVER_ID_NVIDIA_PROPRIETARY, 0x2000, VK_NVX_IMAGE_VIEW_HANDLE_EXTENSION_NAME, 0x8000, NV_GPU_ARCHITECTURE_GV100, NV_GPU_ARCH_IMPLEMENTATION_GV100},
            Data{VK_DRIVER_ID_NVIDIA_PROPRIETARY, 0x2000, VK_NV_CLIP_SPACE_W_SCALING_EXTENSION_NAME, 0x8000, NV_GPU_ARCHITECTURE_GP100, NV_GPU_ARCH_IMPLEMENTATION_GP102},
            Data{VK_DRIVER_ID_NVIDIA_PROPRIETARY, 0x2000, VK_NV_VIEWPORT_ARRAY2_EXTENSION_NAME, 0x4000, NV_GPU_ARCHITECTURE_GM200, NV_GPU_ARCH_IMPLEMENTATION_GM204},
            Data{VK_DRIVER_ID_NVIDIA_PROPRIETARY, 0x2000, VK_EXT_SHADER_IMAGE_ATOMIC_INT64_EXTENSION_NAME, 0x4000, NV_GPU_ARCHITECTURE_GM000, NV_GPU_ARCH_IMPLEMENTATION_GM204},
            Data{VK_DRIVER_ID_NVIDIA_PROPRIETARY, 0x2000, "ext", 0x4000, NV_GPU_ARCHITECTURE_GK100, NV_GPU_ARCH_IMPLEMENTATION_GK104},
            Data{VK_DRIVER_ID_MESA_NVK, 0x2600, VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME, 0x8000, NV_GPU_ARCHITECTURE_AD100, NV_GPU_ARCH_IMPLEMENTATION_AD102},
            Data{VK_DRIVER_ID_MESA_NVK, 0x2000, VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME, 0x8000, NV_GPU_ARCHITECTURE_GA100, NV_GPU_ARCH_IMPLEMENTATION_GA102},
            Data{VK_DRIVER_ID_MESA_NVK, 0x2000, VK_KHR_FRAGMENT_SHADER_BARYCENTRIC_EXTENSION_NAME, 0x8000, NV_GPU_ARCHITECTURE_TU100, NV_GPU_ARCH_IMPLEMENTATION_TU102},
            Data{VK_DRIVER_ID_MESA_NVK, 0x2000, VK_EXT_DEPTH_RANGE_UNRESTRICTED_EXTENSION_NAME, 0x8000, NV_GPU_ARCHITECTURE_GV100, NV_GPU_ARCH_IMPLEMENTATION_GV100},
            Data{VK_DRIVER_ID_MESA_NVK, 0x2000, VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME, 0x8000, NV_GPU_ARCHITECTURE_GP100, NV_GPU_ARCH_IMPLEMENTATION_GP102},
            Data{VK_DRIVER_ID_MESA_NVK, 0x2000, VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME, 0x4000, NV_GPU_ARCHITECTURE_GM200, NV_GPU_ARCH_IMPLEMENTATION_GM204},
            Data{VK_DRIVER_ID_MESA_NVK, 0x2000, VK_EXT_SHADER_IMAGE_ATOMIC_INT64_EXTENSION_NAME, 0x4000, NV_GPU_ARCHITECTURE_GM000, NV_GPU_ARCH_IMPLEMENTATION_GM204},
            Data{VK_DRIVER_ID_MESA_NVK, 0x2000, "ext", 0x4000, NV_GPU_ARCHITECTURE_GK100, NV_GPU_ARCH_IMPLEMENTATION_GK104});

        ALLOW_CALL(*vk, GetDeviceExtensions(_, _))
            .RETURN(std::set<std::string>{
                VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME,
                args.extensionName});
        ALLOW_CALL(*vk, GetPhysicalDeviceProperties2(_, _, _))
            .SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [&args](auto vkProps) {
                        vkProps.props->deviceID = args.deviceId;
                        vkProps.props->limits.maxFramebufferHeight = args.maxFramebufferHeight;
                        vkProps.driverProps->driverID = args.driverId;
                        if (args.extensionName == VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME)
                            vkProps.fragmentShadingRateProps->primitiveFragmentShadingRateWithMultipleViewports = VK_TRUE;
                        if (args.extensionName == VK_KHR_COMPUTE_SHADER_DERIVATIVES_EXTENSION_NAME)
                            vkProps.computeShaderDerivativesProps->meshAndTaskShaderDerivatives = VK_TRUE;
                    }));

        SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(primaryDisplayId, &handle) == NVAPI_OK);

        SECTION("GetArchInfo (V1) returns OK") {
            NV_GPU_ARCH_INFO_V1 archInfo;
            archInfo.version = NV_GPU_ARCH_INFO_VER_1;
            REQUIRE(NvAPI_GPU_GetArchInfo(handle, reinterpret_cast<NV_GPU_ARCH_INFO*>(&archInfo)) == NVAPI_OK);
            REQUIRE(archInfo.architecture == args.expectedArchId);
            REQUIRE(archInfo.implementation == args.expectedImplId);
            REQUIRE(archInfo.revision == NV_GPU_CHIP_REV_UNKNOWN);
        }

        SECTION("GetArchInfo (V2) returns OK") {
            NV_GPU_ARCH_INFO_V2 archInfo;
            archInfo.version = NV_GPU_ARCH_INFO_VER_2;
            REQUIRE(NvAPI_GPU_GetArchInfo(handle, &archInfo) == NVAPI_OK);
            REQUIRE(archInfo.architecture_id == args.expectedArchId);
            REQUIRE(archInfo.implementation_id == args.expectedImplId);
            REQUIRE(archInfo.revision_id == NV_GPU_CHIP_REV_UNKNOWN);
        }
    }

    SECTION("GetArchInfo with unknown struct version returns incompatible-struct-version") {
        SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(primaryDisplayId, &handle) == NVAPI_OK);

        NV_GPU_ARCH_INFO archInfo;
        archInfo.version = NV_GPU_ARCH_INFO_VER_2 + 1;
        REQUIRE(NvAPI_GPU_GetArchInfo(handle, &archInfo) == NVAPI_INCOMPATIBLE_STRUCT_VERSION);
    }

    SECTION("GetArchInfo with current struct version returns not incompatible-struct-version") {
        // This test should fail when a header update provides a newer not yet implemented struct version
        SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(primaryDisplayId, &handle) == NVAPI_OK);

        NV_GPU_ARCH_INFO archInfo;
        archInfo.version = NV_GPU_ARCH_INFO_VER;
        REQUIRE(NvAPI_GPU_GetArchInfo(handle, &archInfo) != NVAPI_INCOMPATIBLE_STRUCT_VERSION);
    }

    SECTION("GetArchInfo spoofs Pascal when a non-NVIDIA device is present") {
        ::SetEnvironmentVariableA("DXVK_NVAPI_ALLOW_OTHER_DRIVERS", "1");

        ALLOW_CALL(*vk, GetPhysicalDeviceProperties2(_, _, _))
            .SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [](auto vkProps) {
                        vkProps.driverProps->driverID = VK_DRIVER_ID_MESA_RADV;
                    }));

        SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(primaryDisplayId, &handle) == NVAPI_OK);

        SECTION("GetArchInfo (V1) returns OK") {
            NV_GPU_ARCH_INFO_V1 archInfo;
            archInfo.version = NV_GPU_ARCH_INFO_VER_1;
            REQUIRE(NvAPI_GPU_GetArchInfo(handle, reinterpret_cast<NV_GPU_ARCH_INFO*>(&archInfo)) == NVAPI_OK);
            REQUIRE(archInfo.architecture == NV_GPU_ARCHITECTURE_GP100);
            REQUIRE(archInfo.implementation == NV_GPU_ARCH_IMPLEMENTATION_GP102);
            REQUIRE(archInfo.revision == NV_GPU_CHIP_REV_UNKNOWN);
        }

        SECTION("GetArchInfo (V2) returns OK") {
            NV_GPU_ARCH_INFO_V2 archInfo;
            archInfo.version = NV_GPU_ARCH_INFO_VER_2;
            REQUIRE(NvAPI_GPU_GetArchInfo(handle, &archInfo) == NVAPI_OK);
            REQUIRE(archInfo.architecture_id == NV_GPU_ARCHITECTURE_GP100);
            REQUIRE(archInfo.implementation_id == NV_GPU_ARCH_IMPLEMENTATION_GP102);
            REQUIRE(archInfo.revision_id == NV_GPU_CHIP_REV_UNKNOWN);
        }
    }

    SECTION("GetGPUInfo returns OK") {
        struct Data {
            VkDriverId driverId;
            uint32_t deviceId;
            std::string extensionName;
            uint32_t expectedRayTracingCores;
            uint32_t expectedTensorCores;
        };
        auto args = GENERATE(
            Data{VK_DRIVER_ID_NVIDIA_PROPRIETARY, 0x2600, VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME, 76, 304},
            Data{VK_DRIVER_ID_NVIDIA_PROPRIETARY, 0x2000, VK_KHR_FRAGMENT_SHADER_BARYCENTRIC_EXTENSION_NAME, 76, 304},
            Data{VK_DRIVER_ID_NVIDIA_PROPRIETARY, 0x2000, VK_NVX_IMAGE_VIEW_HANDLE_EXTENSION_NAME, 0, 0},
            Data{VK_DRIVER_ID_NVIDIA_PROPRIETARY, 0x2000, VK_NV_CLIP_SPACE_W_SCALING_EXTENSION_NAME, 0, 0},
            Data{VK_DRIVER_ID_NVIDIA_PROPRIETARY, 0x2000, VK_NV_VIEWPORT_ARRAY2_EXTENSION_NAME, 0, 0},
            Data{VK_DRIVER_ID_NVIDIA_PROPRIETARY, 0x2000, "ext", 0, 0},
            Data{VK_DRIVER_ID_MESA_NVK, 0x2600, VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME, 76, 304});

        ALLOW_CALL(*vk, GetDeviceExtensions(_, _))
            .RETURN(std::set<std::string>{
                VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME,
                args.extensionName});
        ALLOW_CALL(*vk, GetPhysicalDeviceProperties2(_, _, _))
            .SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [&args](auto vkProps) {
                        vkProps.driverProps->driverID = args.driverId;
                        vkProps.props->deviceID = args.deviceId;
                        if (args.extensionName == VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME)
                            vkProps.fragmentShadingRateProps->primitiveFragmentShadingRateWithMultipleViewports = VK_TRUE;
                    }));

        SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(primaryDisplayId, &handle) == NVAPI_OK);

        SECTION("GetGPUInfo (V1) returns OK") {
            NV_GPU_INFO_V1 gpuInfo;
            gpuInfo.version = NV_GPU_INFO_VER1;
            REQUIRE(NvAPI_GPU_GetGPUInfo(handle, reinterpret_cast<NV_GPU_INFO*>(&gpuInfo)) == NVAPI_OK);
        }

        SECTION("GetGPUInfo (V2) returns OK") {
            NV_GPU_INFO_V2 gpuInfo;
            gpuInfo.version = NV_GPU_INFO_VER2;
            REQUIRE(NvAPI_GPU_GetGPUInfo(handle, &gpuInfo) == NVAPI_OK);
            REQUIRE(gpuInfo.rayTracingCores == args.expectedRayTracingCores);
            REQUIRE(gpuInfo.tensorCores == args.expectedTensorCores);
        }
    }

    SECTION("GetGPUInfo with unknown struct version returns incompatible-struct-version") {
        SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(primaryDisplayId, &handle) == NVAPI_OK);

        NV_GPU_INFO gpuInfo;
        gpuInfo.version = NV_GPU_INFO_VER2 + 1;
        REQUIRE(NvAPI_GPU_GetGPUInfo(handle, &gpuInfo) == NVAPI_INCOMPATIBLE_STRUCT_VERSION);
    }

    SECTION("GetGPUInfo with current struct version returns not incompatible-struct-version") {
        // This test should fail when a header update provides a newer not yet implemented struct version
        SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(primaryDisplayId, &handle) == NVAPI_OK);

        NV_GPU_INFO gpuInfo;
        gpuInfo.version = NV_GPU_INFO_VER;
        REQUIRE(NvAPI_GPU_GetGPUInfo(handle, &gpuInfo) != NVAPI_INCOMPATIBLE_STRUCT_VERSION);
    }

    SECTION("GetPstates20 returns no-implementation") {
        SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(primaryDisplayId, &handle) == NVAPI_OK);

        NV_GPU_PERF_PSTATES20_INFO params;
        params.version = NV_GPU_PERF_PSTATES20_INFO_VER;
        REQUIRE(NvAPI_GPU_GetPstates20(handle, &params) == NVAPI_NO_IMPLEMENTATION);
    }
}

TEST_CASE("GetDisplayViewportsByResolution returns mosaic-not-active", "[.sysinfo]") {
    NvU8 corrected;
    NV_RECT rect[NV_MOSAIC_MAX_DISPLAYS]{};
    REQUIRE(NvAPI_Mosaic_GetDisplayViewportsByResolution(0, 0, 0, rect, &corrected) == NVAPI_MOSAIC_NOT_ACTIVE);
}
