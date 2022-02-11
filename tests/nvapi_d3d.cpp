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

    SECTION("SetResourceHint returns no-implementation") {
        REQUIRE(NvAPI_D3D_SetResourceHint(&unknown, NVDX_ObjectHandle(), NVAPI_D3D_SRH_CATEGORY_SLI, 0, nullptr) == NVAPI_NO_IMPLEMENTATION);
    }

    SECTION("BeginResourceRendering returns OK") {
        REQUIRE(NvAPI_D3D_BeginResourceRendering(&unknown, NVDX_ObjectHandle(), 0) == NVAPI_OK);
    }

    SECTION("EndResourceRendering returns OK") {
        REQUIRE(NvAPI_D3D_EndResourceRendering(&unknown, NVDX_ObjectHandle(), 0) == NVAPI_OK);
    }

    SECTION("GetCurrentSLIState succeeds") {
        SECTION("GetCurrentSLIState (V1) returns OK") {
            NV_GET_CURRENT_SLI_STATE_V1 state;
            state.version = NV_GET_CURRENT_SLI_STATE_VER1;
            REQUIRE(NvAPI_D3D_GetCurrentSLIState(&unknown, reinterpret_cast<NV_GET_CURRENT_SLI_STATE*>(&state)) ==
                    NVAPI_OK);
            REQUIRE(state.maxNumAFRGroups == 1);
            REQUIRE(state.numAFRGroups == 1);
            REQUIRE(state.currentAFRIndex == 0);
            REQUIRE(state.nextFrameAFRIndex == 0);
            REQUIRE(state.previousFrameAFRIndex == 0);
            REQUIRE(state.bIsCurAFRGroupNew == false);
        }

        SECTION("GetCurrentSLIState (V2) returns OK") {
            NV_GET_CURRENT_SLI_STATE_V2 state;
            state.version = NV_GET_CURRENT_SLI_STATE_VER1;
            REQUIRE(NvAPI_D3D_GetCurrentSLIState(&unknown, &state) == NVAPI_OK);
            REQUIRE(state.maxNumAFRGroups == 1);
            REQUIRE(state.numAFRGroups == 1);
            REQUIRE(state.currentAFRIndex == 0);
            REQUIRE(state.nextFrameAFRIndex == 0);
            REQUIRE(state.previousFrameAFRIndex == 0);
            REQUIRE(state.bIsCurAFRGroupNew == false);
            REQUIRE(state.numVRSLIGpus == 0);
        }

        SECTION("GetCurrentSLIState with unknown struct version returns incompatible-struct-version") {
            NV_GET_CURRENT_SLI_STATE state;
            state.version = NV_GET_CURRENT_SLI_STATE_VER2 + 1;
            REQUIRE(NvAPI_D3D_GetCurrentSLIState(&unknown, &state) == NVAPI_INCOMPATIBLE_STRUCT_VERSION);
        }

        SECTION("GetCurrentSLIState with current struct version returns not incompatible-struct-version") {
            // This test fails when a header update provides a newer not yet implemented struct version
            NV_GET_CURRENT_SLI_STATE state;
            state.version = NV_GET_CURRENT_SLI_STATE_VER;
            REQUIRE(NvAPI_D3D_GetCurrentSLIState(&unknown, &state) != NVAPI_INCOMPATIBLE_STRUCT_VERSION);
        }
    }

    SECTION("ImplicitSLIControl succeeds") {
        SECTION("ImplicitSLIControl returns OK") {
            REQUIRE(NvAPI_D3D_ImplicitSLIControl(DISABLE_IMPLICIT_SLI) == NVAPI_OK);
        }

        SECTION("ImplicitSLIControl returns error when enabling SLI") {
            REQUIRE(NvAPI_D3D_ImplicitSLIControl(ENABLE_IMPLICIT_SLI) == NVAPI_ERROR);
        }
    }

    SECTION("GetGraphicsCapabilities succeeds") {
        SECTION("GetGraphicsCapabilities (V1) returns OK") {
            NV_D3D1x_GRAPHICS_CAPS_V1 caps;
            REQUIRE(NvAPI_D3D1x_GetGraphicsCapabilities(&unknown, NV_D3D1x_GRAPHICS_CAPS_VER1, reinterpret_cast<NV_D3D1x_GRAPHICS_CAPS*>(&caps)) == NVAPI_OK);
            REQUIRE(caps.bExclusiveScissorRectsSupported == 0);
            REQUIRE(caps.bVariablePixelRateShadingSupported == 0);
        }

        SECTION("GetGraphicsCapabilities (V2) returns OK") {
            NV_D3D1x_GRAPHICS_CAPS_V2 caps;
            REQUIRE(NvAPI_D3D1x_GetGraphicsCapabilities(&unknown, NV_D3D1x_GRAPHICS_CAPS_VER2, &caps) == NVAPI_OK);
            REQUIRE(caps.bFastUAVClearSupported == 1);
            REQUIRE(caps.majorSMVersion == 0);
            REQUIRE(caps.minorSMVersion == 0);
            REQUIRE(caps.bExclusiveScissorRectsSupported == 0);
            REQUIRE(caps.bVariablePixelRateShadingSupported == 0);
        }

        SECTION("GetGraphicsCapabilities with unknown struct version returns incompatible-struct-version") {
            NV_D3D1x_GRAPHICS_CAPS caps;
            REQUIRE(NvAPI_D3D1x_GetGraphicsCapabilities(&unknown, NV_D3D1x_GRAPHICS_CAPS_VER2 + 1, &caps) == NVAPI_INCOMPATIBLE_STRUCT_VERSION);
        }

        SECTION("GetGraphicsCapabilities with current struct version returns not incompatible-struct-version") {
            // This test should fail when a header update provides a newer not yet implemented struct version
            NV_D3D1x_GRAPHICS_CAPS caps;
            REQUIRE(NvAPI_D3D1x_GetGraphicsCapabilities(&unknown, NV_D3D1x_GRAPHICS_CAPS_VER, &caps) != NVAPI_INCOMPATIBLE_STRUCT_VERSION);
        }
    }
}

