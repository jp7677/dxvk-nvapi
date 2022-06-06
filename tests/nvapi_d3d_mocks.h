#pragma once

#include "nvapi_tests_private.h"
#include "../src/d3d/lfx.h"

using namespace trompeloeil;
using namespace dxvk;

class UnknownMock : public mock_interface<IUnknown> {
    MAKE_MOCK2(QueryInterface, HRESULT(REFIID, void**), override);
    MAKE_MOCK0(AddRef, ULONG(), override);
    MAKE_MOCK0(Release, ULONG(), override);
};

class LfxMock : public mock_interface<Lfx> {
    IMPLEMENT_CONST_MOCK0(IsAvailable);
    IMPLEMENT_MOCK0(WaitAndBeginFrame);
    IMPLEMENT_MOCK1(SetTargetFrameTime);
};
