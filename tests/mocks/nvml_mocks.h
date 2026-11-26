#pragma once

#include "../../src/nvapi_private.h"
#include "../../src/nvapi/nvml.h"

class NvmlMock final : public trompeloeil::mock_interface<dxvk::Nvml> {
    IMPLEMENT_CONST_MOCK0(IsAvailable);
    IMPLEMENT_CONST_MOCK1(ErrorString);
    IMPLEMENT_CONST_MOCK2(DeviceGetHandleByPciBusId_v2);
    IMPLEMENT_CONST_MOCK2(DeviceGetMemoryInfo_v2);
    IMPLEMENT_CONST_MOCK2(DeviceGetPciInfo_v3);
    IMPLEMENT_CONST_MOCK3(DeviceGetClockInfo);
    IMPLEMENT_CONST_MOCK3(DeviceGetMaxClockInfo);
    IMPLEMENT_CONST_MOCK2(DeviceGetTemperatureV);
    IMPLEMENT_CONST_MOCK3(DeviceGetThermalSettings);
    IMPLEMENT_CONST_MOCK2(DeviceGetFanSpeedRPM);
    IMPLEMENT_CONST_MOCK2(DeviceGetPerformanceState);
    IMPLEMENT_CONST_MOCK2(DeviceGetUtilizationRates);
    IMPLEMENT_CONST_MOCK3(DeviceGetVbiosVersion);
    IMPLEMENT_CONST_MOCK2(DeviceGetCurrPcieLinkWidth);
    IMPLEMENT_CONST_MOCK2(DeviceGetIrqNum);
    IMPLEMENT_CONST_MOCK2(DeviceGetNumGpuCores);
    IMPLEMENT_CONST_MOCK2(DeviceGetBusType);
    IMPLEMENT_CONST_MOCK2(DeviceGetDynamicPstatesInfo);
};
