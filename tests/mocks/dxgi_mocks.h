#pragma once

#include "../../src/nvapi_private.h"
#include "../../src/interfaces/dxvk_interfaces.h"

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
