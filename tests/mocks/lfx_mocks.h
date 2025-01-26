#pragma once

#include "../../src/nvapi_private.h"
#include "../../src/nvapi/lfx.h"

class LfxMock : public trompeloeil::mock_interface<dxvk::Lfx> {
    IMPLEMENT_CONST_MOCK0(IsAvailable);
    IMPLEMENT_MOCK0(WaitAndBeginFrame);
    IMPLEMENT_MOCK1(SetTargetFrameTime);
};
