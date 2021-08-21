#include <iostream>
#include <iomanip>
#include <libloaderapi.h>
#include "../inc/nvapi.h"
#include "../inc/nvapi_interface.h"
#include "../inc/catch.hpp"

typedef void* (*PFN_NvAPI_QueryInterface)(unsigned int id);
typedef decltype(&NvAPI_Initialize) PFN_NvAPI_Initialize;
typedef decltype(&NvAPI_Unload) PFN_NvAPI_Unload;
typedef decltype(&NvAPI_GetInterfaceVersionString) PFN_NvAPI_GetInterfaceVersionString;
typedef decltype(&NvAPI_GetInterfaceVersionStringEx) PFN_NvAPI_GetInterfaceVersionStringEx;
typedef decltype(&NvAPI_SYS_GetDriverAndBranchVersion) PFN_NvAPI_SYS_GetDriverAndBranchVersion;
typedef decltype(&NvAPI_EnumPhysicalGPUs) PFN_NvAPI_EnumPhysicalGPUs;
typedef decltype(&NvAPI_GPU_GetGPUType) PFN_NvAPI_GPU_GetGPUType;
typedef decltype(&NvAPI_GPU_GetPCIIdentifiers) PFN_NvAPI_GPU_GetPCIIdentifiers;
typedef decltype(&NvAPI_GPU_GetFullName) PFN_NvAPI_GPU_GetFullName;
typedef decltype(&NvAPI_GPU_GetBusId) PFN_NvAPI_GPU_GetBusId;
typedef decltype(&NvAPI_GPU_GetPhysicalFrameBufferSize) PFN_NvAPI_GPU_GetPhysicalFrameBufferSize;
typedef decltype(&NvAPI_GPU_GetAdapterIdFromPhysicalGpu) PFN_NvAPI_GPU_GetAdapterIdFromPhysicalGpu;
typedef decltype(&NvAPI_GPU_GetArchInfo) PFN_NvAPI_GPU_GetArchInfo;
typedef decltype(&NvAPI_GPU_GetVbiosVersionString) PFN_NvAPI_GPU_GetVbiosVersionString;
typedef decltype(&NvAPI_GPU_GetDynamicPstatesInfoEx) PFN_NvAPI_GPU_GetDynamicPstatesInfoEx;
typedef decltype(&NvAPI_GPU_GetThermalSettings) PFN_NvAPI_GPU_GetThermalSettings;
typedef decltype(&NvAPI_GPU_GetAllClockFrequencies) PFN_NvAPI_GPU_GetAllClockFrequencies;

template<typename T>
T GetNvAPIProcAddress(PFN_NvAPI_QueryInterface nvAPI_QueryInterface, const char* name) {
    auto it = std::find_if(std::begin(nvapi_interface_table), std::end(nvapi_interface_table),
        [&name](const auto& item) {
            return strcmp(item.func, name) == 0;
        });

    REQUIRE(it != std::end(nvapi_interface_table));
    return reinterpret_cast<T>(nvAPI_QueryInterface(it->id));
}

std::string ToGpuType(NV_GPU_TYPE type) {
    switch (type) {
        case NV_SYSTEM_TYPE_DGPU:
            return "Discrete";
        case NV_SYSTEM_TYPE_IGPU:
            return "Integrated";
        default:
            return "Unknown";
    }
}

std::string ToGpuArchitecture(NV_GPU_ARCHITECTURE_ID id) {
    switch (id) {
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
            return "Pre-Fermi";
    }
}

