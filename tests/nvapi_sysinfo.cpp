#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

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

TEST_CASE("GetInterfaceVersionString returns OK", "[.sysinfo]") {
    NvAPI_ShortString desc;
    REQUIRE(NvAPI_GetInterfaceVersionString(desc) == NVAPI_OK);
    REQUIRE(strcmp(desc, "R470") == 0);
}

TEST_CASE("GetInterfaceVersionStringEx returns OK", "[.sysinfo]") {
    NvAPI_ShortString desc;
    REQUIRE(NvAPI_GetInterfaceVersionStringEx(desc) == NVAPI_OK);
    REQUIRE(std::string(desc).rfind("DXVK-NVAPI", 0) == 0);
}

TEST_CASE("GetErrorMessage returns OK", "[.sysinfo]") {
    NvAPI_ShortString desc;
    REQUIRE(NvAPI_GetErrorMessage(NVAPI_NVIDIA_DEVICE_NOT_FOUND, desc) == NVAPI_OK);
    REQUIRE(strcmp(desc, "NVAPI_NVIDIA_DEVICE_NOT_FOUND") == 0);
}

TEST_CASE("Initialize returns device-not-found when DXVK reports no adapters", "[.sysinfo]") {
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

TEST_CASE("Topology methods succeed", "[.sysinfo]") {
    auto dxgiFactory = std::make_unique<DXGIFactoryMock>();
    DXGIDxvkAdapterMock adapter1;
    auto vkDevice1 = reinterpret_cast<VkPhysicalDevice>(0x01); // Very evil, but works for testing since we use this only as identifier
    DXGIDxvkAdapterMock adapter2;
    auto vkDevice2 = reinterpret_cast<VkPhysicalDevice>(0x02); // See above comment
    DXGIOutputMock output1;
    DXGIOutputMock output2;
    DXGIOutputMock output3;
    auto vulkan = std::make_unique<VulkanMock>();
    auto nvml = std::make_unique<NvmlMock>();

    ALLOW_CALL(*dxgiFactory, AddRef())
        .RETURN(1);
    ALLOW_CALL(*dxgiFactory, Release())
        .RETURN(0);
    ALLOW_CALL(*dxgiFactory, EnumAdapters(0U, _))
        .LR_SIDE_EFFECT(*_2 = static_cast<IDXGIAdapter*>(&adapter1))
        .RETURN(S_OK);
    ALLOW_CALL(*dxgiFactory, EnumAdapters(1U, _))
        .LR_SIDE_EFFECT(*_2 = static_cast<IDXGIAdapter*>(&adapter2))
        .RETURN(S_OK);
    ALLOW_CALL(*dxgiFactory, EnumAdapters(2U, _))
        .RETURN(DXGI_ERROR_NOT_FOUND);

    ALLOW_CALL(adapter1, QueryInterface(IDXGIVkInteropAdapter::guid, _))
        .LR_SIDE_EFFECT(*_2 = static_cast<IDXGIVkInteropAdapter*>(&adapter1))
        .RETURN(S_OK);
    ALLOW_CALL(adapter1, Release())
        .RETURN(0);
    ALLOW_CALL(adapter1, EnumOutputs(0U, _))
        .LR_SIDE_EFFECT(*_2 = static_cast<IDXGIOutput*>(&output1))
        .RETURN(S_OK);
    ALLOW_CALL(adapter1, EnumOutputs(1U, _))
        .LR_SIDE_EFFECT(*_2 = static_cast<IDXGIOutput*>(&output2))
        .RETURN(S_OK);
    ALLOW_CALL(adapter1, EnumOutputs(2U, _))
        .RETURN(DXGI_ERROR_NOT_FOUND);
    ALLOW_CALL(adapter1, GetVulkanHandles(_, _))
        .LR_SIDE_EFFECT(*_2 = vkDevice1);

    ALLOW_CALL(adapter2, QueryInterface(IDXGIVkInteropAdapter::guid, _))
        .LR_SIDE_EFFECT(*_2 = static_cast<IDXGIVkInteropAdapter*>(&adapter2))
        .RETURN(S_OK);
    ALLOW_CALL(adapter2, Release())
        .RETURN(0);
    ALLOW_CALL(adapter2, EnumOutputs(0U, _))
        .LR_SIDE_EFFECT(*_2 = static_cast<IDXGIOutput*>(&output3))
        .RETURN(S_OK);
    ALLOW_CALL(adapter2, EnumOutputs(1U, _))
        .RETURN(DXGI_ERROR_NOT_FOUND);
    ALLOW_CALL(adapter2, GetVulkanHandles(_, _))
        .LR_SIDE_EFFECT(*_2 = vkDevice2);

    ALLOW_CALL(output1, Release())
        .RETURN(0);
    ALLOW_CALL(output1, GetDesc(_))
        .SIDE_EFFECT(*_1 = DXGI_OUTPUT_DESC{L"Output1", {0,0,0,0}, 1, DXGI_MODE_ROTATION_UNSPECIFIED, nullptr})
        .RETURN(S_OK);

    ALLOW_CALL(output2, Release())
        .RETURN(0);
    ALLOW_CALL(output2, GetDesc(_))
        .SIDE_EFFECT(*_1 = DXGI_OUTPUT_DESC{L"Output2", {0,0,0,0}, 1, DXGI_MODE_ROTATION_UNSPECIFIED, nullptr})
        .RETURN(S_OK);

    ALLOW_CALL(output3, Release())
        .RETURN(0);
    ALLOW_CALL(output3, GetDesc(_))
        .SIDE_EFFECT(*_1 = DXGI_OUTPUT_DESC{L"Output3", {0,0,0,0}, 1, DXGI_MODE_ROTATION_UNSPECIFIED, nullptr})
        .RETURN(S_OK);

    ALLOW_CALL(*vulkan, IsAvailable())
        .RETURN(true);
    ALLOW_CALL(*vulkan, GetDeviceExtensions(_, _))
        .RETURN(std::set<std::string>{"ext"});
    ALLOW_CALL(*vulkan, GetPhysicalDeviceProperties2(_, vkDevice1, _))
        .SIDE_EFFECT(
            ConfigureGetPhysicalDeviceProperties2(_3,
                [](auto props, auto idProps, auto pciBusInfoProps, auto driverProps, auto fragmentShadingRateProps) {
                    strcpy(props->deviceName, "Device1");
                })
        );
    ALLOW_CALL(*vulkan, GetPhysicalDeviceProperties2(_, vkDevice2, _))
        .SIDE_EFFECT(
            ConfigureGetPhysicalDeviceProperties2(_3,
                [](auto props, auto idProps, auto pciBusInfoProps, auto driverProps, auto fragmentShadingRateProps) {
                    strcpy(props->deviceName, "Device2");
                })
        );
    ALLOW_CALL(*vulkan, GetPhysicalDeviceMemoryProperties2(_, _, _));

    ALLOW_CALL(*nvml, IsAvailable())
        .RETURN(false);

    SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

    SECTION("EnumLogicalGPUs succeeds") {
        NvLogicalGpuHandle handles[NVAPI_MAX_LOGICAL_GPUS];
        NvU32 count = 0U;
        for (auto handle : handles)
            handle = nullptr;

        REQUIRE(NvAPI_EnumLogicalGPUs(handles, &count) == NVAPI_OK);
        REQUIRE(handles[0] != handles[1]);
        REQUIRE(handles[0] != nullptr);
        REQUIRE(handles[1] != nullptr);
        REQUIRE(count == 2);

        NvAPI_ShortString name1;
        REQUIRE(NvAPI_GPU_GetFullName(reinterpret_cast<NvPhysicalGpuHandle>(handles[0]), name1) == NVAPI_OK);
        REQUIRE(strcmp(name1, "Device1") == 0);

        NvAPI_ShortString name2;
        REQUIRE(NvAPI_GPU_GetFullName(reinterpret_cast<NvPhysicalGpuHandle>(handles[1]), name2) == NVAPI_OK);
        REQUIRE(strcmp(name2, "Device2") == 0);
    }

    SECTION("EnumPhysicalGPUs succeeds") {
        NvPhysicalGpuHandle handles[NVAPI_MAX_LOGICAL_GPUS];
        NvU32 count = 0U;
        for (auto handle : handles)
            handle = nullptr;

        REQUIRE(NvAPI_EnumPhysicalGPUs(handles, &count) == NVAPI_OK);
        REQUIRE(handles[0] != handles[1]);
        REQUIRE(handles[0] != nullptr);
        REQUIRE(handles[1] != nullptr);
        REQUIRE(count == 2);

        NvAPI_ShortString name1;
        REQUIRE(NvAPI_GPU_GetFullName(handles[0], name1) == NVAPI_OK);
        REQUIRE(strcmp(name1, "Device1") == 0);

        NvAPI_ShortString name2;
        REQUIRE(NvAPI_GPU_GetFullName(handles[1], name2) == NVAPI_OK);
        REQUIRE(strcmp(name2, "Device2") == 0);
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

    SECTION("GetPhysicalGPUsFromDisplay succeeds") {
        NvDisplayHandle displayHandle1 = nullptr;
        NvDisplayHandle displayHandle2 = nullptr;
        NvDisplayHandle displayHandle3 = nullptr;
        REQUIRE(NvAPI_EnumNvidiaDisplayHandle(0U, &displayHandle1) == NVAPI_OK);
        REQUIRE(NvAPI_EnumNvidiaDisplayHandle(1U, &displayHandle2) == NVAPI_OK);
        REQUIRE(NvAPI_EnumNvidiaDisplayHandle(2U, &displayHandle3) == NVAPI_OK);

        NvPhysicalGpuHandle handles1[NVAPI_MAX_PHYSICAL_GPUS];
        NvU32 count1 = 0U;
        for (auto handle : handles1)
            handle = nullptr;

        REQUIRE(NvAPI_GetPhysicalGPUsFromDisplay(displayHandle1, handles1, &count1) == NVAPI_OK);
        REQUIRE(handles1[0] != nullptr);
        REQUIRE(count1 == 1);

        NvAPI_ShortString name1;
        REQUIRE(NvAPI_GPU_GetFullName(handles1[0], name1) == NVAPI_OK);
        REQUIRE(strcmp(name1, "Device1") == 0);

        NvPhysicalGpuHandle handles2[NVAPI_MAX_PHYSICAL_GPUS];
        NvU32 count2 = 0;
        for (auto handle : handles2)
            handle = nullptr;

        REQUIRE(NvAPI_GetPhysicalGPUsFromDisplay(displayHandle2, handles2, &count2) == NVAPI_OK);
        REQUIRE(handles2[0] != nullptr);
        REQUIRE(count2 == 1);
        REQUIRE(handles2[0] == handles1[0]);

        NvPhysicalGpuHandle handles3[NVAPI_MAX_PHYSICAL_GPUS];
        NvU32 count3 = 0U;
        for (auto handle : handles3)
            handle = nullptr;

        REQUIRE(NvAPI_GetPhysicalGPUsFromDisplay(displayHandle3, handles3, &count3) == NVAPI_OK);
        REQUIRE(handles3[0] != nullptr);
        REQUIRE(count3 == 1);
        REQUIRE(handles3[0] != handles1[0]);

        NvAPI_ShortString name3;
        REQUIRE(NvAPI_GPU_GetFullName(handles3[0], name3) == NVAPI_OK);
        REQUIRE(strcmp(name3, "Device2") == 0);
    }

    SECTION("GetPhysicalGpuFromDisplayId succeeds") {
        NvPhysicalGpuHandle handle1 = nullptr;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(0U, &handle1) == NVAPI_OK);
        REQUIRE(handle1 != nullptr);

        NvAPI_ShortString name1;
        REQUIRE(NvAPI_GPU_GetFullName(handle1, name1) == NVAPI_OK);
        REQUIRE(strcmp(name1, "Device1") == 0);

        NvPhysicalGpuHandle handle2 = nullptr;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(1U, &handle2) == NVAPI_OK);
        REQUIRE(handle2 != nullptr);
        REQUIRE(handle2 == handle1);

        NvPhysicalGpuHandle handle3 = nullptr;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(2U, &handle3) == NVAPI_OK);
        REQUIRE(handle3 != nullptr);

        NvAPI_ShortString name3;
        REQUIRE(NvAPI_GPU_GetFullName(handle3, name3) == NVAPI_OK);
        REQUIRE(strcmp(name3, "Device2") == 0);

        NvPhysicalGpuHandle handle4 = nullptr;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(3U, &handle4) == NVAPI_INVALID_ARGUMENT);
        REQUIRE(handle4 == nullptr);
    }
}

