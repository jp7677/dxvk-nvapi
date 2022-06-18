#pragma once

#include "nvapi_tests_private.h"
#include "../src/dxvk/dxvk_interfaces.h"
#include "../src/sysinfo/vulkan.h"
#include "../src/sysinfo/nvml.h"

class DXGIFactory1Mock : public trompeloeil::mock_interface<IDXGIFactory1> {
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
};

class IDXGIDxvkAdapter : public IDXGIAdapter1, public IDXGIVkInteropAdapter {};

class DXGIDxvkAdapterMock : public trompeloeil::mock_interface<IDXGIDxvkAdapter> {
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
};

class DXGIOutputMock : public trompeloeil::mock_interface<IDXGIOutput> {
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
};

class VulkanMock : public trompeloeil::mock_interface<dxvk::Vulkan> {
    IMPLEMENT_CONST_MOCK0(IsAvailable);
    IMPLEMENT_CONST_MOCK2(GetDeviceExtensions);
    IMPLEMENT_CONST_MOCK3(GetPhysicalDeviceProperties2);
    IMPLEMENT_CONST_MOCK3(GetPhysicalDeviceMemoryProperties2);
};

class NvmlMock : public trompeloeil::mock_interface<dxvk::Nvml> {
    IMPLEMENT_CONST_MOCK0(IsAvailable);
    IMPLEMENT_CONST_MOCK1(ErrorString);
    IMPLEMENT_CONST_MOCK2(DeviceGetHandleByPciBusId_v2);
    IMPLEMENT_CONST_MOCK2(DeviceGetPciInfo_v3);
    IMPLEMENT_CONST_MOCK3(DeviceGetClockInfo);
    IMPLEMENT_CONST_MOCK3(DeviceGetTemperature);
    IMPLEMENT_CONST_MOCK3(DeviceGetThermalSettings);
    IMPLEMENT_CONST_MOCK2(DeviceGetPerformanceState);
    IMPLEMENT_CONST_MOCK2(DeviceGetUtilizationRates);
    IMPLEMENT_CONST_MOCK3(DeviceGetVbiosVersion);
    IMPLEMENT_CONST_MOCK2(DeviceGetBusType);
    IMPLEMENT_CONST_MOCK2(DeviceGetDynamicPstatesInfo);
};
