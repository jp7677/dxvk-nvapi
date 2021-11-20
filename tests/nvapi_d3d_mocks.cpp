using namespace trompeloeil;

class UnknownMock : public mock_interface<IUnknown> {
    MAKE_MOCK2 (QueryInterface, HRESULT(REFIID, void**), override);
    MAKE_MOCK0 (AddRef, ULONG(), override);
    MAKE_MOCK0 (Release, ULONG(), override);
};
