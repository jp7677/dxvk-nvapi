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
    DXGIOutputMock output;

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

TEST_CASE("Topology methods succeed", "[.sysinfo]") {
    auto dxgiFactory = std::make_unique<DXGIFactory1Mock>();
    auto vulkan = std::make_unique<VulkanMock>();
    auto nvml = std::make_unique<NvmlMock>();
    auto lfx = std::make_unique<LfxMock>();
    DXGIDxvkAdapterMock adapter1;
    DXGIDxvkAdapterMock adapter2;
    DXGIOutputMock output1;
    DXGIOutputMock output2;
    DXGIOutputMock output3;

    auto e = ConfigureExtendedTestEnvironment(*dxgiFactory, *vulkan, *nvml, *lfx, adapter1, adapter2, output1, output2, output3);

    SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml), std::move(lfx));
    REQUIRE(NvAPI_Initialize() == NVAPI_OK);

    SECTION("EnumLogicalGPUs succeeds") {
        NvLogicalGpuHandle handles[NVAPI_MAX_LOGICAL_GPUS]{};
        NvU32 count = 0U;

        REQUIRE(NvAPI_EnumLogicalGPUs(handles, &count) == NVAPI_OK);
        REQUIRE(handles[0] != handles[1]);
        REQUIRE(handles[0] != nullptr);
        REQUIRE(handles[1] != nullptr);
        REQUIRE(count == 2);

        NvAPI_ShortString name1;
        REQUIRE(NvAPI_GPU_GetFullName(reinterpret_cast<NvPhysicalGpuHandle>(handles[0]), name1) == NVAPI_OK);
        REQUIRE_THAT(name1, Equals("Device1"));

        NvAPI_ShortString name2;
        REQUIRE(NvAPI_GPU_GetFullName(reinterpret_cast<NvPhysicalGpuHandle>(handles[1]), name2) == NVAPI_OK);
        REQUIRE_THAT(name2, Equals("Device2"));
    }

    SECTION("EnumPhysicalGPUs succeeds") {
        NvPhysicalGpuHandle handles[NVAPI_MAX_PHYSICAL_GPUS]{};
        NvU32 count = 0U;

        REQUIRE(NvAPI_EnumPhysicalGPUs(handles, &count) == NVAPI_OK);
        REQUIRE(handles[0] != handles[1]);
        REQUIRE(handles[0] != nullptr);
        REQUIRE(handles[1] != nullptr);
        REQUIRE(count == 2);

        NvAPI_ShortString name1;
        REQUIRE(NvAPI_GPU_GetFullName(handles[0], name1) == NVAPI_OK);
        REQUIRE_THAT(name1, Equals("Device1"));

        NvAPI_ShortString name2;
        REQUIRE(NvAPI_GPU_GetFullName(handles[1], name2) == NVAPI_OK);
        REQUIRE_THAT(name2, Equals("Device2"));
    }

    SECTION("EnumEnumTCCPhysicalGPUs succeeds") {
        NvPhysicalGpuHandle handles[NVAPI_MAX_PHYSICAL_GPUS];
        NvU32 count = -1U;
        REQUIRE(NvAPI_EnumTCCPhysicalGPUs(handles, &count) == NVAPI_OK);
        REQUIRE(count == 0);
    }

    SECTION("EnumNvidiaDisplayHandle succeeds") {
        NvDisplayHandle handle1 = nullptr;
        REQUIRE(NvAPI_EnumNvidiaDisplayHandle(0U, &handle1) == NVAPI_OK);
        REQUIRE(handle1 != nullptr);

        NvDisplayHandle handle2 = nullptr;
        REQUIRE(NvAPI_EnumNvidiaDisplayHandle(1U, &handle2) == NVAPI_OK);
        REQUIRE(handle2 != nullptr);
        REQUIRE(handle2 != handle1);

        NvDisplayHandle handle3 = nullptr;
        REQUIRE(NvAPI_EnumNvidiaDisplayHandle(2U, &handle3) == NVAPI_OK);
        REQUIRE(handle3 != nullptr);
        REQUIRE(handle3 != handle1);
        REQUIRE(handle3 != handle2);

        NvDisplayHandle handle4 = nullptr;
        REQUIRE(NvAPI_EnumNvidiaDisplayHandle(3U, &handle4) == NVAPI_END_ENUMERATION);
        REQUIRE(handle4 == nullptr);
    }

    SECTION("EnumNvidiaUnAttachedDisplayHandle succeeds") {
        NvUnAttachedDisplayHandle handle = nullptr;
        REQUIRE(NvAPI_EnumNvidiaUnAttachedDisplayHandle(0U, &handle) == NVAPI_END_ENUMERATION);
        REQUIRE(handle == nullptr);
    }

    SECTION("GetPhysicalGPUsFromLogicalGPU succeeds") {
        NvLogicalGpuHandle handles[NVAPI_MAX_LOGICAL_GPUS];
        NvU32 count;
        REQUIRE(NvAPI_EnumLogicalGPUs(handles, &count) == NVAPI_OK);

        NvPhysicalGpuHandle physicalHandles[NVAPI_MAX_PHYSICAL_GPUS];
        REQUIRE(NvAPI_GetPhysicalGPUsFromLogicalGPU(handles[0], physicalHandles, &count) == NVAPI_OK);
        REQUIRE(count == 1);
        REQUIRE(physicalHandles[0] == reinterpret_cast<NvPhysicalGpuHandle>(handles[0]));

        REQUIRE(NvAPI_GetPhysicalGPUsFromLogicalGPU(handles[1], physicalHandles, &count) == NVAPI_OK);
        REQUIRE(count == 1);
        REQUIRE(physicalHandles[0] == reinterpret_cast<NvPhysicalGpuHandle>(handles[1]));
    }

    SECTION("GetPhysicalGPUsFromDisplay succeeds") {
        NvDisplayHandle displayHandle1 = nullptr;
        NvDisplayHandle displayHandle2 = nullptr;
        NvDisplayHandle displayHandle3 = nullptr;
        REQUIRE(NvAPI_EnumNvidiaDisplayHandle(0U, &displayHandle1) == NVAPI_OK);
        REQUIRE(NvAPI_EnumNvidiaDisplayHandle(1U, &displayHandle2) == NVAPI_OK);
        REQUIRE(NvAPI_EnumNvidiaDisplayHandle(2U, &displayHandle3) == NVAPI_OK);

        NvPhysicalGpuHandle handles1[NVAPI_MAX_PHYSICAL_GPUS]{};
        NvU32 count1 = 0U;

        REQUIRE(NvAPI_GetPhysicalGPUsFromDisplay(displayHandle1, handles1, &count1) == NVAPI_OK);
        REQUIRE(handles1[0] != nullptr);
        REQUIRE(count1 == 1);

        NvAPI_ShortString name1;
        REQUIRE(NvAPI_GPU_GetFullName(handles1[0], name1) == NVAPI_OK);
        REQUIRE_THAT(name1, Equals("Device1"));

        NvPhysicalGpuHandle handles2[NVAPI_MAX_PHYSICAL_GPUS]{};
        NvU32 count2 = 0;

        REQUIRE(NvAPI_GetPhysicalGPUsFromDisplay(displayHandle2, handles2, &count2) == NVAPI_OK);
        REQUIRE(handles2[0] != nullptr);
        REQUIRE(count2 == 1);
        REQUIRE(handles2[0] == handles1[0]);

        NvPhysicalGpuHandle handles3[NVAPI_MAX_PHYSICAL_GPUS]{};
        NvU32 count3 = 0U;

        REQUIRE(NvAPI_GetPhysicalGPUsFromDisplay(displayHandle3, handles3, &count3) == NVAPI_OK);
        REQUIRE(handles3[0] != nullptr);
        REQUIRE(count3 == 1);
        REQUIRE(handles3[0] != handles1[0]);

        NvAPI_ShortString name3;
        REQUIRE(NvAPI_GPU_GetFullName(handles3[0], name3) == NVAPI_OK);
        REQUIRE_THAT(name3, Equals("Device2"));
    }

    SECTION("GetPhysicalGpuFromDisplayId succeeds") {
        NvPhysicalGpuHandle handle1 = nullptr;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(0x00010001, &handle1) == NVAPI_OK);
        REQUIRE(handle1 != nullptr);

        NvAPI_ShortString name1;
        REQUIRE(NvAPI_GPU_GetFullName(handle1, name1) == NVAPI_OK);
        REQUIRE_THAT(name1, Equals("Device1"));

        NvPhysicalGpuHandle handle2 = nullptr;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(0x00010002, &handle2) == NVAPI_OK);
        REQUIRE(handle2 != nullptr);
        REQUIRE(handle2 == handle1);

        NvPhysicalGpuHandle handle3 = nullptr;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(0x00020001, &handle3) == NVAPI_OK);
        REQUIRE(handle3 != nullptr);

        NvAPI_ShortString name3;
        REQUIRE(NvAPI_GPU_GetFullName(handle3, name3) == NVAPI_OK);
        REQUIRE_THAT(name3, Equals("Device2"));

        NvPhysicalGpuHandle handle4 = nullptr;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(0x00000000, &handle4) == NVAPI_INVALID_ARGUMENT);
        REQUIRE(handle4 == nullptr);
    }

    SECTION("GetAssociatedNvidiaDisplayName succeeds") {
        NvDisplayHandle handle1 = nullptr;
        NvDisplayHandle handle2 = nullptr;
        NvDisplayHandle handle3 = nullptr;
        REQUIRE(NvAPI_EnumNvidiaDisplayHandle(0U, &handle1) == NVAPI_OK);
        REQUIRE(NvAPI_EnumNvidiaDisplayHandle(1U, &handle2) == NVAPI_OK);
        REQUIRE(NvAPI_EnumNvidiaDisplayHandle(2U, &handle3) == NVAPI_OK);

        NvAPI_ShortString name1;
        REQUIRE(NvAPI_GetAssociatedNvidiaDisplayName(handle1, name1) == NVAPI_OK);
        REQUIRE_THAT(name1, Equals("Output1"));

        NvAPI_ShortString name2;
        REQUIRE(NvAPI_GetAssociatedNvidiaDisplayName(handle2, name2) == NVAPI_OK);
        REQUIRE_THAT(name2, Equals("Output2"));

        NvAPI_ShortString name3;
        REQUIRE(NvAPI_GetAssociatedNvidiaDisplayName(handle3, name3) == NVAPI_OK);
        REQUIRE_THAT(name3, Equals("Output3"));

        NvAPI_ShortString name4;
        NvDisplayHandle handle4 = nullptr;
        REQUIRE(NvAPI_GetAssociatedNvidiaDisplayName(handle4, name4) == NVAPI_INVALID_ARGUMENT);
    }

    SECTION("GetGDIPrimaryDisplayId succeeds") {
        NvU32 displayId;
        REQUIRE(NvAPI_DISP_GetGDIPrimaryDisplayId(&displayId) == NVAPI_NVIDIA_DEVICE_NOT_FOUND); // MONITORINFO.dwFlags isn't mocked
    }
}