TEST_CASE("Sysinfo methods succeed against local system", "[system]") {
    const auto nvapiModuleName = "nvapi64.dll";
    auto nvapiModule = ::LoadLibraryA(nvapiModuleName);
    REQUIRE(nvapiModule != nullptr);

    auto nvAPI_QueryInterface = reinterpret_cast<PFN_NvAPI_QueryInterface>(
        reinterpret_cast<void*>(
            ::GetProcAddress(nvapiModule, "nvapi_QueryInterface")));

    auto nvAPI_Initialize = GetNvAPIProcAddress<PFN_NvAPI_Initialize>(nvAPI_QueryInterface, "NvAPI_Initialize");
    auto nvAPI_Unload = GetNvAPIProcAddress<PFN_NvAPI_Unload>(nvAPI_QueryInterface, "NvAPI_Unload");
    auto nvAPI_GetInterfaceVersionString = GetNvAPIProcAddress<PFN_NvAPI_GetInterfaceVersionString>(nvAPI_QueryInterface, "NvAPI_GetInterfaceVersionString");
    auto nvAPI_GetInterfaceVersionStringEx = GetNvAPIProcAddress<PFN_NvAPI_GetInterfaceVersionStringEx>(nvAPI_QueryInterface, "NvAPI_GetInterfaceVersionStringEx");
    auto nvAPI_SYS_GetDriverAndBranchVersion = GetNvAPIProcAddress<PFN_NvAPI_SYS_GetDriverAndBranchVersion>(nvAPI_QueryInterface, "NvAPI_SYS_GetDriverAndBranchVersion");
    auto nvAPI_EnumPhysicalGPUs = GetNvAPIProcAddress<PFN_NvAPI_EnumPhysicalGPUs>(nvAPI_QueryInterface, "NvAPI_EnumPhysicalGPUs");
    auto nvAPI_GPU_GetGPUType = GetNvAPIProcAddress<PFN_NvAPI_GPU_GetGPUType>(nvAPI_QueryInterface, "NvAPI_GPU_GetGPUType");
    auto nvAPI_GPU_GetPCIIdentifiers = GetNvAPIProcAddress<PFN_NvAPI_GPU_GetPCIIdentifiers>(nvAPI_QueryInterface, "NvAPI_GPU_GetPCIIdentifiers");
    auto nvAPI_GPU_GetFullName = GetNvAPIProcAddress<PFN_NvAPI_GPU_GetFullName>(nvAPI_QueryInterface, "NvAPI_GPU_GetFullName");
    auto nvAPI_GPU_GetBusId = GetNvAPIProcAddress<PFN_NvAPI_GPU_GetBusId>(nvAPI_QueryInterface, "NvAPI_GPU_GetBusId");
    auto nvAPI_GPU_GetPhysicalFrameBufferSize = GetNvAPIProcAddress<PFN_NvAPI_GPU_GetPhysicalFrameBufferSize>(nvAPI_QueryInterface, "NvAPI_GPU_GetPhysicalFrameBufferSize");
    auto nvAPI_GPU_GetAdapterIdFromPhysicalGpu = GetNvAPIProcAddress<PFN_NvAPI_GPU_GetAdapterIdFromPhysicalGpu>(nvAPI_QueryInterface, "NvAPI_GPU_GetAdapterIdFromPhysicalGpu");
    auto nvAPI_GPU_GetArchInfo = GetNvAPIProcAddress<PFN_NvAPI_GPU_GetArchInfo>(nvAPI_QueryInterface, "NvAPI_GPU_GetArchInfo");
    auto nvAPI_GPU_GetVbiosVersionString = GetNvAPIProcAddress<PFN_NvAPI_GPU_GetVbiosVersionString>(nvAPI_QueryInterface, "NvAPI_GPU_GetVbiosVersionString");
    auto nvAPI_GPU_GetDynamicPstatesInfoEx = GetNvAPIProcAddress<PFN_NvAPI_GPU_GetDynamicPstatesInfoEx>(nvAPI_QueryInterface, "NvAPI_GPU_GetDynamicPstatesInfoEx");
    auto nvAPI_GPU_GetThermalSettings = GetNvAPIProcAddress<PFN_NvAPI_GPU_GetThermalSettings>(nvAPI_QueryInterface, "NvAPI_GPU_GetThermalSettings");
    auto nvAPI_GPU_GetAllClockFrequencies = GetNvAPIProcAddress<PFN_NvAPI_GPU_GetAllClockFrequencies>(nvAPI_QueryInterface, "NvAPI_GPU_GetAllClockFrequencies");

    NvAPI_Status result;
    REQUIRE(nvAPI_Initialize() == NVAPI_OK);

    std::cout << "--------------------------------" << std::endl;
    NvAPI_ShortString desc;
    REQUIRE(nvAPI_GetInterfaceVersionString(desc) == NVAPI_OK);
    NvAPI_ShortString descEx;
    REQUIRE(nvAPI_GetInterfaceVersionStringEx(descEx) == NVAPI_OK);
    std::cout << "Interface version:              " << desc << " / " << descEx << std::endl;

    NvU32 version;
    NvAPI_ShortString branch;
    REQUIRE(nvAPI_SYS_GetDriverAndBranchVersion(&version, branch) == NVAPI_OK);
    std::cout << "Driver version:                 " << (version / 100) << "." << std::setfill('0') << std::setw(2) << (version % 100) << std::endl;
    std::cout << "Driver branch:                  " << branch << std::endl;

    NvPhysicalGpuHandle handles[NVAPI_MAX_LOGICAL_GPUS];
    NvU32 count;
    REQUIRE(nvAPI_EnumPhysicalGPUs(handles, &count) == NVAPI_OK);
    for (auto i = 0U; i < count; i++) {
        std::cout << "    ----------------------------" << std::endl;
        std::cout << "    GPU " << i << std::endl;
        auto handle = handles[i];

        NV_GPU_TYPE type;
        REQUIRE(nvAPI_GPU_GetGPUType(handle, &type) == NVAPI_OK);
        std::cout << "    GPU type:                   " << type << " (" << ToGpuType(type) << ")" << std::endl;

        NvU32 deviceId, subSystemId, revisionId, extDeviceId;
        REQUIRE(nvAPI_GPU_GetPCIIdentifiers(handle, &deviceId, &subSystemId, &revisionId, &extDeviceId) == NVAPI_OK);
        std::cout << "    Device ID:                  0x" << std::setfill('0') << std::setw(8) << std::hex << deviceId << std::endl;

        NvAPI_ShortString fullName;
        REQUIRE(nvAPI_GPU_GetFullName(handle, fullName) == NVAPI_OK);
        std::cout << "    Full name:                  " << fullName << std::endl;

        NvU32 busId;
        REQUIRE(nvAPI_GPU_GetBusId(handle, &busId) == NVAPI_OK);
        std::cout << "    Bus ID:                     PCI:" << std::setw(2) << std::dec << busId << std::endl;

        NvU32 size;
        REQUIRE(nvAPI_GPU_GetPhysicalFrameBufferSize(handle, &size) == NVAPI_OK);
        std::cout << "    Physical framebuffer size:  " << std::dec << size / 1024 << "MB" << std::endl;

        LUID luid;
        result = nvAPI_GPU_GetAdapterIdFromPhysicalGpu(handle, static_cast<void*>(&luid));
        std::cout << "    LUID high part:             ";
        result == NVAPI_OK
            ?   std::cout << "0x" << std::setfill('0') << std::setw(8) << std::hex << luid.HighPart << std::endl
            :   std::cout << "N/A" << std::endl;

        std::cout << "    LUID low part:              ";
        result == NVAPI_OK
            ? std::cout << "0x" << std::setfill('0') << std::setw(8) << std::hex << luid.LowPart << std::endl
            : std::cout << "N/A" << std::endl;

        NV_GPU_ARCH_INFO archInfo;
        archInfo.version = NV_GPU_ARCH_INFO_VER_2;
        result = nvAPI_GPU_GetArchInfo(handle, &archInfo);
        std::cout << "    Architecture ID:            ";
        result == NVAPI_OK
            ? std::cout << "0x" << std::setfill('0') << std::setw(8) << std::hex << archInfo.architecture_id << " (" << ToGpuArchitecture(archInfo.architecture_id) << ")" <<  std::endl
            : std::cout << "N/A" << std::endl;
        std::cout << "    Implementation ID:          ";
        result == NVAPI_OK
            ? std::cout << "0x" << std::setfill('0') << std::setw(8) << std::hex << archInfo.implementation_id << std::endl
            : std::cout << "N/A" << std::endl;

        NvAPI_ShortString revision;
        result = nvAPI_GPU_GetVbiosVersionString(handle, revision);
        std::cout << "    VBIOS version:              " << (result == NVAPI_OK ? revision : "N/A") << std::endl;

        NV_GPU_DYNAMIC_PSTATES_INFO_EX info;
        info.version = NV_GPU_DYNAMIC_PSTATES_INFO_EX_VER;
        result = nvAPI_GPU_GetDynamicPstatesInfoEx(handle, &info);
        std::cout << "    Current GPU utilization:    ";
        result == NVAPI_OK
            ? std::cout << std::dec << info.utilization[0].percentage << "%" << std::endl
            : std::cout << "N/A" << std::endl;
        std::cout << "    Current memory utilization: ";
        result == NVAPI_OK
            ? std::cout << std::dec << info.utilization[1].percentage << "%" << std::endl
            : std::cout << "N/A" << std::endl;

        NV_GPU_THERMAL_SETTINGS settings;
        settings.version = NV_GPU_THERMAL_SETTINGS_VER_2;
        result = nvAPI_GPU_GetThermalSettings(handle, NVAPI_THERMAL_TARGET_ALL, &settings);
        std::cout << "    Current GPU temperature:    ";
        result == NVAPI_OK
            ? std::cout << std::dec << settings.sensor[0].currentTemp << "C" << std::endl
            : std::cout << "N/A" << std::endl;

        NV_GPU_CLOCK_FREQUENCIES frequencies;
        frequencies.version = NV_GPU_CLOCK_FREQUENCIES_VER_2;
        frequencies.ClockType = NV_GPU_CLOCK_FREQUENCIES_CURRENT_FREQ;
        result = nvAPI_GPU_GetAllClockFrequencies(handle, &frequencies);
        std::cout << "    Current graphics clock:     ";
        result == NVAPI_OK && frequencies.domain[NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS].bIsPresent
            ? std::cout << std::dec << frequencies.domain[NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS].frequency / 1000 << "MHz" << std::endl
            : std::cout << "N/A" << std::endl;
        std::cout << "    Current memory clock:       ";
        result == NVAPI_OK && frequencies.domain[NVAPI_GPU_PUBLIC_CLOCK_MEMORY].bIsPresent
            ? std::cout << std::dec << frequencies.domain[NVAPI_GPU_PUBLIC_CLOCK_MEMORY].frequency / 1000 << "MHz" << std::endl
            : std::cout << "N/A" << std::endl;
        std::cout << "    Current video clock:        ";
        result == NVAPI_OK && frequencies.domain[NVAPI_GPU_PUBLIC_CLOCK_VIDEO].bIsPresent
            ? std::cout << std::dec << frequencies.domain[NVAPI_GPU_PUBLIC_CLOCK_VIDEO].frequency / 1000 << "MHz" << std::endl
            : std::cout << "N/A" << std::endl;
    }

    REQUIRE(nvAPI_Unload() == NVAPI_OK);
    ::FreeLibrary(nvapiModule);
}
