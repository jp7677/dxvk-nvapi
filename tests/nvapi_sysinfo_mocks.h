#pragma once

#include "nvapi_tests_private.h"
#include "../src/interfaces/dxvk_interfaces.h"
#include "../src/nvapi/vk.h"
#include "../src/nvapi/nvml.h"

class IDXGIDxvkFactoryMock : public IDXGIFactory1, public IDXGIVkInteropFactory1 {};

class DXGIDxvkFactoryMock final : public trompeloeil::mock_interface<IDXGIDxvkFactoryMock> {
    MAKE_MOCK2(QueryInterface, HRESULT(REFIID, void**), override);
    MAKE_MOCK0(AddRef, ULONG(), override);
    MAKE_MOCK0(Release, ULONG(), override);
    IMPLEMENT_MOCK3(GetPrivateData);
    IMPLEMENT_MOCK3(SetPrivateData);
    IMPLEMENT_MOCK2(SetPrivateDataInterface);
    IMPLEMENT_MOCK2(GetParent);
    IMPLEMENT_MOCK2(EnumAdapters);
    IMPLEMENT_MOCK2(MakeWindowAssociation);
    IMPLEMENT_MOCK1(GetWindowAssociation);
    IMPLEMENT_MOCK3(CreateSwapChain);
    IMPLEMENT_MOCK2(CreateSoftwareAdapter);
    IMPLEMENT_MOCK2(EnumAdapters1);
    IMPLEMENT_MOCK0(IsCurrent);
    IMPLEMENT_MOCK2(GetVulkanInstance);
    IMPLEMENT_MOCK2(GetGlobalHDRState);
    IMPLEMENT_MOCK2(SetGlobalHDRState);
};

class IDXGIDxvkAdapter : public IDXGIAdapter3, public IDXGIVkInteropAdapter {};

class DXGIDxvkAdapterMock final : public trompeloeil::mock_interface<IDXGIDxvkAdapter> {
    MAKE_MOCK2(QueryInterface, HRESULT(REFIID, void**), override);
    MAKE_MOCK0(AddRef, ULONG(), override);
    MAKE_MOCK0(Release, ULONG(), override);
    IMPLEMENT_MOCK3(GetPrivateData);
    IMPLEMENT_MOCK3(SetPrivateData);
    IMPLEMENT_MOCK2(SetPrivateDataInterface);
    IMPLEMENT_MOCK2(GetParent);
    IMPLEMENT_MOCK2(EnumOutputs);
    IMPLEMENT_MOCK1(GetDesc);
    IMPLEMENT_MOCK2(CheckInterfaceSupport);
    IMPLEMENT_MOCK1(GetDesc1);
    IMPLEMENT_MOCK2(GetVulkanHandles);
    IMPLEMENT_MOCK1(GetDesc2);
    IMPLEMENT_MOCK2(RegisterHardwareContentProtectionTeardownStatusEvent);
    IMPLEMENT_MOCK1(UnregisterHardwareContentProtectionTeardownStatus);
    IMPLEMENT_MOCK3(QueryVideoMemoryInfo);
    IMPLEMENT_MOCK3(SetVideoMemoryReservation);
    IMPLEMENT_MOCK2(RegisterVideoMemoryBudgetChangeNotificationEvent);
    IMPLEMENT_MOCK1(UnregisterVideoMemoryBudgetChangeNotification);
};

class DXGIOutput6Mock final : public trompeloeil::mock_interface<IDXGIOutput6> {
    MAKE_MOCK2(QueryInterface, HRESULT(REFIID, void**), override);
    MAKE_MOCK0(AddRef, ULONG(), override);
    MAKE_MOCK0(Release, ULONG(), override);
    IMPLEMENT_MOCK3(GetPrivateData);
    IMPLEMENT_MOCK3(SetPrivateData);
    IMPLEMENT_MOCK2(SetPrivateDataInterface);
    IMPLEMENT_MOCK2(GetParent);
    IMPLEMENT_MOCK1(GetDesc);
    IMPLEMENT_MOCK4(GetDisplayModeList);
    IMPLEMENT_MOCK3(FindClosestMatchingMode);
    IMPLEMENT_MOCK0(WaitForVBlank);
    IMPLEMENT_MOCK2(TakeOwnership);
    IMPLEMENT_MOCK0(ReleaseOwnership);
    IMPLEMENT_MOCK1(GetGammaControlCapabilities);
    IMPLEMENT_MOCK1(SetGammaControl);
    IMPLEMENT_MOCK1(GetGammaControl);
    IMPLEMENT_MOCK1(SetDisplaySurface);
    IMPLEMENT_MOCK1(GetDisplaySurfaceData);
    IMPLEMENT_MOCK1(GetFrameStatistics);
    IMPLEMENT_MOCK4(GetDisplayModeList1);
    IMPLEMENT_MOCK3(FindClosestMatchingMode1);
    IMPLEMENT_MOCK1(GetDisplaySurfaceData1);
    IMPLEMENT_MOCK2(DuplicateOutput);
    IMPLEMENT_MOCK0(SupportsOverlays);
    IMPLEMENT_MOCK3(CheckOverlaySupport);
    IMPLEMENT_MOCK4(CheckOverlayColorSpaceSupport);
    IMPLEMENT_MOCK5(DuplicateOutput1);
    IMPLEMENT_MOCK1(GetDesc1);
    IMPLEMENT_MOCK1(CheckHardwareCompositionSupport);
};

class VkMock final : public trompeloeil::mock_interface<dxvk::Vk> {
    IMPLEMENT_CONST_MOCK0(IsAvailable);
    IMPLEMENT_CONST_MOCK2(GetDeviceExtensions);
    IMPLEMENT_CONST_MOCK3(GetPhysicalDeviceProperties2);
};

class NvmlMock final : public trompeloeil::mock_interface<dxvk::Nvml> {
    IMPLEMENT_CONST_MOCK0(IsAvailable);
    IMPLEMENT_CONST_MOCK1(ErrorString);
    IMPLEMENT_CONST_MOCK2(DeviceGetHandleByPciBusId_v2);
    IMPLEMENT_CONST_MOCK2(DeviceGetMemoryInfo_v2);
    IMPLEMENT_CONST_MOCK2(DeviceGetPciInfo_v3);
    IMPLEMENT_CONST_MOCK3(DeviceGetClockInfo);
    IMPLEMENT_CONST_MOCK3(DeviceGetTemperature);
    IMPLEMENT_CONST_MOCK3(DeviceGetThermalSettings);
    IMPLEMENT_CONST_MOCK2(DeviceGetPerformanceState);
    IMPLEMENT_CONST_MOCK2(DeviceGetUtilizationRates);
    IMPLEMENT_CONST_MOCK3(DeviceGetVbiosVersion);
    IMPLEMENT_CONST_MOCK2(DeviceGetCurrPcieLinkWidth);
    IMPLEMENT_CONST_MOCK2(DeviceGetIrqNum);
    IMPLEMENT_CONST_MOCK2(DeviceGetNumGpuCores);
    IMPLEMENT_CONST_MOCK2(DeviceGetBusType);
    IMPLEMENT_CONST_MOCK2(DeviceGetDynamicPstatesInfo);
};