TEST_CASE("Sysinfo methods succeed", "[.sysinfo]") {
    auto dxgiFactory = std::make_unique<DXGIFactory1Mock>();
    auto vulkan = std::make_unique<VulkanMock>();
    auto nvml = std::make_unique<NvmlMock>();
    auto lfx = std::make_unique<LfxMock>();
    DXGIDxvkAdapterMock adapter;
    DXGIOutputMock output;

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

        ::SetEnvironmentVariableA("DXVK_NVAPI_ALLOW_OTHER_DRIVERS", "0");
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
            .RETURN(std::set<std::string>{VK_EXT_PCI_BUS_INFO_EXTENSION_NAME});
        ALLOW_CALL(*vulkan, GetPhysicalDeviceProperties2(_, _, _))
            .LR_SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [](auto props, auto idProps, auto pciBusInfoProps, auto driverProps, auto fragmentShadingRateProps) {
                        pciBusInfoProps->pciDomain = 0x01;
                        pciBusInfoProps->pciBus = 0x02;
                        pciBusInfoProps->pciDevice = 0x03;
                    }));

        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml), std::move(lfx));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(primaryDisplayId, &handle) == NVAPI_OK);

        SECTION("GetGPUIDFromPhysicalGPU succeeds") {
            NvU32 gpuId;
            REQUIRE(NvAPI_GetGPUIDfromPhysicalGPU(handle, &gpuId) == NVAPI_OK);
            REQUIRE(gpuId == 0x10203);
        }

        SECTION("GetPhysicalGPUFromGPUID succeeds") {
            NvPhysicalGpuHandle handleFromGpuId;
            REQUIRE(NvAPI_GetPhysicalGPUFromGPUID(0x10203, &handleFromGpuId) == NVAPI_OK);
            REQUIRE(handleFromGpuId == handle);
        }
    }

    SECTION("CudaEnumComputeCapableGpus returns OK") {
        struct Data {
            VkDriverId driverId;
            std::string extensionName;
            uint32_t gpuCount;
        };
        auto args = GENERATE(
            Data{VK_DRIVER_ID_NVIDIA_PROPRIETARY, VK_NV_VIEWPORT_ARRAY2_EXTENSION_NAME, 1},
            Data{VK_DRIVER_ID_MESA_RADV, VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME, 0},
            Data{VK_DRIVER_ID_NVIDIA_PROPRIETARY, "ext", 0});

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
            REQUIRE(gpuTopology.gpuCount == args.gpuCount);
            if (gpuTopology.gpuCount == 1) {
                REQUIRE(gpuTopology.computeGpus[0].hPhysicalGpu == handle);
                REQUIRE(gpuTopology.computeGpus[0].flags == 0x0b);
            }
        }

        SECTION("CudaEnumComputeCapableGpus (V2) returns OK") {
            NV_COMPUTE_GPU_TOPOLOGY_V2 gpuTopology;
            gpuTopology.version = NV_COMPUTE_GPU_TOPOLOGY_VER;
            REQUIRE(NvAPI_GPU_CudaEnumComputeCapableGpus(&gpuTopology) == NVAPI_OK);
            REQUIRE(gpuTopology.gpuCount == args.gpuCount);
            if (gpuTopology.gpuCount == 1) {
                REQUIRE(gpuTopology.computeGpus[0].hPhysicalGpu == handle);
                REQUIRE(gpuTopology.computeGpus[0].flags == 0x0b);
            }
            delete gpuTopology.computeGpus;
        }
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
        ALLOW_CALL(*vulkan, GetPhysicalDeviceProperties2(_, _, _)) // NOLINT(bugprone-use-after-move)
            .SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [](auto props, auto idProps, auto pciBusInfoProps, auto driverProps, auto fragmentShadingRateProps) {
                        driverProps->driverID = VK_DRIVER_ID_NVIDIA_PROPRIETARY;
                        props->vendorID = 0x10de;
                        props->deviceID = 0x1234;
                    }));

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
        ALLOW_CALL(*vulkan, GetPhysicalDeviceMemoryProperties2(_, _, _)) // NOLINT(bugprone-use-after-move)
            .SIDE_EFFECT({
                _3->memoryProperties.memoryHeapCount = 1;
                _3->memoryProperties.memoryHeaps[0].flags = VK_MEMORY_HEAP_DEVICE_LOCAL_BIT;
                _3->memoryProperties.memoryHeaps[0].size = 8191 * 1024;
            });

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
        NvLogicalGpuHandle handles[NVAPI_MAX_LOGICAL_GPUS];
        REQUIRE(NvAPI_EnumLogicalGPUs(handles, &count) == NVAPI_OK);
        REQUIRE(count == 1);

        NvPhysicalGpuHandle physicalHandles[NVAPI_MAX_PHYSICAL_GPUS];
        REQUIRE(NvAPI_GetPhysicalGPUsFromLogicalGPU(handles[0], physicalHandles, &count) == NVAPI_OK);
        REQUIRE(count == 1);

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

    SECTION("GetLogicalGpuInfo with unknown struct version returns incompatible-struct-version") {
        NvU32 count;
        NvLogicalGpuHandle handles[NVAPI_MAX_LOGICAL_GPUS];
        REQUIRE(NvAPI_EnumLogicalGPUs(handles, &count) == NVAPI_OK);
        REQUIRE(count == 1);

        LUID luid;
        NV_LOGICAL_GPU_DATA data;
        data.pOSAdapterId = static_cast<void*>(&luid);
        data.version = NV_LOGICAL_GPU_DATA_VER1 + 1;
        REQUIRE(NvAPI_GPU_GetLogicalGpuInfo(handles[0], &data) == NVAPI_INCOMPATIBLE_STRUCT_VERSION);
    }

    SECTION("GetLogicalGpuInfo with current struct version returns not incompatible-struct-version") {
        // This test should fail when a header update provides a newer not yet implemented struct version
        NvU32 count;
        NvLogicalGpuHandle handles[NVAPI_MAX_LOGICAL_GPUS];
        REQUIRE(NvAPI_EnumLogicalGPUs(handles, &count) == NVAPI_OK);
        REQUIRE(count == 1);

        LUID luid;
        NV_LOGICAL_GPU_DATA data;
        data.pOSAdapterId = static_cast<void*>(&luid);
        data.version = NV_LOGICAL_GPU_DATA_VER;
        REQUIRE(NvAPI_GPU_GetLogicalGpuInfo(handles[0], &data) != NVAPI_INCOMPATIBLE_STRUCT_VERSION);
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
            REQUIRE(archInfo.revision == NV_GPU_CHIP_REV_A01);
        }

        SECTION("GetArchInfo (V2) returns OK") {
            NV_GPU_ARCH_INFO_V2 archInfo;
            archInfo.version = NV_GPU_ARCH_INFO_VER_2;
            REQUIRE(NvAPI_GPU_GetArchInfo(handle, &archInfo) == NVAPI_OK);
            REQUIRE(archInfo.architecture_id == args.expectedArchId);
            REQUIRE(archInfo.implementation_id == args.expectedImplId);
            REQUIRE(archInfo.revision_id == NV_GPU_CHIP_REV_A01);
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

        ::SetEnvironmentVariableA("DXVK_NVAPI_ALLOW_OTHER_DRIVERS", "0");
    }

    SECTION("GetPstates20 returns no-implementation") {
        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(primaryDisplayId, &handle) == NVAPI_OK);

        NV_GPU_PERF_PSTATES20_INFO params;
        params.version = NV_GPU_PERF_PSTATES_INFO_VER;
        REQUIRE(NvAPI_GPU_GetPstates20(handle, &params) == NVAPI_NO_IMPLEMENTATION);
    }

    SECTION("NVML depending methods succeed when NVML is available") {
        ALLOW_CALL(*nvml, IsAvailable()) // NOLINT(bugprone-use-after-move)
            .RETURN(true);
        ALLOW_CALL(*nvml, DeviceGetHandleByPciBusId_v2(_, _))
            .SIDE_EFFECT(*_2 = reinterpret_cast<nvmlDevice_t>(0x1234)) // Just a non-nullptr
            .RETURN(NVML_SUCCESS);

        SECTION("GetCurrentPCIEDownstreamWidth returns OK") {
            auto linkWidth = 16U;
            ALLOW_CALL(*nvml, DeviceGetCurrPcieLinkWidth(_, _)) // NOLINT(bugprone-use-after-move)
                .LR_SIDE_EFFECT(*_2 = linkWidth)
                .RETURN(NVML_SUCCESS);

            SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml), std::move(lfx));
            REQUIRE(NvAPI_Initialize() == NVAPI_OK);

            NvPhysicalGpuHandle handle;
            REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(primaryDisplayId, &handle) == NVAPI_OK);

            NvU32 width;
            REQUIRE(NvAPI_GPU_GetCurrentPCIEDownstreamWidth(handle, &width) == NVAPI_OK);
            REQUIRE(width == linkWidth);
        }

        SECTION("GetIrq returns OK") {
            auto irqNum = 143U;
            ALLOW_CALL(*nvml, DeviceGetIrqNum(_, _)) // NOLINT(bugprone-use-after-move)
                .LR_SIDE_EFFECT(*_2 = irqNum)
                .RETURN(NVML_SUCCESS);

            SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml), std::move(lfx));
            REQUIRE(NvAPI_Initialize() == NVAPI_OK);

            NvPhysicalGpuHandle handle;
            REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(primaryDisplayId, &handle) == NVAPI_OK);

            NvU32 irq;
            REQUIRE(NvAPI_GPU_GetIRQ(handle, &irq) == NVAPI_OK);
            REQUIRE(irq == irqNum);
        }

        SECTION("GetGpuCoreCount returns OK") {
            auto cores = 1536U;
            ALLOW_CALL(*nvml, DeviceGetNumGpuCores(_, _)) // NOLINT(bugprone-use-after-move)
                .LR_SIDE_EFFECT(*_2 = cores)
                .RETURN(NVML_SUCCESS);

            SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml), std::move(lfx));
            REQUIRE(NvAPI_Initialize() == NVAPI_OK);

            NvPhysicalGpuHandle handle;
            REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(primaryDisplayId, &handle) == NVAPI_OK);

            NvU32 count;
            REQUIRE(NvAPI_GPU_GetGpuCoreCount(handle, &count) == NVAPI_OK);
            REQUIRE(count == cores);
        }

        SECTION("GetPCIIdentifiers returns OK and has subsystem ID when NVML is available") {
            auto id = 0x88161043;
            ALLOW_CALL(*nvml, DeviceGetPciInfo_v3(_, _)) // NOLINT(bugprone-use-after-move)
                .LR_SIDE_EFFECT(_2->pciSubSystemId = id)
                .RETURN(NVML_SUCCESS);

            SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml), std::move(lfx));
            REQUIRE(NvAPI_Initialize() == NVAPI_OK);

            NvPhysicalGpuHandle handle;
            REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(primaryDisplayId, &handle) == NVAPI_OK);

            NvU32 deviceId, subSystemId, revisionId, extDeviceId;
            REQUIRE(NvAPI_GPU_GetPCIIdentifiers(handle, &deviceId, &subSystemId, &revisionId, &extDeviceId) == NVAPI_OK);
            REQUIRE(subSystemId == id);
        }

        SECTION("GetVbiosVersionString returns OK") {
            auto version = "12.34";
            ALLOW_CALL(*nvml, DeviceGetVbiosVersion(_, _, _)) // NOLINT(bugprone-use-after-move)
                .LR_SIDE_EFFECT(strcpy(_2, version))
                .RETURN(NVML_SUCCESS);

            SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml), std::move(lfx));
            REQUIRE(NvAPI_Initialize() == NVAPI_OK);

            NvPhysicalGpuHandle handle;
            REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(primaryDisplayId, &handle) == NVAPI_OK);

            NvAPI_ShortString revision;
            REQUIRE(NvAPI_GPU_GetVbiosVersionString(handle, revision) == NVAPI_OK);
            REQUIRE_THAT(revision, Equals(version));
        }

        SECTION("GetBusType returns OK") {
            struct Data {
                nvmlBusType_t nvmlBusType;
                NV_GPU_BUS_TYPE expectedBusType;
            };
            auto args = GENERATE(
                Data{NVML_BUS_TYPE_PCI, NVAPI_GPU_BUS_TYPE_PCI},
                Data{NVML_BUS_TYPE_PCIE, NVAPI_GPU_BUS_TYPE_PCI_EXPRESS},
                Data{NVML_BUS_TYPE_FPCI, NVAPI_GPU_BUS_TYPE_FPCI},
                Data{NVML_BUS_TYPE_AGP, NVAPI_GPU_BUS_TYPE_AGP},
                Data{NVML_BUS_TYPE_UNKNOWN, NVAPI_GPU_BUS_TYPE_UNDEFINED});

            ALLOW_CALL(*nvml, DeviceGetBusType(_, _)) // NOLINT(bugprone-use-after-move)
                .LR_SIDE_EFFECT(*_2 = args.nvmlBusType)
                .RETURN(NVML_SUCCESS);

            SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml), std::move(lfx));
            REQUIRE(NvAPI_Initialize() == NVAPI_OK);

            NvPhysicalGpuHandle handle;
            REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(primaryDisplayId, &handle) == NVAPI_OK);

            NV_GPU_BUS_TYPE type;
            REQUIRE(NvAPI_GPU_GetBusType(handle, &type) == NVAPI_OK);
            REQUIRE(type == args.expectedBusType);
        }

        SECTION("GetDynamicPstatesInfoEx returns OK when DeviceGetDynamicPstatesInfo is available") {
            auto gpuUtilization = 32U;
            auto fbUtilization = 56U;
            auto vidUtilization = 8U;
            auto busUtilization = 80U;
            ALLOW_CALL(*nvml, DeviceGetDynamicPstatesInfo(_, _)) // NOLINT(bugprone-use-after-move)
                .LR_SIDE_EFFECT({
                    _2->flags = 0;
                    _2->utilization[NVML_GPU_UTILIZATION_DOMAIN_GPU].percentage = gpuUtilization;
                    _2->utilization[NVML_GPU_UTILIZATION_DOMAIN_FB].percentage = fbUtilization;
                    _2->utilization[NVML_GPU_UTILIZATION_DOMAIN_VID].percentage = vidUtilization;
                    _2->utilization[NVML_GPU_UTILIZATION_DOMAIN_BUS].percentage = busUtilization;
                    for (auto i = 0U; i < NVAPI_MAX_GPU_UTILIZATIONS; i++)
                        _2->utilization[i].bIsPresent = i < 4;
                })
                .RETURN(NVML_SUCCESS);
            ALLOW_CALL(*nvml, DeviceGetUtilizationRates(_, _)) // NOLINT(bugprone-use-after-move)
                .LR_SIDE_EFFECT({
                    _2->gpu = gpuUtilization + 1;
                    _2->memory = fbUtilization + 1;
                })
                .RETURN(NVML_SUCCESS);

            SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml), std::move(lfx));
            REQUIRE(NvAPI_Initialize() == NVAPI_OK);

            NvPhysicalGpuHandle handle;
            REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(primaryDisplayId, &handle) == NVAPI_OK);

            NV_GPU_DYNAMIC_PSTATES_INFO_EX info;
            info.version = NV_GPU_DYNAMIC_PSTATES_INFO_EX_VER;
            REQUIRE(NvAPI_GPU_GetDynamicPstatesInfoEx(handle, &info) == NVAPI_OK);
            REQUIRE(info.flags == 0);
            REQUIRE(info.utilization[0].bIsPresent == 1);
            REQUIRE(info.utilization[0].percentage == gpuUtilization);
            REQUIRE(info.utilization[1].bIsPresent == 1);
            REQUIRE(info.utilization[1].percentage == fbUtilization);
            REQUIRE(info.utilization[2].bIsPresent == 1);
            REQUIRE(info.utilization[2].percentage == vidUtilization);
            REQUIRE(info.utilization[3].bIsPresent == 1);
            REQUIRE(info.utilization[3].percentage == busUtilization);
            for (auto i = 4U; i < NVAPI_MAX_GPU_UTILIZATIONS; i++)
                REQUIRE(info.utilization[i].bIsPresent == 0);
        }

        SECTION("GetDynamicPstatesInfoEx returns OK when DeviceGetDynamicPstatesInfo is not available but DeviceGetUtilizationRates is") {
            auto gpuUtilization = 32U;
            auto memoryUtilization = 56U;
            ALLOW_CALL(*nvml, DeviceGetDynamicPstatesInfo(_, _)) // NOLINT(bugprone-use-after-move)
                .RETURN(NVML_ERROR_FUNCTION_NOT_FOUND);
            ALLOW_CALL(*nvml, DeviceGetUtilizationRates(_, _)) // NOLINT(bugprone-use-after-move)
                .LR_SIDE_EFFECT({
                    _2->gpu = gpuUtilization;
                    _2->memory = memoryUtilization;
                })
                .RETURN(NVML_SUCCESS);

            SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml), std::move(lfx));
            REQUIRE(NvAPI_Initialize() == NVAPI_OK);

            NvPhysicalGpuHandle handle;
            REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(primaryDisplayId, &handle) == NVAPI_OK);

            NV_GPU_DYNAMIC_PSTATES_INFO_EX info;
            info.version = NV_GPU_DYNAMIC_PSTATES_INFO_EX_VER;
            REQUIRE(NvAPI_GPU_GetDynamicPstatesInfoEx(handle, &info) == NVAPI_OK);
            REQUIRE(info.flags == 0);
            REQUIRE(info.utilization[0].bIsPresent == 1);
            REQUIRE(info.utilization[0].percentage == gpuUtilization);
            REQUIRE(info.utilization[1].bIsPresent == 1);
            REQUIRE(info.utilization[1].percentage == memoryUtilization);
            REQUIRE(info.utilization[2].bIsPresent == 1);
            REQUIRE(info.utilization[2].percentage == 0);
            REQUIRE(info.utilization[3].bIsPresent == 1);
            REQUIRE(info.utilization[3].percentage == 0);
            for (auto i = 4U; i < NVAPI_MAX_GPU_UTILIZATIONS; i++)
                REQUIRE(info.utilization[i].bIsPresent == 0);
        }

        SECTION("GetThermalSettings succeeds when DeviceGetThermalSettings is available") {
            auto temp = 65;
            auto maxTemp = 127;
            auto minTemp = -40;
            ALLOW_CALL(*nvml, DeviceGetThermalSettings(_, _, _)) // NOLINT(bugprone-use-after-move)
                .LR_SIDE_EFFECT({
                    _3->count = 1;
                    if (_2 == 0 || _2 == NVML_THERMAL_TARGET_ALL) {
                        _3->sensor[0].controller = NVML_THERMAL_CONTROLLER_GPU_INTERNAL;
                        _3->sensor[0].target = NVML_THERMAL_TARGET_GPU;
                        _3->sensor[0].currentTemp = temp;
                        _3->sensor[0].defaultMaxTemp = maxTemp;
                        _3->sensor[0].defaultMinTemp = minTemp;
                    }
                })
                .RETURN(NVML_SUCCESS);
            ALLOW_CALL(*nvml, DeviceGetTemperature(_, _, _)) // NOLINT(bugprone-use-after-move)
                .LR_SIDE_EFFECT(*_3 = temp + 1)
                .RETURN(NVML_SUCCESS);

            SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml), std::move(lfx));
            REQUIRE(NvAPI_Initialize() == NVAPI_OK);

            NvPhysicalGpuHandle handle;
            REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(primaryDisplayId, &handle) == NVAPI_OK);

            SECTION("GetThermalSettings (V1) returns OK") {
                NV_GPU_THERMAL_SETTINGS_V1 settings;
                settings.version = NV_GPU_THERMAL_SETTINGS_VER_1;
                REQUIRE(NvAPI_GPU_GetThermalSettings(handle, NVAPI_THERMAL_TARGET_ALL, reinterpret_cast<NV_GPU_THERMAL_SETTINGS*>(&settings)) == NVAPI_OK);
                REQUIRE(settings.count == 1);
                REQUIRE(settings.sensor[0].controller == NVAPI_THERMAL_CONTROLLER_GPU_INTERNAL);
                REQUIRE(settings.sensor[0].target == NVAPI_THERMAL_TARGET_GPU);
                REQUIRE(settings.sensor[0].currentTemp == static_cast<NvU32>(temp));
                REQUIRE(settings.sensor[0].defaultMaxTemp == static_cast<NvU32>(maxTemp));
                REQUIRE(settings.sensor[0].defaultMinTemp == 0U);
            }

            SECTION("GetThermalSettings (V2) returns OK") {
                NV_GPU_THERMAL_SETTINGS_V2 settings;
                settings.version = NV_GPU_THERMAL_SETTINGS_VER_2;
                REQUIRE(NvAPI_GPU_GetThermalSettings(handle, NVAPI_THERMAL_TARGET_ALL, &settings) == NVAPI_OK);
                REQUIRE(settings.count == 1);
                REQUIRE(settings.sensor[0].controller == NVAPI_THERMAL_CONTROLLER_GPU_INTERNAL);
                REQUIRE(settings.sensor[0].target == NVAPI_THERMAL_TARGET_GPU);
                REQUIRE(settings.sensor[0].currentTemp == temp);
                REQUIRE(settings.sensor[0].defaultMaxTemp == maxTemp);
                REQUIRE(settings.sensor[0].defaultMinTemp == minTemp);
            }

            SECTION("GetThermalSettings with unknown struct version returns incompatible-struct-version") {
                NV_GPU_THERMAL_SETTINGS settings;
                settings.version = NV_GPU_THERMAL_SETTINGS_VER_2 + 1;
                REQUIRE(NvAPI_GPU_GetThermalSettings(handle, NVAPI_THERMAL_TARGET_ALL, &settings) == NVAPI_INCOMPATIBLE_STRUCT_VERSION);
            }

            SECTION("GetThermalSettings with current struct version returns not incompatible-struct-version") {
                // This test should fail when a header update provides a newer not yet implemented struct version
                NV_GPU_THERMAL_SETTINGS settings;
                settings.version = NV_GPU_THERMAL_SETTINGS_VER;
                REQUIRE(NvAPI_GPU_GetThermalSettings(handle, NVAPI_THERMAL_TARGET_ALL, &settings) != NVAPI_INCOMPATIBLE_STRUCT_VERSION);
            }
        }

        SECTION("GetThermalSettings succeeds when DeviceGetThermalSettings is not available but DeviceGetTemperature is") {
            auto temp = 65U;
            ALLOW_CALL(*nvml, DeviceGetThermalSettings(_, _, _)) // NOLINT(bugprone-use-after-move)
                .RETURN(NVML_ERROR_FUNCTION_NOT_FOUND);
            ALLOW_CALL(*nvml, DeviceGetTemperature(_, _, _)) // NOLINT(bugprone-use-after-move)
                .LR_SIDE_EFFECT(*_3 = temp)
                .RETURN(NVML_SUCCESS);

            SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml), std::move(lfx));
            REQUIRE(NvAPI_Initialize() == NVAPI_OK);

            NvPhysicalGpuHandle handle;
            REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(primaryDisplayId, &handle) == NVAPI_OK);

            SECTION("GetThermalSettings (V1) returns OK") {
                NV_GPU_THERMAL_SETTINGS_V1 settings;
                settings.version = NV_GPU_THERMAL_SETTINGS_VER_1;
                REQUIRE(NvAPI_GPU_GetThermalSettings(handle, NVAPI_THERMAL_TARGET_ALL, reinterpret_cast<NV_GPU_THERMAL_SETTINGS*>(&settings)) == NVAPI_OK);
                REQUIRE(settings.count == 1);
                REQUIRE(settings.sensor[0].controller == NVAPI_THERMAL_CONTROLLER_UNKNOWN);
                REQUIRE(settings.sensor[0].target == NVAPI_THERMAL_TARGET_GPU);
                REQUIRE(settings.sensor[0].currentTemp == temp);
                REQUIRE(settings.sensor[0].defaultMaxTemp == 127U);
                REQUIRE(settings.sensor[0].defaultMinTemp == 0U);
            }

            SECTION("GetThermalSettings (V2) returns OK") {
                NV_GPU_THERMAL_SETTINGS_V2 settings;
                settings.version = NV_GPU_THERMAL_SETTINGS_VER_2;
                REQUIRE(NvAPI_GPU_GetThermalSettings(handle, NVAPI_THERMAL_TARGET_ALL, &settings) == NVAPI_OK);
                REQUIRE(settings.count == 1);
                REQUIRE(settings.sensor[0].controller == NVAPI_THERMAL_CONTROLLER_UNKNOWN);
                REQUIRE(settings.sensor[0].target == NVAPI_THERMAL_TARGET_GPU);
                REQUIRE(settings.sensor[0].currentTemp == static_cast<int>(temp));
                REQUIRE(settings.sensor[0].defaultMaxTemp == 127);
                REQUIRE(settings.sensor[0].defaultMinTemp == -256);
            }

            SECTION("GetThermalSettings with unknown struct version returns incompatible-struct-version") {
                NV_GPU_THERMAL_SETTINGS settings;
                settings.version = NV_GPU_THERMAL_SETTINGS_VER_2 + 1;
                REQUIRE(NvAPI_GPU_GetThermalSettings(handle, NVAPI_THERMAL_TARGET_ALL, &settings) == NVAPI_INCOMPATIBLE_STRUCT_VERSION);
            }

            SECTION("GetThermalSettings with current struct version returns not incompatible-struct-version") {
                // This test should fail when a header update provides a newer not yet implemented struct version
                NV_GPU_THERMAL_SETTINGS settings;
                settings.version = NV_GPU_THERMAL_SETTINGS_VER;
                REQUIRE(NvAPI_GPU_GetThermalSettings(handle, NVAPI_THERMAL_TARGET_ALL, &settings) != NVAPI_INCOMPATIBLE_STRUCT_VERSION);
            }
        }

        SECTION("GetCurrentPstate returns OK") {
            ALLOW_CALL(*nvml, DeviceGetPerformanceState(_, _)) // NOLINT(bugprone-use-after-move)
                .LR_SIDE_EFFECT(*_2 = NVML_PSTATE_2)
                .RETURN(NVML_SUCCESS);

            SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml), std::move(lfx));
            REQUIRE(NvAPI_Initialize() == NVAPI_OK);

            NvPhysicalGpuHandle handle;
            REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(primaryDisplayId, &handle) == NVAPI_OK);

            NV_GPU_PERF_PSTATE_ID pstate;
            REQUIRE(NvAPI_GPU_GetCurrentPstate(handle, &pstate) == NVAPI_OK);
            REQUIRE(pstate == NVAPI_GPU_PERF_PSTATE_P2);
        }

        SECTION("GetAllClockFrequencies returns OK") {
            auto graphicsClock = 500U;
            auto memoryClock = 600U;
            auto videoClock = 700U;
            ALLOW_CALL(*nvml, DeviceGetClockInfo(_, NVML_CLOCK_GRAPHICS, _)) // NOLINT(bugprone-use-after-move)
                .LR_SIDE_EFFECT(*_3 = graphicsClock)
                .RETURN(NVML_SUCCESS);
            ALLOW_CALL(*nvml, DeviceGetClockInfo(_, NVML_CLOCK_MEM, _))
                .LR_SIDE_EFFECT(*_3 = memoryClock)
                .RETURN(NVML_SUCCESS);
            ALLOW_CALL(*nvml, DeviceGetClockInfo(_, NVML_CLOCK_VIDEO, _))
                .LR_SIDE_EFFECT(*_3 = videoClock)
                .RETURN(NVML_SUCCESS);

            SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml), std::move(lfx));
            REQUIRE(NvAPI_Initialize() == NVAPI_OK);

            NvPhysicalGpuHandle handle;
            REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(primaryDisplayId, &handle) == NVAPI_OK);

            SECTION("GetAllClockFrequencies (V1) returns OK") {
                NV_GPU_CLOCK_FREQUENCIES_V1 frequencies;
                frequencies.version = NV_GPU_CLOCK_FREQUENCIES_VER_1;
                REQUIRE(NvAPI_GPU_GetAllClockFrequencies(handle, reinterpret_cast<NV_GPU_CLOCK_FREQUENCIES*>(&frequencies)) == NVAPI_OK);
                REQUIRE(frequencies.domain[NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS].bIsPresent == 1);
                REQUIRE(frequencies.domain[NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS].frequency == graphicsClock * 1000);
                REQUIRE(frequencies.domain[NVAPI_GPU_PUBLIC_CLOCK_MEMORY].bIsPresent == 1);
                REQUIRE(frequencies.domain[NVAPI_GPU_PUBLIC_CLOCK_MEMORY].frequency == memoryClock * 1000);
                REQUIRE(frequencies.domain[NVAPI_GPU_PUBLIC_CLOCK_VIDEO].bIsPresent == 1);
                REQUIRE(frequencies.domain[NVAPI_GPU_PUBLIC_CLOCK_VIDEO].frequency == videoClock * 1000);
            }

            SECTION("GetAllClockFrequencies (V2) returns OK") {
                NV_GPU_CLOCK_FREQUENCIES_V2 frequencies;
                frequencies.version = NV_GPU_CLOCK_FREQUENCIES_VER_2;
                frequencies.ClockType = NV_GPU_CLOCK_FREQUENCIES_CURRENT_FREQ;
                REQUIRE(NvAPI_GPU_GetAllClockFrequencies(handle, &frequencies) == NVAPI_OK);
                REQUIRE(frequencies.domain[NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS].bIsPresent == 1);
                REQUIRE(frequencies.domain[NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS].frequency == graphicsClock * 1000);
                REQUIRE(frequencies.domain[NVAPI_GPU_PUBLIC_CLOCK_MEMORY].bIsPresent == 1);
                REQUIRE(frequencies.domain[NVAPI_GPU_PUBLIC_CLOCK_MEMORY].frequency == memoryClock * 1000);
                REQUIRE(frequencies.domain[NVAPI_GPU_PUBLIC_CLOCK_VIDEO].bIsPresent == 1);
                REQUIRE(frequencies.domain[NVAPI_GPU_PUBLIC_CLOCK_VIDEO].frequency == videoClock * 1000);
            }

            SECTION("GetAllClockFrequencies (V3) returns OK") {
                NV_GPU_CLOCK_FREQUENCIES_V2 frequencies;
                frequencies.version = NV_GPU_CLOCK_FREQUENCIES_VER_3;
                frequencies.ClockType = NV_GPU_CLOCK_FREQUENCIES_CURRENT_FREQ;
                REQUIRE(NvAPI_GPU_GetAllClockFrequencies(handle, &frequencies) == NVAPI_OK);
                REQUIRE(frequencies.domain[NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS].bIsPresent == 1);
                REQUIRE(frequencies.domain[NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS].frequency == graphicsClock * 1000);
                REQUIRE(frequencies.domain[NVAPI_GPU_PUBLIC_CLOCK_MEMORY].bIsPresent == 1);
                REQUIRE(frequencies.domain[NVAPI_GPU_PUBLIC_CLOCK_MEMORY].frequency == memoryClock * 1000);
                REQUIRE(frequencies.domain[NVAPI_GPU_PUBLIC_CLOCK_VIDEO].bIsPresent == 1);
                REQUIRE(frequencies.domain[NVAPI_GPU_PUBLIC_CLOCK_VIDEO].frequency == videoClock * 1000);
            }
        }

        SECTION("GetAllClockFrequencies with unknown struct version returns incompatible-struct-version") {
            NvPhysicalGpuHandle handle;
            REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(primaryDisplayId, &handle) == NVAPI_OK);

            NV_GPU_CLOCK_FREQUENCIES frequencies;
            frequencies.version = NV_GPU_CLOCK_FREQUENCIES_VER_3 + 1;
            REQUIRE(NvAPI_GPU_GetAllClockFrequencies(handle, &frequencies) == NVAPI_INCOMPATIBLE_STRUCT_VERSION);
        }

        SECTION("GetAllClockFrequencies with current struct version returns not incompatible-struct-version") {
            // This test should fail when a header update provides a newer not yet implemented struct version
            NvPhysicalGpuHandle handle;
            REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(primaryDisplayId, &handle) == NVAPI_OK);

            NV_GPU_CLOCK_FREQUENCIES frequencies;
            frequencies.version = NV_GPU_CLOCK_FREQUENCIES_VER;
            REQUIRE(NvAPI_GPU_GetAllClockFrequencies(handle, &frequencies) != NVAPI_INCOMPATIBLE_STRUCT_VERSION);
        }

        SECTION("GetAllClockFrequencies returns not-supported for base/boost clock types") {
            struct Data {
                NV_GPU_CLOCK_FREQUENCIES_CLOCK_TYPE clockType;
            };
            auto args = GENERATE(
                Data{NV_GPU_CLOCK_FREQUENCIES_BASE_CLOCK},
                Data{NV_GPU_CLOCK_FREQUENCIES_BOOST_CLOCK});

            NvPhysicalGpuHandle handle;
            REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(primaryDisplayId, &handle) == NVAPI_OK);

            SECTION("GetAllClockFrequencies (V2) returns not-supported") {
                NV_GPU_CLOCK_FREQUENCIES_V2 frequencies;
                frequencies.version = NV_GPU_CLOCK_FREQUENCIES_VER_2;
                frequencies.ClockType = args.clockType;
                REQUIRE(NvAPI_GPU_GetAllClockFrequencies(handle, &frequencies) == NVAPI_NOT_SUPPORTED);
            }

            SECTION("GetAllClockFrequencies (V3) returns not-supported") {
                NV_GPU_CLOCK_FREQUENCIES_V2 frequencies;
                frequencies.version = NV_GPU_CLOCK_FREQUENCIES_VER_3;
                frequencies.ClockType = args.clockType;
                REQUIRE(NvAPI_GPU_GetAllClockFrequencies(handle, &frequencies) == NVAPI_NOT_SUPPORTED);
            }
        }
    }

    SECTION("NVML depending methods succeed when NVML is not available") {
        ALLOW_CALL(*nvml, IsAvailable()) // NOLINT(bugprone-use-after-move)
            .RETURN(false);

        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml), std::move(lfx));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(primaryDisplayId, &handle) == NVAPI_OK);

        SECTION("GetVbiosVersionString returns OK when NVML is not available") {
            NvAPI_ShortString revision;
            REQUIRE(NvAPI_GPU_GetVbiosVersionString(handle, revision) == NVAPI_OK);
            REQUIRE_THAT(revision, Equals("N/A"));
        }

        SECTION("NVML depending methods return no-implementation when NVML is not available") {
            NvU32 width;
            REQUIRE(NvAPI_GPU_GetCurrentPCIEDownstreamWidth(handle, &width) == NVAPI_NO_IMPLEMENTATION);
            NvU32 irq;
            REQUIRE(NvAPI_GPU_GetIRQ(handle, &irq) == NVAPI_NO_IMPLEMENTATION);
            NvU32 cores;
            REQUIRE(NvAPI_GPU_GetGpuCoreCount(handle, &cores) == NVAPI_NO_IMPLEMENTATION);
            NV_GPU_DYNAMIC_PSTATES_INFO_EX info;
            info.version = NV_GPU_DYNAMIC_PSTATES_INFO_EX_VER;
            REQUIRE(NvAPI_GPU_GetDynamicPstatesInfoEx(handle, &info) == NVAPI_NO_IMPLEMENTATION);
            NV_GPU_THERMAL_SETTINGS settings;
            settings.version = NV_GPU_THERMAL_SETTINGS_VER;
            REQUIRE(NvAPI_GPU_GetThermalSettings(handle, NVAPI_THERMAL_TARGET_ALL, &settings) == NVAPI_NO_IMPLEMENTATION);
            NV_GPU_PERF_PSTATE_ID pstate;
            REQUIRE(NvAPI_GPU_GetCurrentPstate(handle, &pstate) == NVAPI_NO_IMPLEMENTATION);
            NV_GPU_CLOCK_FREQUENCIES frequencies;
            frequencies.version = NV_GPU_CLOCK_FREQUENCIES_VER;
            frequencies.ClockType = NV_GPU_CLOCK_FREQUENCIES_CURRENT_FREQ;
            REQUIRE(NvAPI_GPU_GetAllClockFrequencies(handle, &frequencies) == NVAPI_NO_IMPLEMENTATION);
        }
    }

    SECTION("NVML depending methods succeed when NVML is available but without suitable adapter") {
        ALLOW_CALL(*nvml, IsAvailable()) // NOLINT(bugprone-use-after-move)
            .RETURN(true);
        ALLOW_CALL(*nvml, DeviceGetHandleByPciBusId_v2(_, _))
            .RETURN(NVML_ERROR_NOT_FOUND);
        ALLOW_CALL(*nvml, ErrorString(_))
            .RETURN("error");

        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml), std::move(lfx));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(primaryDisplayId, &handle) == NVAPI_OK);

        SECTION("GetVbiosVersionString returns OK when NVML is available but without suitable adapter") {
            NvAPI_ShortString revision;
            REQUIRE(NvAPI_GPU_GetVbiosVersionString(handle, revision) == NVAPI_OK);
            REQUIRE_THAT(revision, Equals("N/A"));
        }

        SECTION("NVML depending methods return handle-invalidated when NVML is available but without suitable adapter") {
            NvU32 width;
            REQUIRE(NvAPI_GPU_GetCurrentPCIEDownstreamWidth(handle, &width) == NVAPI_HANDLE_INVALIDATED);
            NvU32 irq;
            REQUIRE(NvAPI_GPU_GetIRQ(handle, &irq) == NVAPI_HANDLE_INVALIDATED);
            NvU32 cores;
            REQUIRE(NvAPI_GPU_GetGpuCoreCount(handle, &cores) == NVAPI_HANDLE_INVALIDATED);
            NV_GPU_DYNAMIC_PSTATES_INFO_EX info;
            info.version = NV_GPU_DYNAMIC_PSTATES_INFO_EX_VER;
            REQUIRE(NvAPI_GPU_GetDynamicPstatesInfoEx(handle, &info) == NVAPI_HANDLE_INVALIDATED);
            NV_GPU_THERMAL_SETTINGS settings;
            settings.version = NV_GPU_THERMAL_SETTINGS_VER;
            REQUIRE(NvAPI_GPU_GetThermalSettings(handle, NVAPI_THERMAL_TARGET_ALL, &settings) == NVAPI_HANDLE_INVALIDATED);
            NV_GPU_PERF_PSTATE_ID pstate;
            REQUIRE(NvAPI_GPU_GetCurrentPstate(handle, &pstate) == NVAPI_HANDLE_INVALIDATED);
            NV_GPU_CLOCK_FREQUENCIES frequencies;
            frequencies.version = NV_GPU_CLOCK_FREQUENCIES_VER;
            frequencies.ClockType = NV_GPU_CLOCK_FREQUENCIES_CURRENT_FREQ;
            REQUIRE(NvAPI_GPU_GetAllClockFrequencies(handle, &frequencies) == NVAPI_HANDLE_INVALIDATED);
        }
    }
}

