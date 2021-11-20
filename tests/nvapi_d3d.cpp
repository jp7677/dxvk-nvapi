using namespace trompeloeil;

TEST_CASE("D3D methods succeed", "[.d3d]") {
    UnknownMock unknown;

    SECTION("GetObjectHandleForResource returns OK") {
        NVDX_ObjectHandle handle;
        REQUIRE(NvAPI_D3D_GetObjectHandleForResource(&unknown, &unknown, &handle) == NVAPI_OK);
    }

    SECTION("GetObjectHandleForResource returns no-implementation") {
        NvU32 value;
        REQUIRE(NvAPI_D3D_SetResourceHint(&unknown, NVDX_ObjectHandle(), NVAPI_D3D_SRH_CATEGORY_SLI, 1, &value) == NVAPI_NO_IMPLEMENTATION);
    }

    SECTION("SetResourceHint returns NoImplementation") {
        REQUIRE(NvAPI_D3D_SetResourceHint(&unknown, NVDX_ObjectHandle(), NVAPI_D3D_SRH_CATEGORY_SLI, 0, nullptr) == NVAPI_NO_IMPLEMENTATION);
    }

    SECTION("BeginResourceRendering returns OK") {
        REQUIRE(NvAPI_D3D_BeginResourceRendering(&unknown, NVDX_ObjectHandle(), 0) == NVAPI_OK);
    }

    SECTION("BeginResourceRendering returns OK") {
        REQUIRE(NvAPI_D3D_EndResourceRendering(&unknown, NVDX_ObjectHandle(), 0) == NVAPI_OK);
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

    SECTION("GetGraphicsCapabilities (V1) returns OK") {
        NV_D3D1x_GRAPHICS_CAPS_V1 caps;
        REQUIRE(NvAPI_D3D1x_GetGraphicsCapabilities(&unknown, NV_D3D1x_GRAPHICS_CAPS_VER1, reinterpret_cast<NV_D3D1x_GRAPHICS_CAPS*>(&caps)) == NVAPI_OK);
        // returned bExclusiveScissorRectsSupported/bVariablePixelRateShadingSupported values not enforced by testing
    }

    SECTION("GetGraphicsCapabilities (V2) returns OK") {
        NV_D3D1x_GRAPHICS_CAPS_V2 caps;
        REQUIRE(NvAPI_D3D1x_GetGraphicsCapabilities(&unknown, NV_D3D1x_GRAPHICS_CAPS_VER2, reinterpret_cast<NV_D3D1x_GRAPHICS_CAPS*>(&caps)) == NVAPI_OK);
        REQUIRE(caps.bFastUAVClearSupported == 1);
        // returned minorSMVersion/majorSMVersion not enforced by testing since current returns are dummy values anyway
    }

    SECTION("GetGraphicsCapabilities (FUTURE VERSION) returns IncompatibleStructVersion") {
        NV_D3D1x_GRAPHICS_CAPS caps;
        REQUIRE(NvAPI_D3D1x_GetGraphicsCapabilities(&unknown, NV_D3D1x_GRAPHICS_CAPS_VER+1, reinterpret_cast<NV_D3D1x_GRAPHICS_CAPS*>(&caps)) == NVAPI_INCOMPATIBLE_STRUCT_VERSION);
    }
}
