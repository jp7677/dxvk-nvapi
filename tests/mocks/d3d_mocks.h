#pragma once

#include "../../src/nvapi_private.h"

class UnknownMock : public trompeloeil::mock_interface<IUnknown> {
    MAKE_MOCK2(QueryInterface, HRESULT(REFIID, void**), override);
    MAKE_MOCK0(AddRef, ULONG(), override);
    MAKE_MOCK0(Release, ULONG(), override);
};

class D3DLowLatencyDeviceMock : public trompeloeil::mock_interface<ID3DLowLatencyDevice> {
    MAKE_MOCK2(QueryInterface, HRESULT(REFIID, void**), override);
    MAKE_MOCK0(AddRef, ULONG(), override);
    MAKE_MOCK0(Release, ULONG(), override);
    IMPLEMENT_MOCK0(SupportsLowLatency);
    IMPLEMENT_MOCK0(LatencySleep);
    IMPLEMENT_MOCK3(SetLatencySleepMode);
    IMPLEMENT_MOCK2(SetLatencyMarker);
    IMPLEMENT_MOCK1(GetLatencyInfo);
};