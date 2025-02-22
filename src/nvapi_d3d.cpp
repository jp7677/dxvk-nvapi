#include "nvapi_private.h"
#include "nvapi_globals.h"
#include "nvapi/nvapi_d3d_low_latency_device.h"
#include "util/util_statuscode.h"

extern "C" {
    using namespace dxvk;

    NvAPI_Status __cdecl NvAPI_D3D_RegisterDevice(IUnknown* pDev) {
        constexpr auto n = __func__;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(pDev));

        // Just acknowledge the request since there is nothing to do here
        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_D3D_GetObjectHandleForResource(IUnknown* pDevice, IUnknown* pResource, NVDX_ObjectHandle* pHandle) {
        constexpr auto n = __func__;
        thread_local bool alreadyLogged = false;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(pDevice), log::fmt::ptr(pResource), log::fmt::ptr(pHandle));

        if (!pResource || !pHandle)
            return InvalidArgument(n);

        // Fake-implement with a dumb passthrough, though no other NvAPI entry points
        // we're likely to implement should care about the actual handle value.
        *pHandle = reinterpret_cast<NVDX_ObjectHandle>(pResource);
        return Ok(n, alreadyLogged);
    }

    NvAPI_Status __cdecl NvAPI_D3D_SetResourceHint(IUnknown* pDev, NVDX_ObjectHandle obj, NVAPI_D3D_SETRESOURCEHINT_CATEGORY dwHintCategory, NvU32 dwHintName, NvU32* pdwHintValue) {
        constexpr auto n = __func__;
        thread_local bool alreadyLogged = false;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(pDev), log::fmt::hnd(obj), dwHintCategory, dwHintName, log::fmt::ptr(pdwHintValue));

        return NoImplementation(n, alreadyLogged);
    }

    NvAPI_Status __cdecl NvAPI_D3D_BeginResourceRendering(IUnknown* pDeviceOrContext, NVDX_ObjectHandle obj, NvU32 Flags) {
        constexpr auto n = __func__;
        thread_local bool alreadyLogged = false;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(pDeviceOrContext), log::fmt::hnd(obj), log::fmt::flags(Flags));

        // Synchronisation hints for SLI...
        return Ok(n, alreadyLogged);
    }

    NvAPI_Status __cdecl NvAPI_D3D_EndResourceRendering(IUnknown* pDeviceOrContext, NVDX_ObjectHandle obj, NvU32 Flags) {
        constexpr auto n = __func__;
        thread_local bool alreadyLogged = false;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(pDeviceOrContext), log::fmt::hnd(obj), log::fmt::flags(Flags));

        return Ok(n, alreadyLogged);
    }

    NvAPI_Status __cdecl NvAPI_D3D_GetCurrentSLIState(IUnknown* pDevice, NV_GET_CURRENT_SLI_STATE* pSliState) {
        constexpr auto n = __func__;
        thread_local bool alreadyLoggedNoSli = false;
        thread_local bool alreadyLoggedOk = false;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(pDevice), log::fmt::ptr(pSliState));

        if (!pDevice || !pSliState)
            return InvalidArgument(n);

        if (pSliState->version != NV_GET_CURRENT_SLI_STATE_VER1 && pSliState->version != NV_GET_CURRENT_SLI_STATE_VER2)
            return IncompatibleStructVersion(n, pSliState->version);

        // The docs don't list NVAPI_NO_ACTIVE_SLI_TOPOLOGY as a return value,
        // but testing on Windows yielded this exact status, it only returns that for D3D9 devices though.
        Com<IDirect3DDevice9> device9;
        if (SUCCEEDED(pDevice->QueryInterface(IID_PPV_ARGS(&device9)))) {
            if (pSliState->version == NV_GET_CURRENT_SLI_STATE_VER2)
                // Tests on Windows show that this is the only field that gets initialized in this case.
                pSliState->numVRSLIGpus = 0;

            return NoActiveSliTopology(n, alreadyLoggedNoSli);
        }

        // Calling it with an object that is neither a D3D9 device, nor a D3D11 device returns INVALID_ARGUMENT.
        // Hello, Fallout New Vegas.
        Com<ID3D11Device> device11;
        if (FAILED(pDevice->QueryInterface(IID_PPV_ARGS(&device11))))
            return InvalidArgument(n);

        switch (pSliState->version) {
            case NV_GET_CURRENT_SLI_STATE_VER1: {
                auto pSliStateV1 = reinterpret_cast<NV_GET_CURRENT_SLI_STATE_V1*>(pSliState);
                // Report that SLI is not available
                pSliStateV1->maxNumAFRGroups = 1;
                pSliStateV1->numAFRGroups = 1;
                pSliStateV1->currentAFRIndex = 0;
                pSliStateV1->nextFrameAFRIndex = 0;
                pSliStateV1->previousFrameAFRIndex = 0;
                pSliStateV1->bIsCurAFRGroupNew = false;
                break;
            }
            case NV_GET_CURRENT_SLI_STATE_VER2:
                // Report that SLI is not available
                pSliState->maxNumAFRGroups = 1;
                pSliState->numAFRGroups = 1;
                pSliState->currentAFRIndex = 0;
                pSliState->nextFrameAFRIndex = 0;
                pSliState->previousFrameAFRIndex = 0;
                pSliState->bIsCurAFRGroupNew = false;
                pSliState->numVRSLIGpus = 0;
                break;
            default:
                break; // Handled above
        }

        return Ok(n, alreadyLoggedOk);
    }

    NvAPI_Status __cdecl NvAPI_D3D_ImplicitSLIControl(IMPLICIT_SLI_CONTROL implicitSLIControl) {
        constexpr auto n = __func__;

        if (log::tracing())
            log::trace(n, implicitSLIControl);

        if (implicitSLIControl == ENABLE_IMPLICIT_SLI)
            return Error(n); // No SLI with this implementation

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_D3D1x_GetGraphicsCapabilities(IUnknown* pDevice, NvU32 structVersion, NV_D3D1x_GRAPHICS_CAPS* pGraphicsCaps) {
        constexpr auto n = __func__;
        thread_local bool alreadyLoggedOk = false;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(pDevice), structVersion, log::fmt::ptr(pGraphicsCaps));

        if (!pGraphicsCaps)
            return InvalidArgument(n);

        switch (structVersion) {
            case NV_D3D1x_GRAPHICS_CAPS_VER1: {
                auto pGraphicsCapsV1 = reinterpret_cast<NV_D3D1x_GRAPHICS_CAPS_V1*>(pGraphicsCaps);
                *pGraphicsCapsV1 = {};
                pGraphicsCapsV1->bExclusiveScissorRectsSupported = 0;
                pGraphicsCapsV1->bVariablePixelRateShadingSupported = 0;
                break;
            }
            case NV_D3D1x_GRAPHICS_CAPS_VER2:
                *pGraphicsCaps = {};
                // bFastUAVClearSupported is reported mostly for the sake of DLSS.
                // All NVIDIA Vulkan drivers support this.
                pGraphicsCaps->bFastUAVClearSupported = 1;
                // dummy SM version number (unused by DLSS):
                pGraphicsCaps->majorSMVersion = 0;
                pGraphicsCaps->minorSMVersion = 0;
                pGraphicsCaps->bExclusiveScissorRectsSupported = 0;
                pGraphicsCaps->bVariablePixelRateShadingSupported = 0;
                break;
            default:
                return IncompatibleStructVersion(n, structVersion);
        }

        return Ok(n, alreadyLoggedOk);
    }

    NvAPI_Status __cdecl NvAPI_D3D_Sleep(IUnknown* pDevice) {
        constexpr auto n = __func__;
        thread_local bool alreadyLoggedNoReflex = false;
        thread_local bool alreadyLoggedError = false;
        thread_local bool alreadyLoggedOk = false;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(pDevice));

        if (!nvapiAdapterRegistry)
            return ApiNotInitialized(n);

        if (!pDevice)
            return InvalidArgument(n);

        auto lowLatencyDevice = NvapiD3dLowLatencyDevice::GetOrCreate(pDevice);
        if (!lowLatencyDevice || !lowLatencyDevice->SupportsLowLatency())
            return NoImplementation(n, alreadyLoggedNoReflex);

        switch (lowLatencyDevice->LatencySleep()) {
            case S_OK:
                return Ok(n, alreadyLoggedOk);
            case E_NOTIMPL:
                return NoImplementation(n, alreadyLoggedNoReflex);
            default:
                return Error(n, alreadyLoggedError);
        }
    }

    NvAPI_Status __cdecl NvAPI_D3D_SetSleepMode(IUnknown* pDevice, NV_SET_SLEEP_MODE_PARAMS* pSetSleepModeParams) {
        constexpr auto n = __func__;
        thread_local bool alreadyLoggedOk = false;
        thread_local bool alreadyLoggedNoReflex = false;
        thread_local bool alreadyLoggedError = false;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(pDevice), log::fmt::ptr(pSetSleepModeParams));

        static bool lastLowLatencyMode = false;
        static uint32_t lastMinimumIntervalUs = UINT32_MAX;

        if (!nvapiAdapterRegistry)
            return ApiNotInitialized(n);

        if (!pDevice || !pSetSleepModeParams)
            return InvalidArgument(n);

        if (pSetSleepModeParams->version != NV_SET_SLEEP_MODE_PARAMS_VER1)
            return IncompatibleStructVersion(n, pSetSleepModeParams->version);

        auto lowLatencyDevice = NvapiD3dLowLatencyDevice::GetOrCreate(pDevice);
        if (!lowLatencyDevice || !lowLatencyDevice->SupportsLowLatency())
            return NoImplementation(n, alreadyLoggedNoReflex);

        switch (lowLatencyDevice->SetLatencySleepMode(pSetSleepModeParams->bLowLatencyMode, pSetSleepModeParams->bLowLatencyBoost, pSetSleepModeParams->minimumIntervalUs)) {
            case S_OK:
                if (lastLowLatencyMode != pSetSleepModeParams->bLowLatencyMode || lastMinimumIntervalUs != pSetSleepModeParams->minimumIntervalUs) {
                    lastLowLatencyMode = pSetSleepModeParams->bLowLatencyMode;
                    lastMinimumIntervalUs = pSetSleepModeParams->minimumIntervalUs;
                    return Ok(str::format(n, " (", pSetSleepModeParams->bLowLatencyMode ? (str::format("Enabled/", pSetSleepModeParams->minimumIntervalUs, "us")) : "Disabled", ")"));
                }
                return Ok(n, alreadyLoggedOk);
            case E_NOTIMPL:
                return NoImplementation(n, alreadyLoggedNoReflex);
            default:
                return Error(n, alreadyLoggedError);
        }
    }

    NvAPI_Status __cdecl NvAPI_D3D_GetSleepStatus(IUnknown* pDevice, NV_GET_SLEEP_STATUS_PARAMS* pGetSleepStatusParams) {
        constexpr auto n = __func__;
        thread_local bool alreadyLoggedNoReflex = false;
        thread_local bool alreadyLoggedOk = false;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(pDevice), log::fmt::ptr(pGetSleepStatusParams));

        if (!nvapiAdapterRegistry)
            return ApiNotInitialized(n);

        if (!pDevice || !pGetSleepStatusParams)
            return InvalidArgument(n);

        if (pGetSleepStatusParams->version != NV_GET_SLEEP_STATUS_PARAMS_VER1)
            return IncompatibleStructVersion(n, pGetSleepStatusParams->version);

        auto lowLatencyDevice = NvapiD3dLowLatencyDevice::GetOrCreate(pDevice);
        if (!lowLatencyDevice || !lowLatencyDevice->SupportsLowLatency())
            return NoImplementation(n, alreadyLoggedNoReflex);

        pGetSleepStatusParams->bLowLatencyMode = lowLatencyDevice->GetLowLatencyMode();

        return Ok(n, alreadyLoggedOk);
    }

    NvAPI_Status __cdecl NvAPI_D3D_GetLatency(IUnknown* pDev, NV_LATENCY_RESULT_PARAMS* pGetLatencyParams) {
        constexpr auto n = __func__;
        thread_local bool alreadyLoggedNoImpl = false;
        thread_local bool alreadyLoggedError = false;
        thread_local bool alreadyLoggedOk = false;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(pDev), log::fmt::ptr(pGetLatencyParams));

        if (!nvapiAdapterRegistry)
            return ApiNotInitialized(n);

        if (!pDev || !pGetLatencyParams)
            return InvalidArgument(n);

        if (pGetLatencyParams->version != NV_LATENCY_RESULT_PARAMS_VER1)
            return IncompatibleStructVersion(n, pGetLatencyParams->version);

        auto lowLatencyDevice = NvapiD3dLowLatencyDevice::GetOrCreate(pDev);
        if (!lowLatencyDevice || !lowLatencyDevice->SupportsLowLatency())
            return NoImplementation(n, alreadyLoggedNoImpl);

        switch (lowLatencyDevice->GetLatencyInfo(reinterpret_cast<D3D_LATENCY_RESULTS*>(pGetLatencyParams))) {
            case S_OK:
                return Ok(n, alreadyLoggedOk);
            case E_NOTIMPL:
                return NoImplementation(n, alreadyLoggedNoImpl);
            default:
                return Error(n, alreadyLoggedError);
        }
    }

    NvAPI_Status __cdecl NvAPI_D3D_SetLatencyMarker(IUnknown* pDev, NV_LATENCY_MARKER_PARAMS* pSetLatencyMarkerParams) {
        constexpr auto n = __func__;
        thread_local bool alreadyLoggedNoImpl = false;
        thread_local bool alreadyLoggedError = false;
        thread_local bool alreadyLoggedOk = false;
        thread_local bool alreadyLoggedMarkerTypeNotSupported = false;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(pDev), log::fmt::nv_latency_marker_params(pSetLatencyMarkerParams));

        if (!nvapiAdapterRegistry)
            return ApiNotInitialized(n);

        if (!pDev || !pSetLatencyMarkerParams)
            return InvalidArgument(n);

        if (pSetLatencyMarkerParams->version != NV_LATENCY_MARKER_PARAMS_VER1)
            return IncompatibleStructVersion(n, pSetLatencyMarkerParams->version);

        auto lowLatencyDevice = NvapiD3dLowLatencyDevice::GetOrCreate(pDev);
        if (!lowLatencyDevice || !lowLatencyDevice->SupportsLowLatency())
            return NoImplementation(n, alreadyLoggedNoImpl);

        auto markerType = NvapiD3dLowLatencyDevice::ToMarkerType(pSetLatencyMarkerParams->markerType);
        if (!markerType.has_value()) {
            // Silently drop unsupported marker types
            if (!std::exchange(alreadyLoggedMarkerTypeNotSupported, true))
                log::info(str::format("Not supported NV_LATENCY_MARKER_TYPE: ", pSetLatencyMarkerParams->markerType));

            return Ok(n, alreadyLoggedOk);
        }

        switch (lowLatencyDevice->SetLatencyMarker(pSetLatencyMarkerParams->frameID, markerType.value())) {
            case S_OK:
                return Ok(n, alreadyLoggedOk);
            case E_NOTIMPL:
                return NoImplementation(n, alreadyLoggedNoImpl);
            default:
                return Error(n, alreadyLoggedError);
        }
    }
}
