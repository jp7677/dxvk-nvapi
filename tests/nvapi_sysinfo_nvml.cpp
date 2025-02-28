#include "nvapi_tests_private.h"
#include "nvapi/resource_factory_util.h"

using namespace trompeloeil;
using namespace Catch::Matchers;

TEST_CASE("NVML related sysinfo methods succeed", "[.sysinfo-nvml]") {
    auto dxgiFactory = std::make_unique<DXGIDxvkFactoryMock>();
    auto vk = std::make_unique<VkMock>();
    auto nvml = std::make_unique<NvmlMock>();
    DXGIDxvkAdapterMock* adapter = CreateDXGIDxvkAdapterMock();
    DXGIOutput6Mock* output = CreateDXGIOutput6Mock();

    auto e = ConfigureDefaultTestEnvironment(*dxgiFactory, *vk, *nvml, *adapter, *output);
    auto primaryDisplayId = 0x00010001;

    SECTION("NVML depending methods succeed when NVML is available") {
        ALLOW_CALL(*nvml, IsAvailable())
            .RETURN(true);
        ALLOW_CALL(*nvml, DeviceGetHandleByPciBusId_v2(_, _))
            .SIDE_EFFECT(*_2 = reinterpret_cast<nvmlDevice_t>(0x1234)) // Just a non-nullptr
            .RETURN(NVML_SUCCESS);
        ALLOW_CALL(*nvml, DeviceGetMemoryInfo_v2(_, _))
            .SIDE_EFFECT(_2->reserved = 376 * 1024)
            .RETURN(NVML_SUCCESS);

        SECTION("GetCurrentPCIEDownstreamWidth returns OK") {
            auto linkWidth = 16U;
            ALLOW_CALL(*nvml, DeviceGetCurrPcieLinkWidth(_, _))
                .LR_SIDE_EFFECT(*_2 = linkWidth)
                .RETURN(NVML_SUCCESS);

            SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));
            REQUIRE(NvAPI_Initialize() == NVAPI_OK);

            NvPhysicalGpuHandle handle;
            REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(primaryDisplayId, &handle) == NVAPI_OK);

            NvU32 width;
            REQUIRE(NvAPI_GPU_GetCurrentPCIEDownstreamWidth(handle, &width) == NVAPI_OK);
            REQUIRE(width == linkWidth);
        }

        SECTION("GetIrq returns OK") {
            auto irqNum = 143U;
            ALLOW_CALL(*nvml, DeviceGetIrqNum(_, _))
                .LR_SIDE_EFFECT(*_2 = irqNum)
                .RETURN(NVML_SUCCESS);

            SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));
            REQUIRE(NvAPI_Initialize() == NVAPI_OK);

            NvPhysicalGpuHandle handle;
            REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(primaryDisplayId, &handle) == NVAPI_OK);

            NvU32 irq;
            REQUIRE(NvAPI_GPU_GetIRQ(handle, &irq) == NVAPI_OK);
            REQUIRE(irq == irqNum);
        }

        SECTION("GetGpuCoreCount returns OK") {
            auto cores = 1536U;
            ALLOW_CALL(*nvml, DeviceGetNumGpuCores(_, _))
                .LR_SIDE_EFFECT(*_2 = cores)
                .RETURN(NVML_SUCCESS);

            SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));
            REQUIRE(NvAPI_Initialize() == NVAPI_OK);

            NvPhysicalGpuHandle handle;
            REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(primaryDisplayId, &handle) == NVAPI_OK);

            NvU32 count;
            REQUIRE(NvAPI_GPU_GetGpuCoreCount(handle, &count) == NVAPI_OK);
            REQUIRE(count == cores);
        }

        SECTION("GetPCIIdentifiers returns OK and has subsystem ID when NVML is available") {
            auto id = 0x88161043;
            ALLOW_CALL(*nvml, DeviceGetPciInfo_v3(_, _))
                .LR_SIDE_EFFECT(_2->pciSubSystemId = id)
                .RETURN(NVML_SUCCESS);

            SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));
            REQUIRE(NvAPI_Initialize() == NVAPI_OK);

            NvPhysicalGpuHandle handle;
            REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(primaryDisplayId, &handle) == NVAPI_OK);

            NvU32 deviceId, subSystemId, revisionId, extDeviceId;
            REQUIRE(NvAPI_GPU_GetPCIIdentifiers(handle, &deviceId, &subSystemId, &revisionId, &extDeviceId) == NVAPI_OK);
            REQUIRE(subSystemId == id);
        }

        SECTION("GetVbiosVersionString returns OK") {
            auto version = "12.34";
            ALLOW_CALL(*nvml, DeviceGetVbiosVersion(_, _, _))
                .LR_SIDE_EFFECT(strcpy(_2, version))
                .RETURN(NVML_SUCCESS);

            SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));
            REQUIRE(NvAPI_Initialize() == NVAPI_OK);

            NvPhysicalGpuHandle handle;
            REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(primaryDisplayId, &handle) == NVAPI_OK);

            NvAPI_ShortString revision;
            REQUIRE(NvAPI_GPU_GetVbiosVersionString(handle, revision) == NVAPI_OK);
            REQUIRE_THAT(revision, Equals(version));
        }

        SECTION("GetMemoryInfo/GetMemoryInfoEx returns OK") {
            ALLOW_CALL(*adapter, GetDesc1(_))
                .SIDE_EFFECT({
                    _1->VendorId = 0x10de;
                    _1->DedicatedVideoMemory = 8191 * 1024;
                })
                .RETURN(S_OK);
            ALLOW_CALL(*adapter, QueryVideoMemoryInfo(_, _, _))
                .RETURN(S_OK);

            SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));
            REQUIRE(NvAPI_Initialize() == NVAPI_OK);

            NvPhysicalGpuHandle handle;
            REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(primaryDisplayId, &handle) == NVAPI_OK);

            NV_DISPLAY_DRIVER_MEMORY_INFO info;
            info.version = NV_DISPLAY_DRIVER_MEMORY_INFO_VER;
            REQUIRE(NvAPI_GPU_GetMemoryInfo(handle, &info) == NVAPI_OK);
            REQUIRE(info.availableDedicatedVideoMemory == 8191 - 376);

            NV_GPU_MEMORY_INFO_EX infoEx;
            infoEx.version = NV_GPU_MEMORY_INFO_EX_VER;
            REQUIRE(NvAPI_GPU_GetMemoryInfoEx(handle, &infoEx) == NVAPI_OK);
            REQUIRE(infoEx.availableDedicatedVideoMemory == (8191 - 376) * 1024);
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

            ALLOW_CALL(*nvml, DeviceGetBusType(_, _))
                .LR_SIDE_EFFECT(*_2 = args.nvmlBusType)
                .RETURN(NVML_SUCCESS);

            SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));
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
            ALLOW_CALL(*nvml, DeviceGetDynamicPstatesInfo(_, _))
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
            ALLOW_CALL(*nvml, DeviceGetUtilizationRates(_, _))
                .LR_SIDE_EFFECT({
                    _2->gpu = gpuUtilization + 1;
                    _2->memory = fbUtilization + 1;
                })
                .RETURN(NVML_SUCCESS);

            SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));
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
            ALLOW_CALL(*nvml, DeviceGetDynamicPstatesInfo(_, _))
                .RETURN(NVML_ERROR_FUNCTION_NOT_FOUND);
            ALLOW_CALL(*nvml, DeviceGetUtilizationRates(_, _))
                .LR_SIDE_EFFECT({
                    _2->gpu = gpuUtilization;
                    _2->memory = memoryUtilization;
                })
                .RETURN(NVML_SUCCESS);

            SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));
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
            ALLOW_CALL(*nvml, DeviceGetThermalSettings(_, _, _))
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
            ALLOW_CALL(*nvml, DeviceGetTemperature(_, _, _))
                .LR_SIDE_EFFECT(*_3 = temp + 1)
                .RETURN(NVML_SUCCESS);

            SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));
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
            ALLOW_CALL(*nvml, DeviceGetThermalSettings(_, _, _))
                .RETURN(NVML_ERROR_FUNCTION_NOT_FOUND);
            ALLOW_CALL(*nvml, DeviceGetTemperature(_, _, _))
                .LR_SIDE_EFFECT(*_3 = temp)
                .RETURN(NVML_SUCCESS);

            SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));
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

        SECTION("GetTachReading returns OK") {
            ALLOW_CALL(*nvml, DeviceGetFanSpeedRPM(_, _))
                .SIDE_EFFECT(_2->speed = 800)
                .RETURN(NVML_SUCCESS);

            SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));
            REQUIRE(NvAPI_Initialize() == NVAPI_OK);

            NvPhysicalGpuHandle handle;
            REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(primaryDisplayId, &handle) == NVAPI_OK);

            NvU32 value;
            REQUIRE(NvAPI_GPU_GetTachReading(handle, &value) == NVAPI_OK);
            REQUIRE(value == 800);
        }

        SECTION("GetCurrentPstate returns OK") {
            ALLOW_CALL(*nvml, DeviceGetPerformanceState(_, _))
                .LR_SIDE_EFFECT(*_2 = NVML_PSTATE_2)
                .RETURN(NVML_SUCCESS);

            SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));
            REQUIRE(NvAPI_Initialize() == NVAPI_OK);

            NvPhysicalGpuHandle handle;
            REQUIRE(NvAPI_SYS_GetPhysicalGpuFromDisplayId(primaryDisplayId, &handle) == NVAPI_OK);

            NV_GPU_PERF_PSTATE_ID pstate;
            REQUIRE(NvAPI_GPU_GetCurrentPstate(handle, &pstate) == NVAPI_OK);
            REQUIRE(pstate == NVAPI_GPU_PERF_PSTATE_P2);
        }

        SECTION("GetAllClockFrequencies succeeds") {
            auto graphicsClock = 500U;
            auto memoryClock = 600U;
            auto videoClock = 700U;
            ALLOW_CALL(*nvml, DeviceGetClockInfo(_, NVML_CLOCK_GRAPHICS, _))
                .LR_SIDE_EFFECT(*_3 = graphicsClock)
                .RETURN(NVML_SUCCESS);
            ALLOW_CALL(*nvml, DeviceGetClockInfo(_, NVML_CLOCK_MEM, _))
                .LR_SIDE_EFFECT(*_3 = memoryClock)
                .RETURN(NVML_SUCCESS);
            ALLOW_CALL(*nvml, DeviceGetClockInfo(_, NVML_CLOCK_VIDEO, _))
                .LR_SIDE_EFFECT(*_3 = videoClock)
                .RETURN(NVML_SUCCESS);

            SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));
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

            SECTION("GetAllClockFrequencies with unknown struct version returns incompatible-struct-version") {
                NV_GPU_CLOCK_FREQUENCIES frequencies;
                frequencies.version = NV_GPU_CLOCK_FREQUENCIES_VER_3 + 1;
                REQUIRE(NvAPI_GPU_GetAllClockFrequencies(handle, &frequencies) == NVAPI_INCOMPATIBLE_STRUCT_VERSION);
            }

            SECTION("GetAllClockFrequencies with current struct version returns not incompatible-struct-version") {
                // This test should fail when a header update provides a newer not yet implemented struct version
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
    }

    SECTION("NVML depending methods succeed when NVML is not available") {
        ALLOW_CALL(*nvml, IsAvailable())
            .RETURN(false);

        SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));
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
            NvU32 value;
            REQUIRE(NvAPI_GPU_GetTachReading(handle, &value) == NVAPI_NO_IMPLEMENTATION);
            NV_GPU_PERF_PSTATE_ID pstate;
            REQUIRE(NvAPI_GPU_GetCurrentPstate(handle, &pstate) == NVAPI_NO_IMPLEMENTATION);
            NV_GPU_CLOCK_FREQUENCIES frequencies;
            frequencies.version = NV_GPU_CLOCK_FREQUENCIES_VER;
            frequencies.ClockType = NV_GPU_CLOCK_FREQUENCIES_CURRENT_FREQ;
            REQUIRE(NvAPI_GPU_GetAllClockFrequencies(handle, &frequencies) == NVAPI_NO_IMPLEMENTATION);
        }
    }

    SECTION("NVML depending methods succeed when NVML is available but without suitable adapter") {
        ALLOW_CALL(*nvml, IsAvailable())
            .RETURN(true);
        ALLOW_CALL(*nvml, DeviceGetHandleByPciBusId_v2(_, _))
            .RETURN(NVML_ERROR_NOT_FOUND);
        ALLOW_CALL(*nvml, ErrorString(_))
            .RETURN("error");

        SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));
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
            NvU32 value;
            REQUIRE(NvAPI_GPU_GetTachReading(handle, &value) == NVAPI_HANDLE_INVALIDATED);
            NV_GPU_PERF_PSTATE_ID pstate;
            REQUIRE(NvAPI_GPU_GetCurrentPstate(handle, &pstate) == NVAPI_HANDLE_INVALIDATED);
            NV_GPU_CLOCK_FREQUENCIES frequencies;
            frequencies.version = NV_GPU_CLOCK_FREQUENCIES_VER;
            frequencies.ClockType = NV_GPU_CLOCK_FREQUENCIES_CURRENT_FREQ;
            REQUIRE(NvAPI_GPU_GetAllClockFrequencies(handle, &frequencies) == NVAPI_HANDLE_INVALIDATED);
        }
    }
}
