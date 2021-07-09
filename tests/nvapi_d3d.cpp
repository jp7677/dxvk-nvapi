#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#pragma GCC diagnostic ignored "-Wunused-parameter"

#include "../inc/catch.hpp"
#include "../inc/catch2/trompeloeil.hpp"
#include "../src/nvapi_d3d.cpp"

using namespace trompeloeil;

class UnknownMock : public mock_interface<IUnknown> {
    MAKE_MOCK2 (QueryInterface, HRESULT(REFIID, void * *), override);
    MAKE_MOCK0 (AddRef, ULONG(), override);
    MAKE_MOCK0 (Release, ULONG(), override);
};

TEST_CASE("D3D methods succeed", "[.][d3d]") {
    UnknownMock unknown;

    SECTION("GetObjectHandleForResource returns no-implementation") {
        NVDX_ObjectHandle handle;
        REQUIRE(NvAPI_D3D_GetObjectHandleForResource(&unknown, &unknown, &handle) == NVAPI_NO_IMPLEMENTATION);
    }

    SECTION("GetObjectHandleForResource returns no-implementation") {
        NvU32 value;
        REQUIRE(NvAPI_D3D_SetResourceHint(&unknown, NVDX_ObjectHandle(), NVAPI_D3D_SRH_CATEGORY_SLI, 1, &value) == NVAPI_NO_IMPLEMENTATION);
    }

    SECTION("GetCurrentSLIState returns OK") {
        NV_GET_CURRENT_SLI_STATE state;
        state.version = NV_GET_CURRENT_SLI_STATE_VER2;
        REQUIRE(NvAPI_D3D_GetCurrentSLIState(&unknown, &state) == NVAPI_OK);
        REQUIRE(state.maxNumAFRGroups == 1);
        REQUIRE(state.numAFRGroups == 1);
        REQUIRE(state.currentAFRIndex == 0);
        REQUIRE(state.nextFrameAFRIndex == 0);
        REQUIRE(state.previousFrameAFRIndex == 0);
        REQUIRE(state.bIsCurAFRGroupNew == false);
        REQUIRE(state.numVRSLIGpus == 0);
    }
}
