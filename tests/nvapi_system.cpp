#include <iostream>
#include <iomanip>
#include <windows.h>
#include "../inc/nvapi.h"
#include "../inc/nvapi_interface.h"
#include "../inc/catch_amalgamated.hpp"

using namespace Catch::Matchers;

typedef void* (*PFN_NvAPI_QueryInterface)(uint32_t id);

template <typename T>
T GetNvAPIProcAddress(PFN_NvAPI_QueryInterface nvAPI_QueryInterface, const char* name) {
    auto it = std::find_if(std::begin(nvapi_interface_table), std::end(nvapi_interface_table),
        [&name](const auto& item) {
            return strcmp(item.func, name) == 0;
        });

    REQUIRE(it != std::end(nvapi_interface_table));
    return reinterpret_cast<T>(nvAPI_QueryInterface(it->id));
}

static std::string ToGpuType(NV_GPU_TYPE type) {
    switch (type) {
        case NV_SYSTEM_TYPE_DGPU:
            return "Discrete";
        case NV_SYSTEM_TYPE_IGPU:
            return "Integrated";
        default:
            return "Unknown";
    }
}

static std::string ToGpuArchitecture(NV_GPU_ARCHITECTURE_ID id) {
    switch (id) {
        case NV_GPU_ARCHITECTURE_AD100:
            return "Ada";
        case NV_GPU_ARCHITECTURE_GA100:
            return "Ampere";
        case NV_GPU_ARCHITECTURE_TU100:
            return "Turing";
        case NV_GPU_ARCHITECTURE_GV100:
        case NV_GPU_ARCHITECTURE_GV110:
            return "Volta";
        case NV_GPU_ARCHITECTURE_GP100:
            return "Pascal";
        case NV_GPU_ARCHITECTURE_GM000:
        case NV_GPU_ARCHITECTURE_GM200:
            return "Maxwell";
        case NV_GPU_ARCHITECTURE_GK100:
        case NV_GPU_ARCHITECTURE_GK110:
        case NV_GPU_ARCHITECTURE_GK200:
            return "Kepler";
        case NV_GPU_ARCHITECTURE_GF100:
        case NV_GPU_ARCHITECTURE_GF110:
            return "Fermi";
        default:
            if (id > NV_GPU_ARCHITECTURE_AD100)
                return "Post-Ada";
            else
                return "Pre-Fermi";
    }
}

static std::string ToFormattedLuid(LUID& luid) {
    auto uid = static_cast<uint8_t*>(static_cast<void*>(&luid));

    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (auto i = 0U; i < sizeof(luid); i++) {
        if (i == 4)
            ss << '-';
        ss << std::setw(2) << static_cast<unsigned>(uid[i]);
    }
    return ss.str();
}

