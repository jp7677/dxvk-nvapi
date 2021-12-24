using namespace trompeloeil;

class UnknownMock : public mock_interface<IUnknown> {
    MAKE_MOCK2 (QueryInterface, HRESULT(REFIID, void**), override);
    MAKE_MOCK0 (AddRef, ULONG(), override);
    MAKE_MOCK0 (Release, ULONG(), override);
};

class LfxMock : public mock_interface<Lfx> {
    IMPLEMENT_CONST_MOCK0 (IsAvailable);
    IMPLEMENT_MOCK0 (WaitAndBeginFrame);
    IMPLEMENT_MOCK1 (SetTargetFrameTime);
};