TEST_CASE("D3D Reflex/LatencyFleX depending methods succeed", "[.d3d]"){
    UnknownMock unknown;
    auto dxgiFactory = std::make_unique<DXGIFactory1Mock>();
    auto vulkan = std::make_unique<VulkanMock>();
    auto nvml = std::make_unique<NvmlMock>();
    auto lfx = std::make_unique<LfxMock>();
    DXGIDxvkAdapterMock adapter;
    DXGIOutputMock output;

    auto e = ConfigureDefaultTestEnvironment(*dxgiFactory, *vulkan, *nvml, *lfx, adapter, output);

    ALLOW_CALL(*lfx, IsAvailable())
        .RETURN(false);

    SECTION("Reflex methods succeed when LFX is available") {
        ALLOW_CALL(*lfx, IsAvailable())
            .RETURN(true); // NOLINT(bugprone-use-after-move)

        SECTION("GetSleepStatus returns OK") {
            SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml), std::move(lfx));
            REQUIRE(NvAPI_Initialize() == NVAPI_OK);

            NV_GET_SLEEP_STATUS_PARAMS_V1 params{};
            params.version = NV_GET_SLEEP_STATUS_PARAMS_VER1;
            REQUIRE(NvAPI_D3D_GetSleepStatus(&unknown, &params) == NVAPI_OK);
        }

        SECTION("SetSleepMode returns OK") {
            REQUIRE_CALL(*lfx, SetTargetFrameTime(250ULL * 1000)); // NOLINT(bugprone-use-after-move)

            SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml), std::move(lfx));
            REQUIRE(NvAPI_Initialize() == NVAPI_OK);

            NV_SET_SLEEP_MODE_PARAMS_V1 params{};
            params.version = NV_SET_SLEEP_MODE_PARAMS_VER1;
            params.bLowLatencyMode = true;
            params.minimumIntervalUs = 250;
            REQUIRE(NvAPI_D3D_SetSleepMode(&unknown, &params) == NVAPI_OK);
        }

        SECTION("Sleep calls LFX throttle callback and returns OK") {
            REQUIRE_CALL(*lfx, SetTargetFrameTime(500ULL * 1000)); // NOLINT(bugprone-use-after-move)
            REQUIRE_CALL(*lfx, WaitAndBeginFrame());

            SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml), std::move(lfx));
            REQUIRE(NvAPI_Initialize() == NVAPI_OK);

            NV_SET_SLEEP_MODE_PARAMS params{};
            params.version = NV_SET_SLEEP_MODE_PARAMS_VER;
            params.bLowLatencyMode = true;
            params.minimumIntervalUs = 500;
            REQUIRE(NvAPI_D3D_SetSleepMode(&unknown, &params) == NVAPI_OK);
            REQUIRE(NvAPI_D3D_Sleep(&unknown) == NVAPI_OK);
        }

        SECTION("GetSleepStatus with unknown struct version returns incompatible-struct-version") {
            NV_GET_SLEEP_STATUS_PARAMS params{};
            params.version = NV_GET_SLEEP_STATUS_PARAMS_VER1 + 1;
            REQUIRE(NvAPI_D3D_GetSleepStatus(&unknown, &params) == NVAPI_INCOMPATIBLE_STRUCT_VERSION);
        }

        SECTION("GetSleepStatus with current struct version returns not incompatible-struct-version") {
            // This test should fail when a header update provides a newer not yet implemented struct version
            SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml), std::move(lfx));
            REQUIRE(NvAPI_Initialize() == NVAPI_OK);

            NV_GET_SLEEP_STATUS_PARAMS params{};
            params.version = NV_GET_SLEEP_STATUS_PARAMS_VER;
            REQUIRE(NvAPI_D3D_GetSleepStatus(&unknown, &params) != NVAPI_INCOMPATIBLE_STRUCT_VERSION);
        }

        SECTION("SetSleepMode with unknown struct version returns incompatible-struct-version") {
            NV_SET_SLEEP_MODE_PARAMS params{};
            params.version = NV_SET_SLEEP_MODE_PARAMS_VER1 + 1;
            REQUIRE(NvAPI_D3D_SetSleepMode(&unknown, &params) == NVAPI_INCOMPATIBLE_STRUCT_VERSION);
        }

        SECTION("SetSleepMode with current struct version returns not incompatible-struct-version") {
            // This test should fail when a header update provides a newer not yet implemented struct version
            SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml), std::move(lfx));
            REQUIRE(NvAPI_Initialize() == NVAPI_OK);

            NV_SET_SLEEP_MODE_PARAMS params{};
            params.version = NV_SET_SLEEP_MODE_PARAMS_VER;
            REQUIRE(NvAPI_D3D_SetSleepMode(&unknown, &params) != NVAPI_INCOMPATIBLE_STRUCT_VERSION);
        }
    }

    SECTION("LatencyFleX depending methods succeed when LFX is unavailable") {
        SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml), std::move(lfx));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        SECTION("GetSleepStatus returns NoImplementation") {
            NV_GET_SLEEP_STATUS_PARAMS params{};
            params.version = NV_GET_SLEEP_STATUS_PARAMS_VER;
            REQUIRE(NvAPI_D3D_GetSleepStatus(&unknown, &params) == NVAPI_NO_IMPLEMENTATION);
        }

        SECTION("SetSleepMode returns NoImplementation") {
            NV_SET_SLEEP_MODE_PARAMS params{};
            params.version = NV_SET_SLEEP_MODE_PARAMS_VER;
            REQUIRE(NvAPI_D3D_SetSleepMode(&unknown, &params) == NVAPI_NO_IMPLEMENTATION);
        }

        SECTION("Sleep returns NoImplementation") {
            REQUIRE(NvAPI_D3D_Sleep(&unknown) == NVAPI_NO_IMPLEMENTATION);
        }
    }
}
