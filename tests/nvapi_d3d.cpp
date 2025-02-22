#include "nvapi_tests_private.h"
#include "mocks/d3d_mocks.h"
#include "mocks/d3d11_mocks.h"
#include "nvapi/resource_factory_util.h"

using namespace trompeloeil;

TEST_CASE("D3D methods succeed", "[.d3d]") {
    UnknownMock unknown;
    UnknownMock d3d9Device;
    UnknownMock d3d11Device;

    ALLOW_CALL(unknown, QueryInterface(__uuidof(IDirect3DDevice9), _))
        .RETURN(E_NOINTERFACE);
    ALLOW_CALL(unknown, QueryInterface(__uuidof(ID3D11Device), _))
        .RETURN(E_NOINTERFACE);
    ALLOW_CALL(d3d9Device, QueryInterface(__uuidof(IDirect3DDevice9), _))
        .RETURN(S_OK);
    ALLOW_CALL(d3d9Device, QueryInterface(__uuidof(ID3D11Device), _))
        .RETURN(E_NOINTERFACE);
    ALLOW_CALL(d3d11Device, QueryInterface(__uuidof(IDirect3DDevice9), _))
        .RETURN(E_NOINTERFACE);
    ALLOW_CALL(d3d11Device, QueryInterface(__uuidof(ID3D11Device), _))
        .RETURN(S_OK);

    SECTION("RegisterDevice returns OK") {
        REQUIRE(NvAPI_D3D_RegisterDevice(&unknown) == NVAPI_OK);
    }

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
            REQUIRE(NvAPI_D3D_GetCurrentSLIState(&d3d11Device, reinterpret_cast<NV_GET_CURRENT_SLI_STATE*>(&state)) == NVAPI_OK);
            REQUIRE(state.maxNumAFRGroups == 1);
            REQUIRE(state.numAFRGroups == 1);
            REQUIRE(state.currentAFRIndex == 0);
            REQUIRE(state.nextFrameAFRIndex == 0);
            REQUIRE(state.previousFrameAFRIndex == 0);
            REQUIRE(state.bIsCurAFRGroupNew == false);
        }

        SECTION("GetCurrentSLIState (V2) returns OK") {
            NV_GET_CURRENT_SLI_STATE_V2 state;
            state.version = NV_GET_CURRENT_SLI_STATE_VER2;
            REQUIRE(NvAPI_D3D_GetCurrentSLIState(&d3d11Device, &state) == NVAPI_OK);
            REQUIRE(state.maxNumAFRGroups == 1);
            REQUIRE(state.numAFRGroups == 1);
            REQUIRE(state.currentAFRIndex == 0);
            REQUIRE(state.nextFrameAFRIndex == 0);
            REQUIRE(state.previousFrameAFRIndex == 0);
            REQUIRE(state.bIsCurAFRGroupNew == false);
            REQUIRE(state.numVRSLIGpus == 0);
        }
    }

    SECTION("GetCurrentSLIState fails") {
        SECTION("GetCurrentSLIState with null pointers returns invalid-argument") {
            NV_GET_CURRENT_SLI_STATE_V1 state;
            state.version = NV_GET_CURRENT_SLI_STATE_VER1;
            REQUIRE(NvAPI_D3D_GetCurrentSLIState(&d3d9Device, nullptr) == NVAPI_INVALID_ARGUMENT);
            REQUIRE(NvAPI_D3D_GetCurrentSLIState(nullptr, reinterpret_cast<NV_GET_CURRENT_SLI_STATE*>(&state)) == NVAPI_INVALID_ARGUMENT);
        }

        SECTION("GetCurrentSLIState with objects that arent devices returns invalid-argument") {
            UnknownMock notADevice;
            ALLOW_CALL(notADevice, QueryInterface(__uuidof(IDirect3DDevice9), _))
                .RETURN(E_NOINTERFACE);
            ALLOW_CALL(notADevice, QueryInterface(__uuidof(ID3D11Device), _))
                .RETURN(E_NOINTERFACE);

            NV_GET_CURRENT_SLI_STATE_V1 state;
            state.version = NV_GET_CURRENT_SLI_STATE_VER1;
            REQUIRE(NvAPI_D3D_GetCurrentSLIState(&notADevice, reinterpret_cast<NV_GET_CURRENT_SLI_STATE*>(&state)) == NVAPI_INVALID_ARGUMENT);
        }

        SECTION("GetCurrentSLIState (V1) for D3D9 returns no-active-sli-topology") {
            NV_GET_CURRENT_SLI_STATE_V1 state;
            state.version = NV_GET_CURRENT_SLI_STATE_VER1;
            REQUIRE(NvAPI_D3D_GetCurrentSLIState(&d3d9Device, reinterpret_cast<NV_GET_CURRENT_SLI_STATE*>(&state)) == NVAPI_NO_ACTIVE_SLI_TOPOLOGY);
        }

        SECTION("GetCurrentSLIState (V2) for D3D9 returns no-active-sli-topology") {
            NV_GET_CURRENT_SLI_STATE_V2 state;
            state.numVRSLIGpus = 0xdeadbeef;
            state.version = NV_GET_CURRENT_SLI_STATE_VER2;
            REQUIRE(NvAPI_D3D_GetCurrentSLIState(&d3d9Device, &state) == NVAPI_NO_ACTIVE_SLI_TOPOLOGY);
            REQUIRE(state.numVRSLIGpus == 0);
        }

        SECTION("GetCurrentSLIState with unknown struct version returns incompatible-struct-version") {
            NV_GET_CURRENT_SLI_STATE state;
            state.version = NV_GET_CURRENT_SLI_STATE_VER2 + 1;
            REQUIRE(NvAPI_D3D_GetCurrentSLIState(&d3d11Device, &state) == NVAPI_INCOMPATIBLE_STRUCT_VERSION);
        }

        SECTION("GetCurrentSLIState with current struct version returns not incompatible-struct-version") {
            // This test fails when a header update provides a newer not yet implemented struct version
            NV_GET_CURRENT_SLI_STATE state;
            state.version = NV_GET_CURRENT_SLI_STATE_VER;
            REQUIRE(NvAPI_D3D_GetCurrentSLIState(&d3d11Device, &state) != NVAPI_INCOMPATIBLE_STRUCT_VERSION);
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

TEST_CASE("D3D Reflex depending methods succeed", "[.d3d]") {
    auto dxgiFactory = std::make_unique<DXGIDxvkFactoryMock>();
    auto vk = std::make_unique<VkMock>();
    auto nvml = std::make_unique<NvmlMock>();
    DXGIDxvkAdapterMock* adapter = CreateDXGIDxvkAdapterMock();
    DXGIOutput6Mock* output = CreateDXGIOutput6Mock();
    D3D11DxvkDeviceMock d3d11Device;
    D3D11DxvkDeviceContextMock d3d11DeviceContext;
    D3DLowLatencyDeviceMock lowLatencyDevice;
    auto d3d11DeviceRefCount = 0;
    auto d3d11DeviceContextRefCount = 0;
    auto lowLatencyDeviceRefCount = 0;

    auto e = ConfigureDefaultTestEnvironment(*dxgiFactory, *vk, *nvml, *adapter, *output);

    SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));

    SECTION("Reflex methods fail when given null device") {
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        SECTION("GetSleepStatus returns invalid-argument") {
            NV_GET_SLEEP_STATUS_PARAMS_V1 params{};
            params.version = NV_GET_SLEEP_STATUS_PARAMS_VER1;
            REQUIRE(NvAPI_D3D_GetSleepStatus(nullptr, &params) == NVAPI_INVALID_ARGUMENT);
        }

        SECTION("SetSleepMode returns invalid-argument") {
            NV_SET_SLEEP_MODE_PARAMS params{};
            params.version = NV_SET_SLEEP_MODE_PARAMS_VER;
            REQUIRE(NvAPI_D3D_SetSleepMode(nullptr, &params) == NVAPI_INVALID_ARGUMENT);
        }

        SECTION("Sleep returns invalid-argument") {
            REQUIRE(NvAPI_D3D_Sleep(nullptr) == NVAPI_INVALID_ARGUMENT);
        }

        SECTION("GetLatency returns invalid-argument") {
            NV_LATENCY_RESULT_PARAMS params;
            params.version = NV_LATENCY_RESULT_PARAMS_VER;
            REQUIRE(NvAPI_D3D_GetLatency(nullptr, &params) == NVAPI_INVALID_ARGUMENT);
        }

        SECTION("SetLatencyMarker returns invalid-argument") {
            NV_LATENCY_MARKER_PARAMS params;
            params.version = NV_LATENCY_MARKER_PARAMS_VER;
            REQUIRE(NvAPI_D3D_SetLatencyMarker(nullptr, &params) == NVAPI_INVALID_ARGUMENT);
        }
    }

    SECTION("Reflex depending methods succeed when D3DLowLatencyDevice is available") {
        ALLOW_CALL(d3d11Device, AddRef())
            .LR_SIDE_EFFECT(d3d11DeviceRefCount++)
            .RETURN(d3d11DeviceRefCount);
        ALLOW_CALL(d3d11Device, Release())
            .LR_SIDE_EFFECT(d3d11DeviceRefCount--)
            .RETURN(d3d11DeviceRefCount);
        ALLOW_CALL(d3d11Device, QueryInterface(__uuidof(ID3DLowLatencyDevice), _))
            .LR_SIDE_EFFECT(*_2 = static_cast<ID3DLowLatencyDevice*>(&lowLatencyDevice))
            .LR_SIDE_EFFECT(lowLatencyDeviceRefCount++)
            .RETURN(S_OK);

        ALLOW_CALL(d3d11DeviceContext, AddRef())
            .LR_SIDE_EFFECT(d3d11DeviceContextRefCount++)
            .RETURN(d3d11DeviceContextRefCount);
        ALLOW_CALL(d3d11DeviceContext, Release())
            .LR_SIDE_EFFECT(d3d11DeviceContextRefCount--)
            .RETURN(d3d11DeviceContextRefCount);
        ALLOW_CALL(d3d11DeviceContext, QueryInterface(__uuidof(ID3D11DeviceChild), _))
            .LR_SIDE_EFFECT(*_2 = static_cast<ID3D11DeviceContext*>(&d3d11DeviceContext))
            .LR_SIDE_EFFECT(d3d11DeviceContextRefCount++)
            .RETURN(S_OK);
        ALLOW_CALL(d3d11DeviceContext, GetDevice(_))
            .LR_SIDE_EFFECT(*_1 = static_cast<ID3D11Device*>(&d3d11Device))
            .LR_SIDE_EFFECT(d3d11DeviceRefCount++);
        ALLOW_CALL(d3d11DeviceContext, QueryInterface(__uuidof(ID3DLowLatencyDevice), _))
            .RETURN(E_NOINTERFACE);

        ALLOW_CALL(lowLatencyDevice, AddRef())
            .LR_SIDE_EFFECT(lowLatencyDeviceRefCount++)
            .RETURN(lowLatencyDeviceRefCount);
        ALLOW_CALL(lowLatencyDevice, Release())
            .LR_SIDE_EFFECT(lowLatencyDeviceRefCount--)
            .RETURN(lowLatencyDeviceRefCount);

        ALLOW_CALL(lowLatencyDevice, SupportsLowLatency())
            .RETURN(false);

        SECTION("D3DLowLatencyDevice does not support low latency") {
            REQUIRE(NvAPI_Initialize() == NVAPI_OK);

            SECTION("GetSleepStatus returns NoImplementation") {
                REQUIRE_CALL(lowLatencyDevice, SupportsLowLatency())
                    .RETURN(false);

                NV_GET_SLEEP_STATUS_PARAMS_V1 params{};
                params.version = NV_GET_SLEEP_STATUS_PARAMS_VER1;
                REQUIRE(NvAPI_D3D_GetSleepStatus(reinterpret_cast<IUnknown*>(&d3d11Device), &params) == NVAPI_NO_IMPLEMENTATION);
            }

            SECTION("SetSleepMode returns NoImplementation") {
                NV_SET_SLEEP_MODE_PARAMS params{};
                params.version = NV_SET_SLEEP_MODE_PARAMS_VER;
                REQUIRE(NvAPI_D3D_SetSleepMode(reinterpret_cast<IUnknown*>(&d3d11Device), &params) == NVAPI_NO_IMPLEMENTATION);
            }

            SECTION("Sleep returns NoImplementation") {
                REQUIRE(NvAPI_D3D_Sleep(reinterpret_cast<IUnknown*>(&d3d11Device)) == NVAPI_NO_IMPLEMENTATION);
            }

            SECTION("GetLatency returns no-implementation") {
                NV_LATENCY_RESULT_PARAMS params;
                params.version = NV_LATENCY_RESULT_PARAMS_VER;
                REQUIRE(NvAPI_D3D_GetLatency(reinterpret_cast<IUnknown*>(&d3d11Device), &params) == NVAPI_NO_IMPLEMENTATION);
            }

            SECTION("SetLatencyMarker returns no-implementation") {
                NV_LATENCY_MARKER_PARAMS params;
                params.version = NV_LATENCY_MARKER_PARAMS_VER;
                REQUIRE(NvAPI_D3D_SetLatencyMarker(reinterpret_cast<IUnknown*>(&d3d11Device), &params) == NVAPI_NO_IMPLEMENTATION);
            }
        }

        SECTION("D3DLowLatencyDevice supports low latency") {
            ALLOW_CALL(lowLatencyDevice, SupportsLowLatency())
                .RETURN(true);

            SECTION("GetSleepStatus returns OK") {
                REQUIRE_CALL(lowLatencyDevice, SupportsLowLatency())
                    .RETURN(true);

                REQUIRE(NvAPI_Initialize() == NVAPI_OK);

                NV_GET_SLEEP_STATUS_PARAMS_V1 params{};
                params.version = NV_GET_SLEEP_STATUS_PARAMS_VER1;
                REQUIRE(NvAPI_D3D_GetSleepStatus(reinterpret_cast<IUnknown*>(&d3d11Device), &params) == NVAPI_OK);
                REQUIRE(params.bLowLatencyMode == false);
            }

            SECTION("SetSleepMode calls ID3DLowLatencyDevice::SetLatencySleepMode returns OK") {
                sequence seq1, seq2;
                REQUIRE_CALL(lowLatencyDevice, SetLatencySleepMode(true, false, 250U))
                    .IN_SEQUENCE(seq1)
                    .RETURN(S_OK);
                REQUIRE_CALL(lowLatencyDevice, SetLatencySleepMode(false, false, 0U))
                    .IN_SEQUENCE(seq2)
                    .RETURN(S_OK);

                REQUIRE(NvAPI_Initialize() == NVAPI_OK);

                NV_SET_SLEEP_MODE_PARAMS_V1 params{};
                params.version = NV_SET_SLEEP_MODE_PARAMS_VER1;
                params.bLowLatencyMode = true;
                params.minimumIntervalUs = 250;
                REQUIRE(NvAPI_D3D_SetSleepMode(reinterpret_cast<IUnknown*>(&d3d11Device), &params) == NVAPI_OK);

                NV_GET_SLEEP_STATUS_PARAMS_V1 status{};
                status.version = NV_GET_SLEEP_STATUS_PARAMS_VER1;
                REQUIRE(NvAPI_D3D_GetSleepStatus(reinterpret_cast<IUnknown*>(&d3d11Device), &status) == NVAPI_OK);
                REQUIRE(status.bLowLatencyMode == true);

                params.bLowLatencyMode = false;
                params.minimumIntervalUs = 0;
                REQUIRE(NvAPI_D3D_SetSleepMode(reinterpret_cast<IUnknown*>(&d3d11Device), &params) == NVAPI_OK);

                REQUIRE(NvAPI_D3D_GetSleepStatus(reinterpret_cast<IUnknown*>(&d3d11Device), &status) == NVAPI_OK);
                REQUIRE(status.bLowLatencyMode == false);
            }

            SECTION("Sleep calls ID3DLowLatencyDevice::LatencySleep and returns OK") {
                REQUIRE_CALL(lowLatencyDevice, SetLatencySleepMode(true, false, 500U))
                    .RETURN(S_OK);
                REQUIRE_CALL(lowLatencyDevice, LatencySleep())
                    .RETURN(S_OK);

                REQUIRE(NvAPI_Initialize() == NVAPI_OK);

                NV_SET_SLEEP_MODE_PARAMS sleepModeParams{};
                sleepModeParams.version = NV_SET_SLEEP_MODE_PARAMS_VER;
                sleepModeParams.bLowLatencyMode = true;
                sleepModeParams.minimumIntervalUs = 500;
                REQUIRE(NvAPI_D3D_SetSleepMode(reinterpret_cast<IUnknown*>(&d3d11Device), &sleepModeParams) == NVAPI_OK);
                REQUIRE(NvAPI_D3D_Sleep(reinterpret_cast<IUnknown*>(&d3d11Device)) == NVAPI_OK);
            }

            SECTION("SetLatencyMarker calls ID3DLowLatencyDevice::SetLatencyMarker and returns OK") {
                REQUIRE_CALL(lowLatencyDevice, SetLatencySleepMode(true, false, 750U))
                    .RETURN(S_OK);
                REQUIRE_CALL(lowLatencyDevice, LatencySleep())
                    .RETURN(S_OK);
                REQUIRE_CALL(lowLatencyDevice, SetLatencyMarker(1ULL, VK_LATENCY_MARKER_SIMULATION_START_NV))
                    .RETURN(S_OK);

                REQUIRE(NvAPI_Initialize() == NVAPI_OK);

                NV_SET_SLEEP_MODE_PARAMS sleepModeParams{};
                sleepModeParams.version = NV_SET_SLEEP_MODE_PARAMS_VER;
                sleepModeParams.bLowLatencyMode = true;
                sleepModeParams.minimumIntervalUs = 750;
                NV_LATENCY_MARKER_PARAMS latencyMarkerParams{};
                latencyMarkerParams.version = NV_LATENCY_MARKER_PARAMS_VER1;
                latencyMarkerParams.frameID = 123ULL;
                latencyMarkerParams.markerType = SIMULATION_START;
                REQUIRE(NvAPI_D3D_SetSleepMode(reinterpret_cast<IUnknown*>(&d3d11Device), &sleepModeParams) == NVAPI_OK);
                REQUIRE(NvAPI_D3D_Sleep(reinterpret_cast<IUnknown*>(&d3d11Device)) == NVAPI_OK);
                REQUIRE(NvAPI_D3D_SetLatencyMarker(reinterpret_cast<IUnknown*>(&d3d11Device), &latencyMarkerParams) == NVAPI_OK);
            }

            SECTION("Sleep  successfully handles being passed ID3D11DeviceContext as IUnknown") {
                REQUIRE_CALL(lowLatencyDevice, SetLatencySleepMode(true, false, 500U))
                    .RETURN(S_OK);
                REQUIRE_CALL(lowLatencyDevice, LatencySleep())
                    .RETURN(S_OK);

                REQUIRE(NvAPI_Initialize() == NVAPI_OK);

                NV_SET_SLEEP_MODE_PARAMS sleepModeParams{};
                sleepModeParams.version = NV_SET_SLEEP_MODE_PARAMS_VER;
                sleepModeParams.bLowLatencyMode = true;
                sleepModeParams.minimumIntervalUs = 500;
                REQUIRE(NvAPI_D3D_SetSleepMode(reinterpret_cast<IUnknown*>(&d3d11DeviceContext), &sleepModeParams) == NVAPI_OK);
                REQUIRE(NvAPI_D3D_Sleep(reinterpret_cast<IUnknown*>(&d3d11DeviceContext)) == NVAPI_OK);
            }

            SECTION("SetLatencyMarker successfully handles being passed ID3D11DeviceContext as IUnknown") {
                REQUIRE_CALL(lowLatencyDevice, SetLatencyMarker(1ULL, VK_LATENCY_MARKER_OUT_OF_BAND_RENDERSUBMIT_START_NV))
                    .RETURN(S_OK);

                REQUIRE(NvAPI_Initialize() == NVAPI_OK);

                NV_LATENCY_MARKER_PARAMS params{};
                params.version = NV_LATENCY_MARKER_PARAMS_VER1;
                params.frameID = 123ULL;
                params.markerType = OUT_OF_BAND_RENDERSUBMIT_START;
                REQUIRE(NvAPI_D3D_SetLatencyMarker(reinterpret_cast<IUnknown*>(&d3d11DeviceContext), &params) == NVAPI_OK);
            }

            SECTION("SetLatencyMarker drops PC_LATENCY_PING and returns OK") {
                FORBID_CALL(lowLatencyDevice, SetLatencyMarker(_, _));

                REQUIRE(NvAPI_Initialize() == NVAPI_OK);

                NV_LATENCY_MARKER_PARAMS latencyMarkerParams{};
                latencyMarkerParams.version = NV_LATENCY_MARKER_PARAMS_VER1;
                latencyMarkerParams.frameID = 123ULL;
                latencyMarkerParams.markerType = PC_LATENCY_PING;
                REQUIRE(NvAPI_D3D_SetLatencyMarker(reinterpret_cast<IUnknown*>(&d3d11Device), &latencyMarkerParams) == NVAPI_OK);
            }

            SECTION("SetLatencyMarker drops repeated frame IDs and returns OK") {
                sequence seq1, seq2, seq3;

                REQUIRE_CALL(lowLatencyDevice, SetLatencyMarker(1ULL, VK_LATENCY_MARKER_PRESENT_START_NV))
                    .IN_SEQUENCE(seq1)
                    .TIMES(1)
                    .RETURN(S_OK);
                REQUIRE_CALL(lowLatencyDevice, SetLatencyMarker(2ULL, VK_LATENCY_MARKER_PRESENT_START_NV))
                    .IN_SEQUENCE(seq2)
                    .TIMES(1)
                    .RETURN(S_OK);
                REQUIRE_CALL(lowLatencyDevice, SetLatencyMarker(2ULL, VK_LATENCY_MARKER_PRESENT_END_NV))
                    .IN_SEQUENCE(seq3)
                    .TIMES(1)
                    .RETURN(S_OK);

                REQUIRE(NvAPI_Initialize() == NVAPI_OK);

                NV_LATENCY_MARKER_PARAMS latencyMarkerParams{};
                latencyMarkerParams.version = NV_LATENCY_MARKER_PARAMS_VER1;
                latencyMarkerParams.frameID = 1;
                latencyMarkerParams.markerType = PRESENT_START;
                REQUIRE(NvAPI_D3D_SetLatencyMarker(reinterpret_cast<IUnknown*>(&d3d11Device), &latencyMarkerParams) == NVAPI_OK);
                REQUIRE(NvAPI_D3D_SetLatencyMarker(reinterpret_cast<IUnknown*>(&d3d11Device), &latencyMarkerParams) == NVAPI_OK);
                latencyMarkerParams.frameID = 2;
                REQUIRE(NvAPI_D3D_SetLatencyMarker(reinterpret_cast<IUnknown*>(&d3d11Device), &latencyMarkerParams) == NVAPI_OK);
                REQUIRE(NvAPI_D3D_SetLatencyMarker(reinterpret_cast<IUnknown*>(&d3d11Device), &latencyMarkerParams) == NVAPI_OK);
                latencyMarkerParams.frameID = 2;
                latencyMarkerParams.markerType = PRESENT_END;
                REQUIRE(NvAPI_D3D_SetLatencyMarker(reinterpret_cast<IUnknown*>(&d3d11Device), &latencyMarkerParams) == NVAPI_OK);
                REQUIRE(NvAPI_D3D_SetLatencyMarker(reinterpret_cast<IUnknown*>(&d3d11Device), &latencyMarkerParams) == NVAPI_OK);
            }

            SECTION("SetLatencyMarker correctly produces monotonic frame ids for a sequence of unique application frame ids") {
                REQUIRE_CALL(lowLatencyDevice, SetLatencySleepMode(true, false, 750U))
                    .RETURN(S_OK);

                REQUIRE(NvAPI_Initialize() == NVAPI_OK);

                NV_SET_SLEEP_MODE_PARAMS sleepModeParams{};
                sleepModeParams.version = NV_SET_SLEEP_MODE_PARAMS_VER;
                sleepModeParams.bLowLatencyMode = true;
                sleepModeParams.minimumIntervalUs = 750;
                NV_LATENCY_MARKER_PARAMS latencyMarkerParams{};
                latencyMarkerParams.version = NV_LATENCY_MARKER_PARAMS_VER1;
                latencyMarkerParams.frameID = 256ULL;
                latencyMarkerParams.markerType = SIMULATION_START;
                REQUIRE(NvAPI_D3D_SetSleepMode(reinterpret_cast<IUnknown*>(&d3d11Device), &sleepModeParams) == NVAPI_OK);

                for (uint64_t i = 0; i < 1100; i++) {
                    uint64_t lowLatencyDeviceFrameId = i + 1;

                    {
                        latencyMarkerParams.markerType = SIMULATION_START;
                        REQUIRE_CALL(lowLatencyDevice, SetLatencyMarker(lowLatencyDeviceFrameId, SIMULATION_START))
                            .RETURN(S_OK);
                        REQUIRE(NvAPI_D3D_SetLatencyMarker(reinterpret_cast<IUnknown*>(&d3d11Device), &latencyMarkerParams) == NVAPI_OK);
                    }

                    {
                        latencyMarkerParams.markerType = SIMULATION_END;
                        REQUIRE_CALL(lowLatencyDevice, SetLatencyMarker(lowLatencyDeviceFrameId, SIMULATION_END))
                            .RETURN(S_OK);
                        REQUIRE(NvAPI_D3D_SetLatencyMarker(reinterpret_cast<IUnknown*>(&d3d11Device), &latencyMarkerParams) == NVAPI_OK);
                    }

                    if (i % 2)
                        latencyMarkerParams.frameID += 512ULL;
                    else
                        latencyMarkerParams.frameID -= 128ULL;
                }
            }

            SECTION("GetLatencyInfo correctly returns application frame ids rather than device frame ids") {
                std::unique_ptr<NV_LATENCY_RESULT_PARAMS> latencyResults = std::make_unique<NV_LATENCY_RESULT_PARAMS>();

                REQUIRE_CALL(lowLatencyDevice, SetLatencySleepMode(true, false, 750U))
                    .RETURN(S_OK);

                REQUIRE(NvAPI_Initialize() == NVAPI_OK);

                NV_SET_SLEEP_MODE_PARAMS sleepModeParams{};
                sleepModeParams.version = NV_SET_SLEEP_MODE_PARAMS_VER;
                sleepModeParams.bLowLatencyMode = true;
                sleepModeParams.minimumIntervalUs = 750;
                NV_LATENCY_MARKER_PARAMS latencyMarkerParams{};
                latencyMarkerParams.version = NV_LATENCY_MARKER_PARAMS_VER1;
                latencyMarkerParams.frameID = 0ULL;
                latencyMarkerParams.markerType = SIMULATION_START;
                REQUIRE(NvAPI_D3D_SetSleepMode(reinterpret_cast<IUnknown*>(&d3d11Device), &sleepModeParams) == NVAPI_OK);

                for (uint64_t i = 0; i < 1100; i++) {
                    uint64_t lowLatencyDeviceFrameId = i + 1;

                    {
                        latencyMarkerParams.markerType = SIMULATION_START;
                        REQUIRE_CALL(lowLatencyDevice, SetLatencyMarker(lowLatencyDeviceFrameId, SIMULATION_START))
                            .RETURN(S_OK);
                        REQUIRE(NvAPI_D3D_SetLatencyMarker(reinterpret_cast<IUnknown*>(&d3d11Device), &latencyMarkerParams) == NVAPI_OK);
                    }

                    {
                        latencyMarkerParams.markerType = SIMULATION_END;
                        REQUIRE_CALL(lowLatencyDevice, SetLatencyMarker(lowLatencyDeviceFrameId, SIMULATION_END))
                            .RETURN(S_OK);
                        REQUIRE(NvAPI_D3D_SetLatencyMarker(reinterpret_cast<IUnknown*>(&d3d11Device), &latencyMarkerParams) == NVAPI_OK);
                    }

                    {
                        memset(latencyResults.get(), 0, sizeof(NV_LATENCY_RESULT_PARAMS));
                        latencyResults->version = NV_LATENCY_RESULT_PARAMS_VER1;

                        REQUIRE_CALL(lowLatencyDevice, GetLatencyInfo(reinterpret_cast<D3D_LATENCY_RESULTS*>(latencyResults.get())))
                            .SIDE_EFFECT(std::ranges::for_each(_1->frame_reports, [&](auto& report) {
                                report.frameID = lowLatencyDeviceFrameId;
                            }))
                            .RETURN(S_OK);

                        REQUIRE(NvAPI_D3D_GetLatency(reinterpret_cast<IUnknown*>(&d3d11Device), latencyResults.get()) == NVAPI_OK);
                        REQUIRE(std::ranges::all_of(latencyResults->frameReport, [&](auto& report) {
                            return (report.frameID == latencyMarkerParams.frameID);
                        }));
                    }

                    latencyMarkerParams.frameID += 10ULL;
                }
            }

            SECTION("SetLatencyMarker with unknown struct version returns incompatible-struct-version") {
                REQUIRE(NvAPI_Initialize() == NVAPI_OK);

                NV_LATENCY_MARKER_PARAMS params{};
                params.version = NV_LATENCY_MARKER_PARAMS_VER1 + 1;
                REQUIRE(NvAPI_D3D_SetLatencyMarker(reinterpret_cast<IUnknown*>(&d3d11Device), &params) == NVAPI_INCOMPATIBLE_STRUCT_VERSION);
            }

            SECTION("SetLatencyMarker with current struct version returns not incompatible-struct-version") {
                ALLOW_CALL(lowLatencyDevice, SetLatencyMarker(_, _))
                    .RETURN(S_OK);

                REQUIRE(NvAPI_Initialize() == NVAPI_OK);

                NV_LATENCY_MARKER_PARAMS params{};
                params.version = NV_LATENCY_MARKER_PARAMS_VER;
                REQUIRE(NvAPI_D3D_SetLatencyMarker(reinterpret_cast<IUnknown*>(&d3d11Device), &params) != NVAPI_INCOMPATIBLE_STRUCT_VERSION);
            }

            SECTION("GetLatency calls ID3DLowLatencyDevice::GetLatencyInfo and returns OK") {
                REQUIRE_CALL(lowLatencyDevice, SetLatencySleepMode(true, false, 1000U))
                    .RETURN(S_OK);
                REQUIRE_CALL(lowLatencyDevice, GetLatencyInfo(_))
                    .RETURN(S_OK);

                REQUIRE(NvAPI_Initialize() == NVAPI_OK);

                NV_SET_SLEEP_MODE_PARAMS sleepModeParams{};
                sleepModeParams.version = NV_SET_SLEEP_MODE_PARAMS_VER;
                sleepModeParams.bLowLatencyMode = true;
                sleepModeParams.minimumIntervalUs = 1000;
                NV_LATENCY_RESULT_PARAMS latencyResults{};
                latencyResults.version = NV_LATENCY_RESULT_PARAMS_VER1;
                REQUIRE(NvAPI_D3D_SetSleepMode(reinterpret_cast<IUnknown*>(&d3d11Device), &sleepModeParams) == NVAPI_OK);
                REQUIRE(NvAPI_D3D_GetLatency(reinterpret_cast<IUnknown*>(&d3d11Device), &latencyResults) == NVAPI_OK);
            }
        }
    }

    CHECK(d3d11DeviceRefCount == 0);
    CHECK(lowLatencyDeviceRefCount == 0);
    CHECK(d3d11DeviceContextRefCount == 0);
}