TEST_CASE("Sysinfo methods succeed", "[.sysinfo]") {
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

    ::SetEnvironmentVariableA("DXVK_NVAPI_DRIVER_VERSION", "");

    SECTION("Initialize and unloads return OK") {
        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);
        REQUIRE(NvAPI_Unload() == NVAPI_OK);
    }

    SECTION("GetDriverAndBranchVersion returns OK") {
        ALLOW_CALL(*vulkan, GetDeviceExtensions(_, _)) // NOLINT(bugprone-use-after-move)
            .RETURN(std::set<std::string>{VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME});
        ALLOW_CALL(*vulkan, GetPhysicalDeviceProperties2(_, _, _))
            .SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [](auto props, auto idProps, auto pciBusInfoProps, auto driverProps, auto fragmentShadingRateProps) {
                        props->driverVersion = (470 << 22) | (35 << 12) | 1;
                    })
            );

        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvU32 version;
        NvAPI_ShortString branch;
        REQUIRE(NvAPI_SYS_GetDriverAndBranchVersion(&version, branch) == NVAPI_OK);
        REQUIRE(version == 47035);
        REQUIRE(std::string(branch).length() > 0);
    }

    SECTION("GetDisplayDriverVersion returns OK") {
        struct Data {VkDriverId driverId; unsigned int major; unsigned int minor; unsigned int patch; unsigned int expectedVersion;};
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
                        strcpy(props->deviceName, "GPU0");
                        if (args.driverId == VK_DRIVER_ID_NVIDIA_PROPRIETARY)
                            props->driverVersion = (args.major << 22) | (args.minor << 14) | (args.patch << 6);
                        else
                            props->driverVersion = (args.major << 22) | (args.minor << 12) | args.patch;
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

    SECTION("GetDisplayDriverVersion with version override returns OK") {
        struct Data {std::string override; unsigned int expectedVersion;};
        auto args = GENERATE(
            Data{"", 47045},
            Data{"0", 47045},
            Data{"99", 47045},
            Data{"100000", 47045},
            Data{"AB39976", 47045},
            Data{"39976AB", 47045},
            Data{"39976", 39976});

        ::SetEnvironmentVariableA("DXVK_NVAPI_DRIVER_VERSION", args.override.c_str());

        ALLOW_CALL(*vulkan, GetDeviceExtensions(_, _)) // NOLINT(bugprone-use-after-move)
            .RETURN(std::set<std::string>{VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME});
        ALLOW_CALL(*vulkan, GetPhysicalDeviceProperties2(_, _, _))
            .SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [](auto props, auto idProps, auto pciBusInfoProps, auto driverProps, auto fragmentShadingRateProps) {
                        props->driverVersion = (470 << 22) | (45 << 12) | 0;
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

        ::SetEnvironmentVariableA("DXVK_NVAPI_DRIVER_VERSION", "");
    }

    SECTION("GetGPUType returns OK") {
        ALLOW_CALL(*vulkan, GetPhysicalDeviceProperties2(_, _, _)) // NOLINT(bugprone-use-after-move)
            .SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [](auto props, auto idProps, auto pciBusInfoProps, auto driverProps, auto fragmentShadingRateProps) {
                        props->deviceType = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
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

    SECTION("GetPCIIdentifiers returns OK") {
        ALLOW_CALL(*vulkan, GetPhysicalDeviceProperties2(_, _, _)) // NOLINT(bugprone-use-after-move)
            .SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [](auto props, auto idProps, auto pciBusInfoProps, auto driverProps, auto fragmentShadingRateProps) {
                        props->vendorID = 0x10de;
                        props->deviceID = 0x1234;
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

    SECTION("GetFullName returns OK") {
        auto name = "High-End GPU01";
        ALLOW_CALL(*vulkan, GetPhysicalDeviceProperties2(_, _, _)) // NOLINT(bugprone-use-after-move)
            .LR_SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [&name](auto props, auto idProps, auto pciBusInfoProps, auto driverProps, auto fragmentShadingRateProps) {
                        strcpy(props->deviceName, name);
                    })
            );

        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(0, &handle) == NVAPI_OK);

        NvAPI_ShortString fullName;
        REQUIRE(NvAPI_GPU_GetFullName(handle, fullName) == NVAPI_OK);
        REQUIRE(strcmp(fullName, name) == 0);
    }

    SECTION("GetBusId returns OK") {
        auto id = 2U;
        ALLOW_CALL(*vulkan, GetDeviceExtensions(_, _)) // NOLINT(bugprone-use-after-move)
            .RETURN(std::set<std::string>{VK_EXT_PCI_BUS_INFO_EXTENSION_NAME});
        ALLOW_CALL(*vulkan, GetPhysicalDeviceProperties2(_, _, _))
            .LR_SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [&id](auto props, auto idProps, auto pciBusInfoProps, auto driverProps, auto fragmentShadingRateProps) {
                        pciBusInfoProps->pciBus = id;
                    })
            );

        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(0, &handle) == NVAPI_OK);

        NvU32 busId;
        REQUIRE(NvAPI_GPU_GetBusId(handle, &busId) == NVAPI_OK);
        REQUIRE(busId == id);
    }

    SECTION("GetPhysicalFrameBufferSize returns OK") {
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

    SECTION("NVML depending methods succeed when NVML is available") {
        ALLOW_CALL(*nvml, IsAvailable()) // NOLINT(bugprone-use-after-move)
            .RETURN(true);
        ALLOW_CALL(*nvml, DeviceGetHandleByPciBusId_v2(_, _))
            .RETURN(NVML_SUCCESS);

        SECTION("GetDynamicPstatesInfoEx returns OK") {
            auto version = "12.34";
            ALLOW_CALL(*nvml, DeviceGetVbiosVersion(_, _, _))
                .LR_SIDE_EFFECT(strcpy(_2, version))
                .RETURN(NVML_SUCCESS);

            SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml));
            REQUIRE(NvAPI_Initialize() == NVAPI_OK);

            NvPhysicalGpuHandle handle;
            REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(0, &handle) == NVAPI_OK);

            NvAPI_ShortString revision;
            REQUIRE(NvAPI_GPU_GetVbiosVersionString(handle, revision) == NVAPI_OK);
            REQUIRE(strcmp(revision, version) == 0);
        }

        SECTION("GetDynamicPstatesInfoEx returns OK") {
            auto gpuUtilization = 32U;
            auto memoryUtilization = 56U;
            ALLOW_CALL(*nvml, DeviceGetUtilizationRates(_, _)) // NOLINT(bugprone-use-after-move)
                .LR_SIDE_EFFECT({
                    _2->gpu = gpuUtilization;
                    _2->memory = memoryUtilization;
                })
                .RETURN(NVML_SUCCESS);

            SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml));
            REQUIRE(NvAPI_Initialize() == NVAPI_OK);

            NvPhysicalGpuHandle handle;
            REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(0, &handle) == NVAPI_OK);

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

        SECTION("GetThermalSettings returns OK") {
            auto temp = 65;
            ALLOW_CALL(*nvml, DeviceGetTemperature(_, _, _)) // NOLINT(bugprone-use-after-move)
                .LR_SIDE_EFFECT(*_3 = temp)
                .RETURN(NVML_SUCCESS);

            SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml));
            REQUIRE(NvAPI_Initialize() == NVAPI_OK);

            NvPhysicalGpuHandle handle;
            REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(0, &handle) == NVAPI_OK);

            NV_GPU_THERMAL_SETTINGS settings;
            settings.version = NV_GPU_THERMAL_SETTINGS_VER_2;
            REQUIRE(NvAPI_GPU_GetThermalSettings(handle, NVAPI_THERMAL_TARGET_ALL, &settings) == NVAPI_OK);
            REQUIRE(settings.count == 1);
            REQUIRE(settings.sensor[0].controller == NVAPI_THERMAL_CONTROLLER_UNKNOWN);
            REQUIRE(settings.sensor[0].target == NVAPI_THERMAL_TARGET_GPU);
            REQUIRE(settings.sensor[0].currentTemp == static_cast<int>(temp));
            REQUIRE(settings.sensor[0].defaultMaxTemp == 127);
            REQUIRE(settings.sensor[0].defaultMinTemp == -256);
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

            SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml));
            REQUIRE(NvAPI_Initialize() == NVAPI_OK);

            NvPhysicalGpuHandle handle;
            REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(0, &handle) == NVAPI_OK);

            NV_GPU_CLOCK_FREQUENCIES frequencies;
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
    }

    SECTION("NVML depending methods return no-implementation when NVML is not available") {
        ALLOW_CALL(*nvml, IsAvailable()) // NOLINT(bugprone-use-after-move)
            .RETURN(false);

        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(0, &handle) == NVAPI_OK);

        NvAPI_ShortString revision;
        REQUIRE(NvAPI_GPU_GetVbiosVersionString(handle, revision) == NVAPI_NO_IMPLEMENTATION);
        NV_GPU_DYNAMIC_PSTATES_INFO_EX info;
        info.version = NV_GPU_DYNAMIC_PSTATES_INFO_EX_VER;
        REQUIRE(NvAPI_GPU_GetDynamicPstatesInfoEx(handle, &info) == NVAPI_NO_IMPLEMENTATION);
        NV_GPU_THERMAL_SETTINGS settings;
        settings.version = NV_GPU_THERMAL_SETTINGS_VER_2;
        REQUIRE(NvAPI_GPU_GetThermalSettings(handle, NVAPI_THERMAL_TARGET_ALL, &settings) == NVAPI_NO_IMPLEMENTATION);
        NV_GPU_CLOCK_FREQUENCIES frequencies;
        frequencies.version = NV_GPU_CLOCK_FREQUENCIES_VER_2;
        frequencies.ClockType = NV_GPU_CLOCK_FREQUENCIES_CURRENT_FREQ;
        REQUIRE(NvAPI_GPU_GetAllClockFrequencies(handle, &frequencies) == NVAPI_NO_IMPLEMENTATION);
    }

    SECTION("NVML depending methods return handle-invalidated when NVML is available but without suitable adapter") {
        ALLOW_CALL(*nvml, IsAvailable()) // NOLINT(bugprone-use-after-move)
            .RETURN(true);
        ALLOW_CALL(*nvml, DeviceGetHandleByPciBusId_v2(_, _))
            .RETURN(NVML_ERROR_NOT_FOUND);
        ALLOW_CALL(*nvml, ErrorString(_))
            .RETURN("error");

        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        NvPhysicalGpuHandle handle;
        REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(0, &handle) == NVAPI_OK);

        NvAPI_ShortString revision;
        REQUIRE(NvAPI_GPU_GetVbiosVersionString(handle, revision) == NVAPI_HANDLE_INVALIDATED);
        NV_GPU_DYNAMIC_PSTATES_INFO_EX info;
        info.version = NV_GPU_DYNAMIC_PSTATES_INFO_EX_VER;
        REQUIRE(NvAPI_GPU_GetDynamicPstatesInfoEx(handle, &info) == NVAPI_HANDLE_INVALIDATED);
        NV_GPU_THERMAL_SETTINGS settings;
        settings.version = NV_GPU_THERMAL_SETTINGS_VER_2;
        REQUIRE(NvAPI_GPU_GetThermalSettings(handle, NVAPI_THERMAL_TARGET_ALL, &settings) == NVAPI_HANDLE_INVALIDATED);
        NV_GPU_CLOCK_FREQUENCIES frequencies;
        frequencies.version = NV_GPU_CLOCK_FREQUENCIES_VER_2;
        frequencies.ClockType = NV_GPU_CLOCK_FREQUENCIES_CURRENT_FREQ;
        REQUIRE(NvAPI_GPU_GetAllClockFrequencies(handle, &frequencies) == NVAPI_HANDLE_INVALIDATED);
    }
}

TEST_CASE("GetHdrCapabilities returns OK", "[.sysinfo]") {
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

TEST_CASE("GetDisplayViewportsByResolution returns mosaic-ot-active", "[.sysinfo]") {
    NvU8 corrected;
    NV_RECT rect[NV_MOSAIC_MAX_DISPLAYS];
    REQUIRE(NvAPI_Mosaic_GetDisplayViewportsByResolution(0, 0, 0, rect, &corrected) == NVAPI_MOSAIC_NOT_ACTIVE);
}
