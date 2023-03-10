#include "nvapi_tests_private.h"
#include "resource_factory_util.h"
#include "nvapi_sysinfo_mocks.h"

using namespace trompeloeil;
using namespace Catch::Matchers;

TEST_CASE("GetInterfaceVersionString returns OK", "[.sysinfo]") {
    NvAPI_ShortString desc;
    REQUIRE(NvAPI_GetInterfaceVersionString(desc) == NVAPI_OK);
    REQUIRE_THAT(desc, Equals("DXVK_NVAPI"));
}

TEST_CASE("GetErrorMessage returns OK", "[.sysinfo]") {
    NvAPI_ShortString desc;
    REQUIRE(NvAPI_GetErrorMessage(NVAPI_NVIDIA_DEVICE_NOT_FOUND, desc) == NVAPI_OK);
    REQUIRE_THAT(desc, Equals("NVAPI_NVIDIA_DEVICE_NOT_FOUND"));
}

TEST_CASE("Initialize succeeds", "[.sysinfo]") {
    auto dxgiFactory = std::make_unique<DXGIFactory1Mock>();
    auto vulkan = std::make_unique<VulkanMock>();
    auto nvml = std::make_unique<NvmlMock>();
    auto lfx = std::make_unique<LfxMock>();
    DXGIDxvkAdapterMock adapter;
    DXGIOutput6Mock output;

    auto e = ConfigureDefaultTestEnvironment(*dxgiFactory, *vulkan, *nvml, *lfx, adapter, output);

    SECTION("Initialize returns OK") {
        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml), std::move(lfx));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);
        REQUIRE(NvAPI_Unload() == NVAPI_OK);
    }

    SECTION("Initialize returns device-not-found when DXVK reports no adapters") {
        ALLOW_CALL(*dxgiFactory, EnumAdapters1(_, _)) // NOLINT(bugprone-use-after-move)
            .RETURN(DXGI_ERROR_NOT_FOUND);

        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml), std::move(lfx));
        REQUIRE(NvAPI_Initialize() == NVAPI_NVIDIA_DEVICE_NOT_FOUND);
        REQUIRE(NvAPI_Unload() == NVAPI_API_NOT_INITIALIZED);
    }

    SECTION("Initialize returns device-not-found when DXVK NVAPI hack is enabled") {
        ALLOW_CALL(adapter, GetDesc1(_))
            .SIDE_EFFECT(_1->VendorId = 0x1002)
            .RETURN(S_OK);

        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml), std::move(lfx));
        REQUIRE(NvAPI_Initialize() == NVAPI_NVIDIA_DEVICE_NOT_FOUND);
        REQUIRE(NvAPI_Unload() == NVAPI_API_NOT_INITIALIZED);
    }

    SECTION("Initialize returns device-not-found when adapter with non NVIDIA driver ID has been found") {
        ALLOW_CALL(*vulkan, GetPhysicalDeviceProperties2(_, _, _)) // NOLINT(bugprone-use-after-move)
            .SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [](auto props, auto idProps, auto pciBusInfoProps, auto driverProps, auto fragmentShadingRateProps) {
                        driverProps->driverID = VK_DRIVER_ID_MESA_RADV;
                    }));

        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml), std::move(lfx));
        REQUIRE(NvAPI_Initialize() == NVAPI_NVIDIA_DEVICE_NOT_FOUND);
        REQUIRE(NvAPI_Unload() == NVAPI_API_NOT_INITIALIZED);
    }
}

