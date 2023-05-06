#include "nvapi_tests_private.h"
#include "resource_factory_util.h"
#include "nvapi_sysinfo_mocks.h"

using namespace trompeloeil;
using namespace Catch::Matchers;

TEST_CASE("Topology methods succeed", "[.sysinfo-topo]") {
    auto dxgiFactory = std::make_unique<DXGIFactory1Mock>();
    auto vulkan = std::make_unique<VulkanMock>();
    auto nvml = std::make_unique<NvmlMock>();
    auto lfx = std::make_unique<LfxMock>();
    DXGIDxvkAdapterMock adapter1;
    DXGIDxvkAdapterMock adapter2;
    DXGIOutput6Mock output1;
    DXGIOutput6Mock output2;
    DXGIOutput6Mock output3;

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
        NvPhysicalGpuHandle handles[NVAPI_MAX_PHYSICAL_GPUS]{};
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

    SECTION("GetLogicalGPUFromPhysicalGPU succeeds") {
        NvPhysicalGpuHandle handles[NVAPI_MAX_PHYSICAL_GPUS]{};
        NvU32 count;
        REQUIRE(NvAPI_EnumPhysicalGPUs(handles, &count) == NVAPI_OK);

        NvLogicalGpuHandle logicalhandle;
        REQUIRE(NvAPI_GetLogicalGPUFromPhysicalGPU(handles[0], &logicalhandle) == NVAPI_OK);
        REQUIRE(logicalhandle == reinterpret_cast<NvLogicalGpuHandle>(handles[0]));

        REQUIRE(NvAPI_GetLogicalGPUFromPhysicalGPU(handles[1], &logicalhandle) == NVAPI_OK);
        REQUIRE(logicalhandle == reinterpret_cast<NvLogicalGpuHandle>(handles[1]));
    }

    SECTION("GetPhysicalGPUsFromLogicalGPU succeeds") {
        NvLogicalGpuHandle handles[NVAPI_MAX_LOGICAL_GPUS]{};
        NvU32 count;
        REQUIRE(NvAPI_EnumLogicalGPUs(handles, &count) == NVAPI_OK);

        NvPhysicalGpuHandle physicalHandles[NVAPI_MAX_PHYSICAL_GPUS]{};
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

    SECTION("GetConnectedDisplayIds succeeds") {
        NvPhysicalGpuHandle gpuHandles[NVAPI_MAX_PHYSICAL_GPUS]{};
        NvU32 gpuCount = 0U;
        REQUIRE(NvAPI_EnumPhysicalGPUs(gpuHandles, &gpuCount) == NVAPI_OK);
        REQUIRE(gpuCount == 2);

        SECTION("GetConnectedDisplayIds with nullptr returns OK") {
            NvU32 gpu0DisplayIdCount = 0U;
            REQUIRE(NvAPI_GPU_GetConnectedDisplayIds(gpuHandles[0U], nullptr, &gpu0DisplayIdCount, 0) == NVAPI_OK);
            REQUIRE(gpu0DisplayIdCount == 2);

            NvU32 gpu1DisplayIdCount = 0U;
            REQUIRE(NvAPI_GPU_GetConnectedDisplayIds(gpuHandles[1U], nullptr, &gpu1DisplayIdCount, 0) == NVAPI_OK);
            REQUIRE(gpu1DisplayIdCount == 1);
        }

        SECTION("GetConnectedDisplayIds with wrong display ID count returns insufficient-buffer") {
            NvU32 gpu0DisplayIdCount = 1U;
            NV_GPU_DISPLAYIDS gpu0DisplayIds[gpu0DisplayIdCount];
            memset(&gpu0DisplayIds, 0, sizeof(gpu0DisplayIds));
            gpu0DisplayIds->version = NV_GPU_DISPLAYIDS_VER;
            REQUIRE(NvAPI_GPU_GetConnectedDisplayIds(gpuHandles[0U], gpu0DisplayIds, &gpu0DisplayIdCount, 0) == NVAPI_INSUFFICIENT_BUFFER);
            REQUIRE(gpu0DisplayIdCount == 2);
        }

        SECTION("GetConnectedDisplayIds with correct display ID count returns OK") {
            NvU32 gpu0DisplayIdCount = 2U;
            NV_GPU_DISPLAYIDS gpu0DisplayIds[gpu0DisplayIdCount];
            memset(&gpu0DisplayIds, 0, sizeof(gpu0DisplayIds));
            gpu0DisplayIds->version = NV_GPU_DISPLAYIDS_VER;
            REQUIRE(NvAPI_GPU_GetConnectedDisplayIds(gpuHandles[0U], gpu0DisplayIds, &gpu0DisplayIdCount, 0) == NVAPI_OK);
            REQUIRE(gpu0DisplayIds[0].displayId == 0x00010001);
            REQUIRE(gpu0DisplayIds[0].isConnected == true);
            REQUIRE(gpu0DisplayIds[1].displayId == 0x00010002);
            REQUIRE(gpu0DisplayIds[1].isConnected == true);

            NvU32 gpu1DisplayIdCount = 1U;
            NV_GPU_DISPLAYIDS gpu1DisplayIds[gpu1DisplayIdCount];
            memset(&gpu1DisplayIds, 0, sizeof(gpu1DisplayIds));
            gpu1DisplayIds->version = NV_GPU_DISPLAYIDS_VER;
            REQUIRE(NvAPI_GPU_GetConnectedDisplayIds(gpuHandles[1U], gpu1DisplayIds, &gpu1DisplayIdCount, 0) == NVAPI_OK);
            REQUIRE(gpu1DisplayIds[0].displayId == 0x00020001);
            REQUIRE(gpu0DisplayIds[0].isConnected == true);
        }

        SECTION("GetConnectedDisplayIds (V1) returns OK") {
            NvU32 gpu0DisplayIdCount = 2U;
            NV_GPU_DISPLAYIDS gpu0DisplayIds[gpu0DisplayIdCount];
            memset(&gpu0DisplayIds, 0, sizeof(gpu0DisplayIds));
            gpu0DisplayIds->version = NV_GPU_DISPLAYIDS_VER1;
            REQUIRE(NvAPI_GPU_GetConnectedDisplayIds(gpuHandles[0U], gpu0DisplayIds, &gpu0DisplayIdCount, 0) == NVAPI_OK);
        }

        SECTION("GetConnectedDisplayIds (V2) returns OK") {
            NvU32 gpu0DisplayIdCount = 2U;
            NV_GPU_DISPLAYIDS gpu0DisplayIds[gpu0DisplayIdCount];
            memset(&gpu0DisplayIds, 0, sizeof(gpu0DisplayIds));
            gpu0DisplayIds->version = NV_GPU_DISPLAYIDS_VER2;
            REQUIRE(NvAPI_GPU_GetConnectedDisplayIds(gpuHandles[0U], gpu0DisplayIds, &gpu0DisplayIdCount, 0) == NVAPI_OK);
        }

        SECTION("GetConnectedDisplayIds with unknown struct version returns incompatible-struct-version") {
            NvU32 gpu0DisplayIdCount = 2U;
            NV_GPU_DISPLAYIDS gpu0DisplayIds[gpu0DisplayIdCount];
            memset(&gpu0DisplayIds, 0, sizeof(gpu0DisplayIds));
            gpu0DisplayIds->version = NV_GPU_DISPLAYIDS_VER2 + 1;
            REQUIRE(NvAPI_GPU_GetConnectedDisplayIds(gpuHandles[0U], gpu0DisplayIds, &gpu0DisplayIdCount, 0) == NVAPI_INCOMPATIBLE_STRUCT_VERSION);
        }

        SECTION("GetConnectedDisplayIds with current struct version returns not incompatible-struct-version") {
            // This test should fail when a header update provides a newer not yet implemented struct version
            NvU32 gpu0DisplayIdCount = 2U;
            NV_GPU_DISPLAYIDS gpu0DisplayIds[gpu0DisplayIdCount];
            memset(&gpu0DisplayIds, 0, sizeof(gpu0DisplayIds));
            gpu0DisplayIds->version = NV_GPU_DISPLAYIDS_VER;
            REQUIRE(NvAPI_GPU_GetConnectedDisplayIds(gpuHandles[0U], gpu0DisplayIds, &gpu0DisplayIdCount, 0) != NVAPI_INCOMPATIBLE_STRUCT_VERSION);
        }
    }
}