TEST_CASE("Sysinfo methods succeed against local system", "[system]") {
    ::SetEnvironmentVariableA("DXVK_ENABLE_NVAPI", "1");
    ::SetEnvironmentVariableA("DXVK_NVAPI_ALLOW_OTHER_DRIVERS", "1");

    const auto nvapiModuleName = "nvapi64.dll";
    auto nvapiModule = ::LoadLibraryA(nvapiModuleName);
    REQUIRE(nvapiModule);

    auto nvAPI_QueryInterface = reinterpret_cast<PFN_NvAPI_QueryInterface>(
        reinterpret_cast<void*>(
            ::GetProcAddress(nvapiModule, "nvapi_QueryInterface")));

    REQUIRE(nvAPI_QueryInterface);

#define GETNVAPIPROCADDR(x) auto nvAPI_##x = GetNvAPIProcAddress<decltype(&NvAPI_##x)>(nvAPI_QueryInterface, "NvAPI_" #x)

    GETNVAPIPROCADDR(Initialize);
    GETNVAPIPROCADDR(Unload);
    GETNVAPIPROCADDR(GetInterfaceVersionString);
    GETNVAPIPROCADDR(SYS_GetDisplayDriverInfo);
    GETNVAPIPROCADDR(EnumPhysicalGPUs);
    GETNVAPIPROCADDR(GetGPUIDfromPhysicalGPU);
    GETNVAPIPROCADDR(GPU_GetFullName);
    GETNVAPIPROCADDR(GPU_GetGPUType);
    GETNVAPIPROCADDR(GPU_GetPCIIdentifiers);
    GETNVAPIPROCADDR(GPU_GetBusId);
    GETNVAPIPROCADDR(GPU_GetBusSlotId);
    GETNVAPIPROCADDR(GPU_GetPhysicalFrameBufferSize);
    GETNVAPIPROCADDR(GPU_GetAdapterIdFromPhysicalGpu);
    GETNVAPIPROCADDR(GPU_GetArchInfo);
    GETNVAPIPROCADDR(GPU_GetCurrentPCIEDownstreamWidth);
    GETNVAPIPROCADDR(GPU_GetIRQ);
    GETNVAPIPROCADDR(GPU_GetGpuCoreCount);
    GETNVAPIPROCADDR(GPU_CudaEnumComputeCapableGpus);
    GETNVAPIPROCADDR(GPU_GetVbiosVersionString);
    GETNVAPIPROCADDR(GPU_GetDynamicPstatesInfoEx);
    GETNVAPIPROCADDR(GPU_GetThermalSettings);
    GETNVAPIPROCADDR(GPU_GetCurrentPstate);
    GETNVAPIPROCADDR(GPU_GetAllClockFrequencies);
    GETNVAPIPROCADDR(GPU_GetConnectedDisplayIds);
    GETNVAPIPROCADDR(DISP_GetGDIPrimaryDisplayId);
    GETNVAPIPROCADDR(EnumNvidiaDisplayHandle);
    GETNVAPIPROCADDR(SYS_GetPhysicalGpuFromDisplayId);
    GETNVAPIPROCADDR(GetAssociatedNvidiaDisplayName);
    GETNVAPIPROCADDR(DISP_GetDisplayIdByDisplayName);
    GETNVAPIPROCADDR(Disp_GetHdrCapabilities);

#undef GETNVAPIPROCADDR

    REQUIRE(nvAPI_Initialize);
    REQUIRE(nvAPI_Unload);
    REQUIRE(nvAPI_GetInterfaceVersionString);
    REQUIRE(nvAPI_SYS_GetDisplayDriverInfo);
    REQUIRE(nvAPI_EnumPhysicalGPUs);
    REQUIRE(nvAPI_GetGPUIDfromPhysicalGPU);
    REQUIRE(nvAPI_GPU_GetFullName);
    REQUIRE(nvAPI_GPU_GetGPUType);
    REQUIRE(nvAPI_GPU_GetPCIIdentifiers);
    REQUIRE(nvAPI_GPU_GetBusId);
    REQUIRE(nvAPI_GPU_GetBusSlotId);
    REQUIRE(nvAPI_GPU_GetPhysicalFrameBufferSize);
    REQUIRE(nvAPI_GPU_GetAdapterIdFromPhysicalGpu);
    REQUIRE(nvAPI_GPU_GetArchInfo);
    REQUIRE(nvAPI_GPU_GetCurrentPCIEDownstreamWidth);
    REQUIRE(nvAPI_GPU_GetIRQ);
    REQUIRE(nvAPI_GPU_GetGpuCoreCount);
    REQUIRE(nvAPI_GPU_CudaEnumComputeCapableGpus);
    REQUIRE(nvAPI_GPU_GetVbiosVersionString);
    REQUIRE(nvAPI_GPU_GetDynamicPstatesInfoEx);
    REQUIRE(nvAPI_GPU_GetThermalSettings);
    REQUIRE(nvAPI_GPU_GetCurrentPstate);
    REQUIRE(nvAPI_GPU_GetAllClockFrequencies);
    REQUIRE(nvAPI_GPU_GetConnectedDisplayIds);
    REQUIRE(nvAPI_DISP_GetGDIPrimaryDisplayId);
    REQUIRE(nvAPI_EnumNvidiaDisplayHandle);
    REQUIRE(nvAPI_SYS_GetPhysicalGpuFromDisplayId);
    REQUIRE(nvAPI_GetAssociatedNvidiaDisplayName);
    REQUIRE(nvAPI_DISP_GetDisplayIdByDisplayName);
    REQUIRE(nvAPI_Disp_GetHdrCapabilities);

    NvAPI_Status result;
    REQUIRE(nvAPI_Initialize() == NVAPI_OK);

    NvAPI_ShortString desc;
    REQUIRE(nvAPI_GetInterfaceVersionString(desc) == NVAPI_OK);
    std::cout << "--------------------------------" << std::endl;
    std::cout << "Interface version:              " << desc << std::endl;

    NV_DISPLAY_DRIVER_INFO driverInfo;
    driverInfo.version = NV_DISPLAY_DRIVER_INFO_VER;
    REQUIRE(nvAPI_SYS_GetDisplayDriverInfo(&driverInfo) == NVAPI_OK);
    std::cout << "Driver version:                 " << (driverInfo.driverVersion / 100) << "." << std::setfill('0') << std::setw(2) << (driverInfo.driverVersion % 100) << std::endl;
    std::cout << "Driver branch:                  " << driverInfo.szBuildBranch << std::endl;
    std::cout << "Driver base branch:             " << driverInfo.szBuildBaseBranch << std::endl;

    NV_COMPUTE_GPU_TOPOLOGY_V1 computeGpuTopology; // Version 2 returns `NVAPI_INCOMPATIBLE_STRUCT_VERSION` on NVIDIA's NVAPI on Windows, so enforce version 1
    computeGpuTopology.version = NV_COMPUTE_GPU_TOPOLOGY_VER1;
    REQUIRE(nvAPI_GPU_CudaEnumComputeCapableGpus(reinterpret_cast<NV_COMPUTE_GPU_TOPOLOGY*>(&computeGpuTopology)) == NVAPI_OK);
    std::vector<NV_COMPUTE_GPU> computeGpus;
    for (auto i = 0U; i < computeGpuTopology.gpuCount; i++)
        computeGpus.push_back(NV_COMPUTE_GPU{computeGpuTopology.computeGpus[i].hPhysicalGpu, computeGpuTopology.computeGpus[i].flags});

    NvPhysicalGpuHandle handles[NVAPI_MAX_PHYSICAL_GPUS]{};
    NvU32 count;
    REQUIRE(nvAPI_EnumPhysicalGPUs(handles, &count) == NVAPI_OK);

    std::vector<NvPhysicalGpuHandle> gpuHandles;
    for (auto i = 0U; i < count; i++)
        gpuHandles.push_back(handles[i]);

    for (const auto handle : gpuHandles) {
        std::cout << "    ----------------------------" << std::endl;
        std::cout << "    GPU "
                  << std::distance(gpuHandles.begin(), std::find(gpuHandles.begin(), gpuHandles.end(), handle))
                  << std::endl;

        NvAPI_ShortString fullName;
        REQUIRE(nvAPI_GPU_GetFullName(handle, fullName) == NVAPI_OK);
        std::cout << "    GPU name:                   " << fullName << std::endl;

        NV_GPU_TYPE type;
        REQUIRE(nvAPI_GPU_GetGPUType(handle, &type) == NVAPI_OK);
        std::cout << "    GPU type:                   " << type << " (" << ToGpuType(type) << ")" << std::endl;

        NvU32 deviceId, subSystemId, revisionId, extDeviceId;
        REQUIRE(nvAPI_GPU_GetPCIIdentifiers(handle, &deviceId, &subSystemId, &revisionId, &extDeviceId) == NVAPI_OK);
        std::cout << "    Device ID:                  0x" << std::setfill('0') << std::setw(8) << std::hex << deviceId << std::endl;
        std::cout << "    Subsystem ID:               ";
        if (subSystemId != 0)
            std::cout << "0x" << std::setfill('0') << std::setw(8) << std::hex << subSystemId << std::endl;
        else
            std::cout << "N/A" << std::endl;

        NvU32 busId;
        REQUIRE(nvAPI_GPU_GetBusId(handle, &busId) == NVAPI_OK);
        NvU32 busSlotId;
        REQUIRE(nvAPI_GPU_GetBusSlotId(handle, &busSlotId) == NVAPI_OK);
        std::cout << "    Bus:Slot ID:                PCI:" << std::setw(2) << std::hex << busId << ":" << std::setw(2) << std::hex << busSlotId << std::endl;

        NvU32 gpuId;
        REQUIRE(nvAPI_GetGPUIDfromPhysicalGPU(handle, &gpuId) == NVAPI_OK);
        std::cout << "    Board ID:                   0x" << std::hex << gpuId << std::endl;

        NvU32 size;
        REQUIRE(nvAPI_GPU_GetPhysicalFrameBufferSize(handle, &size) == NVAPI_OK);
        std::cout << "    Physical framebuffer size:  " << std::dec << size / 1024 << "MB" << std::endl;

        LUID luid;
        result = nvAPI_GPU_GetAdapterIdFromPhysicalGpu(handle, static_cast<void*>(&luid));
        std::cout << "    Adapter ID/LUID:            ";
        if (result == NVAPI_OK)
            std::cout << ToFormattedLuid(luid) << " ("
                      << "0x" << std::setfill('0') << std::setw(8) << std::hex << luid.HighPart << "/"
                      << "0x" << std::setfill('0') << std::setw(8) << std::hex << luid.LowPart << ")" << std::endl;
        else
            std::cout << "N/A" << std::endl;

        NV_GPU_ARCH_INFO archInfo;
        archInfo.version = NV_GPU_ARCH_INFO_VER;
        result = nvAPI_GPU_GetArchInfo(handle, &archInfo);
        std::cout << "    Architecture ID:            ";
        if (result == NVAPI_OK)
            std::cout << "0x" << std::setfill('0') << std::setw(8)
                      << std::hex << archInfo.architecture_id << " ("
                      << ToGpuArchitecture(archInfo.architecture_id) << ")"
                      << std::endl;
        else
            std::cout << "N/A" << std::endl;

        std::cout << "    Implementation ID:          ";
        if (result == NVAPI_OK)
            std::cout << "0x" << std::setfill('0') << std::setw(8) << std::hex << archInfo.implementation_id << std::endl;
        else
            std::cout << "N/A" << std::endl;

        NvU32 gpuCoreCount;
        result = nvAPI_GPU_GetGpuCoreCount(handle, &gpuCoreCount);
        std::cout << "    GPU core count:             ";
        if (result == NVAPI_OK)
            std::cout << std::dec << gpuCoreCount << std::endl;
        else
            std::cout << "N/A" << std::endl;

        NvU32 gpuPcieWidth;
        result = nvAPI_GPU_GetCurrentPCIEDownstreamWidth(handle, &gpuPcieWidth);
        std::cout << "    Current PCIe link width:    ";
        if (result == NVAPI_OK)
            std::cout << "x" << std::dec << gpuPcieWidth << std::endl;
        else
            std::cout << "N/A" << std::endl;

        NvU32 gpuIrq;
        result = nvAPI_GPU_GetIRQ(handle, &gpuIrq);
        std::cout << "    IRQ:                        ";
        if (result == NVAPI_OK)
            std::cout << std::dec << gpuIrq << std::endl;
        else
            std::cout << "N/A" << std::endl;

        auto computeGpusIt = std::find_if(
            std::begin(computeGpus),
            std::end(computeGpus),
            [handle](const auto& cudaGpu) { return cudaGpu.hPhysicalGpu == handle; });
        std::cout << "    Compute capable:            ";
        if (computeGpusIt != std::end(computeGpus))
            std::cout << "Yes (Compute GPU topology flags: 0x" << std::setfill('0') << std::setw(2)
                      << std::hex << (computeGpusIt->flags) << ")"
                      << std::endl; // TODO: print other info if available
        else
            std::cout << "-" << std::endl;

        NvAPI_ShortString revision;
        result = nvAPI_GPU_GetVbiosVersionString(handle, revision);
        std::cout << "    VBIOS version:              " << (result == NVAPI_OK ? revision : "N/A") << std::endl;

        NV_GPU_DYNAMIC_PSTATES_INFO_EX info;
        info.version = NV_GPU_DYNAMIC_PSTATES_INFO_EX_VER;
        result = nvAPI_GPU_GetDynamicPstatesInfoEx(handle, &info);
        std::cout << "    Current GPU utilization:    ";
        if (result == NVAPI_OK)
            std::cout << std::dec << info.utilization[0].percentage << "%" << std::endl;
        else
            std::cout << "N/A" << std::endl;

        std::cout << "    Current memory utilization: ";
        if (result == NVAPI_OK)
            std::cout << std::dec << info.utilization[1].percentage << "%" << std::endl;
        else
            std::cout << "N/A" << std::endl;

        std::cout << "    Current video utilization:  ";
        if (result == NVAPI_OK)
            std::cout << std::dec << info.utilization[2].percentage << "%" << std::endl;
        else
            std::cout << "N/A" << std::endl;

        NV_GPU_THERMAL_SETTINGS settings;
        settings.version = NV_GPU_THERMAL_SETTINGS_VER;
        result = nvAPI_GPU_GetThermalSettings(handle, NVAPI_THERMAL_TARGET_ALL, &settings);
        std::cout << "    Current GPU temperature:    ";
        if (result == NVAPI_OK)
            std::cout << std::dec << settings.sensor[0].currentTemp << "C" << std::endl;
        else
            std::cout << "N/A" << std::endl;

        NV_GPU_PERF_PSTATE_ID currentPstate;
        result = nvAPI_GPU_GetCurrentPstate(handle, &currentPstate);
        std::cout << "    Current performance state:  ";
        if (result == NVAPI_OK)
            std::cout << "P" << std::dec << currentPstate << std::endl;
        else
            std::cout << "N/A" << std::endl;

        NV_GPU_CLOCK_FREQUENCIES frequencies;
        frequencies.version = NV_GPU_CLOCK_FREQUENCIES_VER;
        frequencies.ClockType = NV_GPU_CLOCK_FREQUENCIES_CURRENT_FREQ;
        result = nvAPI_GPU_GetAllClockFrequencies(handle, &frequencies);

        std::cout << "    Current graphics clock:     ";
        if (result == NVAPI_OK && frequencies.domain[NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS].bIsPresent)
            std::cout << std::dec << frequencies.domain[NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS].frequency / 1000 << "MHz" << std::endl;
        else
            std::cout << "N/A" << std::endl;

        std::cout << "    Current memory clock:       ";
        if (result == NVAPI_OK && frequencies.domain[NVAPI_GPU_PUBLIC_CLOCK_MEMORY].bIsPresent)
            std::cout << std::dec << frequencies.domain[NVAPI_GPU_PUBLIC_CLOCK_MEMORY].frequency / 1000 << "MHz" << std::endl;
        else
            std::cout << "N/A" << std::endl;

        std::cout << "    Current video clock:        ";
        if (result == NVAPI_OK && frequencies.domain[NVAPI_GPU_PUBLIC_CLOCK_VIDEO].bIsPresent)
            std::cout << std::dec << frequencies.domain[NVAPI_GPU_PUBLIC_CLOCK_VIDEO].frequency / 1000 << "MHz" << std::endl;
        else
            std::cout << "N/A" << std::endl;

        NvU32 displayIdCount = 0U;
        REQUIRE(nvAPI_GPU_GetConnectedDisplayIds(handle, nullptr, &displayIdCount, 0) == NVAPI_OK);
        std::cout << "    Connected display ID(s):    ";
        if (displayIdCount != 0) {
            NV_GPU_DISPLAYIDS displayIds[displayIdCount];
            memset(&displayIds, 0, sizeof(displayIds));
            displayIds->version = NV_GPU_DISPLAYIDS_VER;
            REQUIRE(nvAPI_GPU_GetConnectedDisplayIds(handle, displayIds, &displayIdCount, 0) == NVAPI_OK);
            for (auto i = 0U; i < displayIdCount; ++i) {
                auto displayId = displayIds[i];
                std::cout << (i == 0 ? "0x" : "                                0x") << std::setfill('0') << std::setw(8)
                          << std::hex << displayId.displayId
                          << std::endl;
            }
        } else
            std::cout << "-" << std::endl;
    }

    std::cout << std::endl;

    NvU32 primaryDisplayId;
    REQUIRE_THAT(nvAPI_DISP_GetGDIPrimaryDisplayId(&primaryDisplayId), Predicate<NvAPI_Status>([](auto s) -> bool { return s == NVAPI_OK || s == NVAPI_NVIDIA_DEVICE_NOT_FOUND; }));
    NvDisplayHandle handle;
    NvU32 i = 0;
    while (nvAPI_EnumNvidiaDisplayHandle(i, &handle) == NVAPI_OK) {
        std::cout << "    ----------------------------" << std::endl;
        std::cout << "    Display " << i << std::endl;
        NvAPI_ShortString displayName;
        REQUIRE(nvAPI_GetAssociatedNvidiaDisplayName(handle, displayName) == NVAPI_OK);
        NvU32 displayId;
        REQUIRE(nvAPI_DISP_GetDisplayIdByDisplayName(displayName, &displayId) == NVAPI_OK);
        std::cout << "    Display name:               " << displayName;
        if (displayId == primaryDisplayId)
            std::cout << " (Primary)" << std::endl;
        else
            std::cout << std::endl;

        std::cout << "    Display ID:                 0x" << std::setfill('0') << std::setw(8)
                  << std::hex << displayId
                  << std::endl;

        NvPhysicalGpuHandle connectedGpuHandle;
        REQUIRE(nvAPI_SYS_GetPhysicalGpuFromDisplayId(displayId, &connectedGpuHandle) == NVAPI_OK);
        std::cout << "    Connected to:               GPU "
                  << std::distance(gpuHandles.begin(), std::find(gpuHandles.begin(), gpuHandles.end(), connectedGpuHandle))
                  << std::endl;

        NV_HDR_CAPABILITIES hdrCapabilities;
        hdrCapabilities.version = NV_HDR_CAPABILITIES_VER2; // Support drivers older than 520
        REQUIRE(nvAPI_Disp_GetHdrCapabilities(displayId, &hdrCapabilities) == NVAPI_OK);
        std::cout << "    ST2084/HDR support:         ";
        if (hdrCapabilities.isST2084EotfSupported)
            std::cout << "Yes" << std::endl;
        else
            std::cout << "-" << std::endl;

        std::cout << "    Primary 0/red:              ";
        std::cout << "X = " << std::dec << hdrCapabilities.display_data.displayPrimary_x0;
        std::cout << ", Y = " << std::dec << hdrCapabilities.display_data.displayPrimary_y0 << std::endl;
        std::cout << "    Primary 1/green:            ";
        std::cout << "X = " << std::dec << hdrCapabilities.display_data.displayPrimary_x1;
        std::cout << ", Y = " << std::dec << hdrCapabilities.display_data.displayPrimary_y1 << std::endl;
        std::cout << "    Primary 2/blue:             ";
        std::cout << "X = " << std::dec << hdrCapabilities.display_data.displayPrimary_x2;
        std::cout << ", Y = " << std::dec << hdrCapabilities.display_data.displayPrimary_y2 << std::endl;
        std::cout << "    White point:                ";
        std::cout << "X = " << std::dec << hdrCapabilities.display_data.displayWhitePoint_x;
        std::cout << ", Y = " << std::dec << hdrCapabilities.display_data.displayWhitePoint_y << std::endl;
        std::cout << "    Desired luminance:          ";
        std::cout << "Min = " << static_cast<float>(hdrCapabilities.display_data.desired_content_min_luminance) * 0.0001f;
        std::cout << ", Max = " << std::dec << hdrCapabilities.display_data.desired_content_max_luminance;
        std::cout << ", Max frame avg = " << std::dec << hdrCapabilities.display_data.desired_content_max_frame_average_luminance << std::endl;

        i++;
    }

    std::cout << std::endl;

    REQUIRE(nvAPI_Unload() == NVAPI_OK);
    ::FreeLibrary(nvapiModule);
}