TEST_CASE("Sysinfo methods succeed", "[.sysinfo]") {
    auto dxgiFactory = std::make_unique<DXGIFactory1Mock>();
    auto vulkan = std::make_unique<VulkanMock>();
    auto nvml = std::make_unique<NvmlMock>();
    auto lfx = std::make_unique<LfxMock>();
    DXGIDxvkAdapterMock adapter;
    DXGIOutput6Mock output;

    auto e = ConfigureDefaultTestEnvironment(*dxgiFactory, *vulkan, *nvml, *lfx, adapter, output);
    auto primaryDisplayId = 0x00010001;

    ::SetEnvironmentVariableA("DXVK_NVAPI_DRIVER_VERSION", "");

    SECTION("Initialize and unloads return OK") {
        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml), std::move(lfx));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);
        REQUIRE(NvAPI_Unload() == NVAPI_OK);
    }

    SECTION("GetDriverAndBranchVersion returns OK") {
        ALLOW_CALL(*vulkan, GetPhysicalDeviceProperties2(_, _, _)) // NOLINT(bugprone-use-after-move)
            .SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [](auto props, auto idProps, auto pciBusInfoProps, auto driverProps, auto fragmentShadingRateProps) {
                        driverProps->driverID = VK_DRIVER_ID_NVIDIA_PROPRIETARY;
                        props->driverVersion = (470 << 22) | (35 << 14) | 1 << 6;
                    }));

        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml), std::move(lfx));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvU32 version;
        NvAPI_ShortString branch;
        REQUIRE(NvAPI_SYS_GetDriverAndBranchVersion(&version, branch) == NVAPI_OK);
        REQUIRE(version == 47035);
        REQUIRE_THAT(branch, StartsWith("r"));
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
            Data{VK_DRIVER_ID_AMD_OPEN_SOURCE, 21, 2, 3, 2102});

        ::SetEnvironmentVariableA("DXVK_NVAPI_ALLOW_OTHER_DRIVERS", "1");

        ALLOW_CALL(*vulkan, GetPhysicalDeviceProperties2(_, _, _)) // NOLINT(bugprone-use-after-move)
            .SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [&args](auto props, auto idProps, auto pciBusInfoProps, auto driverProps, auto fragmentShadingRateProps) {
                        driverProps->driverID = args.driverId;
                        strcpy(props->deviceName, "GPU0");
                        if (args.driverId == VK_DRIVER_ID_NVIDIA_PROPRIETARY)
                            props->driverVersion = (args.major << 22) | (args.minor << 14) | (args.patch << 6);
                        else
                            props->driverVersion = (args.major << 22) | (args.minor << 12) | args.patch;
                    }));

        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml), std::move(lfx));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvDisplayHandle handle;
        REQUIRE(NvAPI_EnumNvidiaDisplayHandle(0, &handle) == NVAPI_OK);

        NV_DISPLAY_DRIVER_VERSION version;
        version.version = NV_DISPLAY_DRIVER_VERSION_VER;
        REQUIRE(NvAPI_GetDisplayDriverVersion(handle, &version) == NVAPI_OK);
        REQUIRE(version.drvVersion == args.expectedVersion);
        REQUIRE_THAT(version.szAdapterString, Equals("GPU0"));
        REQUIRE_THAT(version.szBuildBranchString, StartsWith("r"));

        ::SetEnvironmentVariableA("DXVK_NVAPI_ALLOW_OTHER_DRIVERS", "");
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

        ALLOW_CALL(*vulkan, GetPhysicalDeviceProperties2(_, _, _)) // NOLINT(bugprone-use-after-move)
            .SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [](auto props, auto idProps, auto pciBusInfoProps, auto driverProps, auto fragmentShadingRateProps) {
                        driverProps->driverID = VK_DRIVER_ID_NVIDIA_PROPRIETARY;
                        props->driverVersion = (470 << 22) | (45 << 14) | (0 << 6);
                    }));

        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml), std::move(lfx));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvDisplayHandle handle;
        REQUIRE(NvAPI_EnumNvidiaDisplayHandle(0, &handle) == NVAPI_OK);

        NV_DISPLAY_DRIVER_VERSION version;
        version.version = NV_DISPLAY_DRIVER_VERSION_VER;
        REQUIRE(NvAPI_GetDisplayDriverVersion(handle, &version) == NVAPI_OK);
        REQUIRE(version.drvVersion == args.expectedVersion);

        ::SetEnvironmentVariableA("DXVK_NVAPI_DRIVER_VERSION", "");
    }

    SECTION("GetGPUIDFromPhysicalGPU / GetPhysicalGPUFromGPUID succeeds") {
        ALLOW_CALL(*vulkan, GetDeviceExtensions(_, _)) // NOLINT(bugprone-use-after-move)
            .RETURN(std::set<std::string>{VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME, VK_EXT_PCI_BUS_INFO_EXTENSION_NAME});
        ALLOW_CALL(*vulkan, GetPhysicalDeviceProperties2(_, _, _))
            .LR_SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [](auto props, auto idProps, auto pciBusInfoProps, auto driverProps, auto fragmentShadingRateProps) {
                        driverProps->driverID = VK_DRIVER_ID_NVIDIA_PROPRIETARY;
                        pciBusInfoProps->pciDomain = 0x01;
                        pciBusInfoProps->pciBus = 0x02;
                        pciBusInfoProps->pciDevice = 0x03;
                    }));

        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml), std::move(lfx));
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

        ALLOW_CALL(*vulkan, GetDeviceExtensions(_, _)) // NOLINT(bugprone-use-after-move)
            .RETURN(std::set<std::string>{
                VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME,
                args.extensionName});
        ALLOW_CALL(*vulkan, GetPhysicalDeviceProperties2(_, _, _))
            .SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [args](auto props, auto idProps, auto pciBusInfoProps, auto driverProps, auto fragmentShadingRateProps) {
                        driverProps->driverID = args.driverId;
                        if (args.extensionName == VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME)
                            fragmentShadingRateProps->primitiveFragmentShadingRateWithMultipleViewports = VK_TRUE;
                    }));

        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml), std::move(lfx));
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

        SECTION("CudaEnumComputeCapableGpus (V2) returns OK") {
            NV_COMPUTE_GPU_TOPOLOGY_V2 gpuTopology;
            gpuTopology.version = NV_COMPUTE_GPU_TOPOLOGY_VER;
            gpuTopology.computeGpus = new NV_COMPUTE_GPU[1];
            REQUIRE(NvAPI_GPU_CudaEnumComputeCapableGpus(&gpuTopology) == NVAPI_OK);
            REQUIRE(gpuTopology.gpuCount == args.cudaGpuCount);
            if (gpuTopology.gpuCount == 1) {
                REQUIRE(gpuTopology.computeGpus[0].hPhysicalGpu == handle);
                REQUIRE(gpuTopology.computeGpus[0].flags == 0x0b);
            }
            delete gpuTopology.computeGpus;
        }

        SECTION("CudaEnumComputeCapableGpus (V2) without set compute-gpus returns invalid-argument") {
            NV_COMPUTE_GPU_TOPOLOGY_V2 gpuTopology{};
            gpuTopology.version = NV_COMPUTE_GPU_TOPOLOGY_VER;
            REQUIRE(NvAPI_GPU_CudaEnumComputeCapableGpus(&gpuTopology) == NVAPI_INVALID_ARGUMENT);
        }

        ::SetEnvironmentVariableA("DXVK_NVAPI_ALLOW_OTHER_DRIVERS", "");
    }

    SECTION("GetGPUType returns OK") {
        ALLOW_CALL(*vulkan, GetPhysicalDeviceProperties2(_, _, _)) // NOLINT(bugprone-use-after-move)
            .SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [](auto props, auto idProps, auto pciBusInfoProps, auto driverProps, auto fragmentShadingRateProps) {
                        driverProps->driverID = VK_DRIVER_ID_NVIDIA_PROPRIETARY;
                        props->deviceType = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
                    }));

        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml), std::move(lfx));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(primaryDisplayId, &handle) == NVAPI_OK);

        NV_GPU_TYPE type;
        REQUIRE(NvAPI_GPU_GetGPUType(handle, &type) == NVAPI_OK);
        REQUIRE(type == NV_SYSTEM_TYPE_DGPU);
    }

    SECTION("GetPCIIdentifiers returns OK") {
        ALLOW_CALL(adapter, GetDesc1(_))
            .SIDE_EFFECT({
                _1->VendorId = 0x10de;
                _1->DeviceId = 0x1234;
            })
            .RETURN(S_OK);

        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml), std::move(lfx));
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
        ALLOW_CALL(*vulkan, GetPhysicalDeviceProperties2(_, _, _)) // NOLINT(bugprone-use-after-move)
            .LR_SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [&name](auto props, auto idProps, auto pciBusInfoProps, auto driverProps, auto fragmentShadingRateProps) {
                        driverProps->driverID = VK_DRIVER_ID_NVIDIA_PROPRIETARY;
                        strcpy(props->deviceName, name);
                    }));

        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml), std::move(lfx));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(primaryDisplayId, &handle) == NVAPI_OK);

        NvAPI_ShortString fullName;
        REQUIRE(NvAPI_GPU_GetFullName(handle, fullName) == NVAPI_OK);
        REQUIRE_THAT(fullName, Equals(name));
    }

    SECTION("GetBusId returns OK") {
        auto id = 2U;
        ALLOW_CALL(*vulkan, GetDeviceExtensions(_, _)) // NOLINT(bugprone-use-after-move)
            .RETURN(std::set<std::string>{VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME, VK_EXT_PCI_BUS_INFO_EXTENSION_NAME});
        ALLOW_CALL(*vulkan, GetPhysicalDeviceProperties2(_, _, _))
            .LR_SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [&id](auto props, auto idProps, auto pciBusInfoProps, auto driverProps, auto fragmentShadingRateProps) {
                        driverProps->driverID = VK_DRIVER_ID_NVIDIA_PROPRIETARY;
                        pciBusInfoProps->pciBus = id;
                    }));

        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml), std::move(lfx));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(primaryDisplayId, &handle) == NVAPI_OK);

        NvU32 busId;
        REQUIRE(NvAPI_GPU_GetBusId(handle, &busId) == NVAPI_OK);
        REQUIRE(busId == id);
    }

    SECTION("GetBusSlotId returns OK") {
        auto id = 3U;
        ALLOW_CALL(*vulkan, GetDeviceExtensions(_, _)) // NOLINT(bugprone-use-after-move)
            .RETURN(std::set<std::string>{VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME, VK_EXT_PCI_BUS_INFO_EXTENSION_NAME});
        ALLOW_CALL(*vulkan, GetPhysicalDeviceProperties2(_, _, _))
            .LR_SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [&id](auto props, auto idProps, auto pciBusInfoProps, auto driverProps, auto fragmentShadingRateProps) {
                        driverProps->driverID = VK_DRIVER_ID_NVIDIA_PROPRIETARY;
                        pciBusInfoProps->pciDevice = id;
                    }));

        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml), std::move(lfx));
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

        ALLOW_CALL(*vulkan, GetDeviceExtensions(_, _)) // NOLINT(bugprone-use-after-move)
            .RETURN(std::set<std::string>{
                VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME,
                args.extensionName});
        ALLOW_CALL(*vulkan, GetPhysicalDeviceProperties2(_, _, _))
            .SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [](auto props, auto idProps, auto pciBusInfoProps, auto driverProps, auto fragmentShadingRateProps) {
                        driverProps->driverID = VK_DRIVER_ID_NVIDIA_PROPRIETARY;
                    }));

        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml), std::move(lfx));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(primaryDisplayId, &handle) == NVAPI_OK);

        NV_GPU_BUS_TYPE type;
        REQUIRE(NvAPI_GPU_GetBusType(handle, &type) == NVAPI_OK);
        REQUIRE(type == args.expectedBusType);
    }

    SECTION("GetPhysicalFrameBufferSize returns OK") {
        ALLOW_CALL(adapter, GetDesc1(_))
            .SIDE_EFFECT({
                _1->VendorId = 0x10de;
                _1->DedicatedVideoMemory = 8191 * 1024;
            })
            .RETURN(S_OK);

        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml), std::move(lfx));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(primaryDisplayId, &handle) == NVAPI_OK);

        NvU32 size;
        REQUIRE(NvAPI_GPU_GetPhysicalFrameBufferSize(handle, &size) == NVAPI_OK);
        REQUIRE(size == 8191);
    }

    SECTION("GetAdapterIdFromPhysicalGpu returns OK") {
        ALLOW_CALL(*vulkan, GetPhysicalDeviceProperties2(_, _, _)) // NOLINT(bugprone-use-after-move)
            .SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [](auto props, auto idProps, auto pciBusInfoProps, auto driverProps, auto fragmentShadingRateProps) {
                        auto luid = LUID{0x04030211, 0x08070655};
                        memcpy(&idProps->deviceLUID, &luid, sizeof(luid));
                        idProps->deviceLUIDValid = VK_TRUE;
                        driverProps->driverID = VK_DRIVER_ID_NVIDIA_PROPRIETARY;
                    }));

        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml), std::move(lfx));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(primaryDisplayId, &handle) == NVAPI_OK);

        LUID luid;
        REQUIRE(NvAPI_GPU_GetAdapterIdFromPhysicalGpu(handle, static_cast<void*>(&luid)) == NVAPI_OK);
        REQUIRE(luid.HighPart == 0x08070655);
        REQUIRE(luid.LowPart == 0x04030211);
    }

    SECTION("GetLogicalGpuInfo returns OK") {
        ALLOW_CALL(*vulkan, GetPhysicalDeviceProperties2(_, _, _)) // NOLINT(bugprone-use-after-move)
            .SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [](auto props, auto idProps, auto pciBusInfoProps, auto driverProps, auto fragmentShadingRateProps) {
                        auto luid = LUID{0x04030211, 0x08070655};
                        memcpy(&idProps->deviceLUID, &luid, sizeof(luid));
                        idProps->deviceLUIDValid = VK_TRUE;
                        driverProps->driverID = VK_DRIVER_ID_NVIDIA_PROPRIETARY;
                    }));

        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml), std::move(lfx));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvU32 count;
        NvLogicalGpuHandle handles[NVAPI_MAX_LOGICAL_GPUS]{};
        REQUIRE(NvAPI_EnumLogicalGPUs(handles, &count) == NVAPI_OK);
        REQUIRE(count == 1);

        NvPhysicalGpuHandle physicalHandles[NVAPI_MAX_PHYSICAL_GPUS]{};
        REQUIRE(NvAPI_GetPhysicalGPUsFromLogicalGPU(handles[0], physicalHandles, &count) == NVAPI_OK);
        REQUIRE(count == 1);

        SECTION("GetLogicalGpuInfo succeeds") {
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
            uint32_t deviceId;
            std::string extensionName;
            NV_GPU_ARCHITECTURE_ID expectedArchId;
            NV_GPU_ARCH_IMPLEMENTATION_ID expectedImplId;
        };
        auto args = GENERATE(
            Data{0x2600, VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME, NV_GPU_ARCHITECTURE_AD100, NV_GPU_ARCH_IMPLEMENTATION_AD102},
            Data{0x2000, VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME, NV_GPU_ARCHITECTURE_GA100, NV_GPU_ARCH_IMPLEMENTATION_GA102},
            Data{0x2000, VK_NV_SHADING_RATE_IMAGE_EXTENSION_NAME, NV_GPU_ARCHITECTURE_TU100, NV_GPU_ARCH_IMPLEMENTATION_TU102},
            Data{0x2000, VK_NVX_IMAGE_VIEW_HANDLE_EXTENSION_NAME, NV_GPU_ARCHITECTURE_GV100, NV_GPU_ARCH_IMPLEMENTATION_GV100},
            Data{0x2000, VK_NV_CLIP_SPACE_W_SCALING_EXTENSION_NAME, NV_GPU_ARCHITECTURE_GP100, NV_GPU_ARCH_IMPLEMENTATION_GP102},
            Data{0x2000, VK_NV_VIEWPORT_ARRAY2_EXTENSION_NAME, NV_GPU_ARCHITECTURE_GM200, NV_GPU_ARCH_IMPLEMENTATION_GM204},
            Data{0x2000, "ext", NV_GPU_ARCHITECTURE_GK100, NV_GPU_ARCH_IMPLEMENTATION_GK104});

        ALLOW_CALL(*vulkan, GetDeviceExtensions(_, _)) // NOLINT(bugprone-use-after-move)
            .RETURN(std::set<std::string>{
                VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME,
                args.extensionName});
        ALLOW_CALL(*vulkan, GetPhysicalDeviceProperties2(_, _, _))
            .SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [&args](auto props, auto idProps, auto pciBusInfoProps, auto driverProps, auto fragmentShadingRateProps) {
                        props->deviceID = args.deviceId;
                        driverProps->driverID = VK_DRIVER_ID_NVIDIA_PROPRIETARY;
                        if (args.extensionName == VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME)
                            fragmentShadingRateProps->primitiveFragmentShadingRateWithMultipleViewports = VK_TRUE;
                    }));

        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml), std::move(lfx));
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
        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(primaryDisplayId, &handle) == NVAPI_OK);

        NV_GPU_ARCH_INFO archInfo;
        archInfo.version = NV_GPU_ARCH_INFO_VER_2 + 1;
        REQUIRE(NvAPI_GPU_GetArchInfo(handle, &archInfo) == NVAPI_INCOMPATIBLE_STRUCT_VERSION);
    }

    SECTION("GetArchInfo with current struct version returns not incompatible-struct-version") {
        // This test should fail when a header update provides a newer not yet implemented struct version
        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(primaryDisplayId, &handle) == NVAPI_OK);

        NV_GPU_ARCH_INFO archInfo;
        archInfo.version = NV_GPU_ARCH_INFO_VER;
        REQUIRE(NvAPI_GPU_GetArchInfo(handle, &archInfo) != NVAPI_INCOMPATIBLE_STRUCT_VERSION);
    }

    SECTION("GetArchInfo returns device-not-found when no NVIDIA device is present") {
        ::SetEnvironmentVariableA("DXVK_NVAPI_ALLOW_OTHER_DRIVERS", "1");

        ALLOW_CALL(*vulkan, GetPhysicalDeviceProperties2(_, _, _)) // NOLINT(bugprone-use-after-move)
            .SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [](auto props, auto idProps, auto pciBusInfoProps, auto driverProps, auto fragmentShadingRateProps) {
                        driverProps->driverID = VK_DRIVER_ID_MESA_RADV;
                    }));

        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml), std::move(lfx));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(primaryDisplayId, &handle) == NVAPI_OK);

        NV_GPU_ARCH_INFO archInfo;
        archInfo.version = NV_GPU_ARCH_INFO_VER;
        REQUIRE(NvAPI_GPU_GetArchInfo(handle, &archInfo) == NVAPI_NVIDIA_DEVICE_NOT_FOUND);

        ::SetEnvironmentVariableA("DXVK_NVAPI_ALLOW_OTHER_DRIVERS", "");
    }

    SECTION("GetPstates20 returns no-implementation") {
        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(primaryDisplayId, &handle) == NVAPI_OK);

        NV_GPU_PERF_PSTATES20_INFO params;
        params.version = NV_GPU_PERF_PSTATES_INFO_VER;
        REQUIRE(NvAPI_GPU_GetPstates20(handle, &params) == NVAPI_NO_IMPLEMENTATION);
    }
}

TEST_CASE("GetDisplayViewportsByResolution returns mosaic-not-active", "[.sysinfo]") {
    NvU8 corrected;
    NV_RECT rect[NV_MOSAIC_MAX_DISPLAYS]{};
    REQUIRE(NvAPI_Mosaic_GetDisplayViewportsByResolution(0, 0, 0, rect, &corrected) == NVAPI_MOSAIC_NOT_ACTIVE);
}