TEST_CASE("GetHdrCapabilities succeeds", "[.sysinfo]") {
    SECTION("GetHdrCapabilities (V1) returns OK") {
        NV_HDR_CAPABILITIES_V1 capabilities;
        capabilities.version = NV_HDR_CAPABILITIES_VER1;
        REQUIRE(NvAPI_Disp_GetHdrCapabilities(0, reinterpret_cast<NV_HDR_CAPABILITIES*>(&capabilities)) == NVAPI_OK);
        REQUIRE(capabilities.isST2084EotfSupported == false);
        REQUIRE(capabilities.isTraditionalHdrGammaSupported == false);
        REQUIRE(capabilities.isEdrSupported == false);
        REQUIRE(capabilities.driverExpandDefaultHdrParameters == false);
        REQUIRE(capabilities.isTraditionalSdrGammaSupported == false);
        REQUIRE(capabilities.display_data.displayPrimary_x0 == 0);
        REQUIRE(capabilities.display_data.displayPrimary_y0 == 0);
        REQUIRE(capabilities.display_data.displayPrimary_x1 == 0);
        REQUIRE(capabilities.display_data.displayPrimary_y1 == 0);
        REQUIRE(capabilities.display_data.displayPrimary_x2 == 0);
        REQUIRE(capabilities.display_data.displayPrimary_y2 == 0);
        REQUIRE(capabilities.display_data.displayWhitePoint_x == 0);
        REQUIRE(capabilities.display_data.displayWhitePoint_y == 0);
        REQUIRE(capabilities.display_data.desired_content_min_luminance == 0);
        REQUIRE(capabilities.display_data.desired_content_max_luminance == 0);
        REQUIRE(capabilities.display_data.desired_content_max_frame_average_luminance == 0);
    }

    SECTION("GetHdrCapabilities (V2) returns OK") {
        NV_HDR_CAPABILITIES_V2 capabilities;
        capabilities.version = NV_HDR_CAPABILITIES_VER2;
        REQUIRE(NvAPI_Disp_GetHdrCapabilities(0, reinterpret_cast<NV_HDR_CAPABILITIES*>(&capabilities)) == NVAPI_OK);
        REQUIRE(capabilities.isST2084EotfSupported == false);
        REQUIRE(capabilities.isTraditionalHdrGammaSupported == false);
        REQUIRE(capabilities.isEdrSupported == false);
        REQUIRE(capabilities.driverExpandDefaultHdrParameters == false);
        REQUIRE(capabilities.isTraditionalSdrGammaSupported == false);
        REQUIRE(capabilities.display_data.displayPrimary_x0 == 0);
        REQUIRE(capabilities.display_data.displayPrimary_y0 == 0);
        REQUIRE(capabilities.display_data.displayPrimary_x1 == 0);
        REQUIRE(capabilities.display_data.displayPrimary_y1 == 0);
        REQUIRE(capabilities.display_data.displayPrimary_x2 == 0);
        REQUIRE(capabilities.display_data.displayPrimary_y2 == 0);
        REQUIRE(capabilities.display_data.displayWhitePoint_x == 0);
        REQUIRE(capabilities.display_data.displayWhitePoint_y == 0);
        REQUIRE(capabilities.display_data.desired_content_min_luminance == 0);
        REQUIRE(capabilities.display_data.desired_content_max_luminance == 0);
        REQUIRE(capabilities.display_data.desired_content_max_frame_average_luminance == 0);
    }

    SECTION("GetHdrCapabilities (V3) returns OK") {
        NV_HDR_CAPABILITIES_V3 capabilities;
        capabilities.version = NV_HDR_CAPABILITIES_VER3;
        REQUIRE(NvAPI_Disp_GetHdrCapabilities(0, &capabilities) == NVAPI_OK);
        REQUIRE(capabilities.isST2084EotfSupported == false);
        REQUIRE(capabilities.isTraditionalHdrGammaSupported == false);
        REQUIRE(capabilities.isEdrSupported == false);
        REQUIRE(capabilities.driverExpandDefaultHdrParameters == false);
        REQUIRE(capabilities.isTraditionalSdrGammaSupported == false);
        REQUIRE(capabilities.isDolbyVisionSupported == false);
        REQUIRE(capabilities.display_data.displayPrimary_x0 == 0);
        REQUIRE(capabilities.display_data.displayPrimary_y0 == 0);
        REQUIRE(capabilities.display_data.displayPrimary_x1 == 0);
        REQUIRE(capabilities.display_data.displayPrimary_y1 == 0);
        REQUIRE(capabilities.display_data.displayPrimary_x2 == 0);
        REQUIRE(capabilities.display_data.displayPrimary_y2 == 0);
        REQUIRE(capabilities.display_data.displayWhitePoint_x == 0);
        REQUIRE(capabilities.display_data.displayWhitePoint_y == 0);
        REQUIRE(capabilities.display_data.desired_content_min_luminance == 0);
        REQUIRE(capabilities.display_data.desired_content_max_luminance == 0);
        REQUIRE(capabilities.display_data.desired_content_max_frame_average_luminance == 0);
    }

    SECTION("GetHdrCapabilities with unknown struct version returns incompatible-struct-version") {
        NV_HDR_CAPABILITIES capabilities;
        capabilities.version = NV_HDR_CAPABILITIES_VER3 + 1;
        REQUIRE(NvAPI_Disp_GetHdrCapabilities(0, &capabilities) == NVAPI_INCOMPATIBLE_STRUCT_VERSION);
    }

    SECTION("GetHdrCapabilities with current struct version returns not incompatible-struct-version") {
        // This test should fail when a header update provides a newer not yet implemented struct version
        NV_HDR_CAPABILITIES capabilities;
        capabilities.version = NV_HDR_CAPABILITIES_VER;
        REQUIRE(NvAPI_Disp_GetHdrCapabilities(0, &capabilities) != NVAPI_INCOMPATIBLE_STRUCT_VERSION);
    }
}

TEST_CASE("GetDisplayViewportsByResolution returns mosaic-not-active", "[.sysinfo]") {
    NvU8 corrected;
    NV_RECT rect[NV_MOSAIC_MAX_DISPLAYS];
    REQUIRE(NvAPI_Mosaic_GetDisplayViewportsByResolution(0, 0, 0, rect, &corrected) == NVAPI_MOSAIC_NOT_ACTIVE);
}
