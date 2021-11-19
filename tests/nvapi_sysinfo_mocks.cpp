#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"

using namespace trompeloeil;

class DXGIFactory1Mock : public mock_interface<IDXGIFactory1> {
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

class DXGIDxvkAdapterMock : public mock_interface<IDXGIDxvkAdapter> {
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
    IMPLEMENT_MOCK2(GetVulkanHandles);
    IMPLEMENT_MOCK1(GetDesc1);
};

class DXGIOutputMock : public mock_interface<IDXGIOutput> {
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

class VulkanMock : public mock_interface<Vulkan> {
    IMPLEMENT_CONST_MOCK0(IsAvailable);
    IMPLEMENT_CONST_MOCK2(GetDeviceExtensions);
    IMPLEMENT_CONST_MOCK3(GetPhysicalDeviceProperties2);
    IMPLEMENT_CONST_MOCK3(GetPhysicalDeviceMemoryProperties2);
};

class NvmlMock : public mock_interface<Nvml> {
    IMPLEMENT_CONST_MOCK0(IsAvailable);
    IMPLEMENT_CONST_MOCK1(ErrorString);
    IMPLEMENT_CONST_MOCK2(DeviceGetHandleByPciBusId_v2);
    IMPLEMENT_CONST_MOCK2(DeviceGetPciInfo_v3);
    IMPLEMENT_CONST_MOCK3(DeviceGetTemperature);
    IMPLEMENT_CONST_MOCK2(DeviceGetUtilizationRates);
    IMPLEMENT_CONST_MOCK3(DeviceGetVbiosVersion);
    IMPLEMENT_CONST_MOCK2(DeviceGetPerformanceState);
    IMPLEMENT_CONST_MOCK3(DeviceGetClockInfo);
};
