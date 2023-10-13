#pragma once

#include "../nvapi_private.h"

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#endif // __GNUC__

/**
 * \brief Frame Report Info
 */
typedef struct D3D_LATENCY_RESULTS {
    UINT32 version;
    struct D3D_FRAME_REPORT {
        UINT64 frameID;
        UINT64 inputSampleTime;
        UINT64 simStartTime;
        UINT64 simEndTime;
        UINT64 renderSubmitStartTime;
        UINT64 renderSubmitEndTime;
        UINT64 presentStartTime;
        UINT64 presentEndTime;
        UINT64 driverStartTime;
        UINT64 driverEndTime;
        UINT64 osRenderQueueStartTime;
        UINT64 osRenderQueueEndTime;
        UINT64 gpuRenderStartTime;
        UINT64 gpuRenderEndTime;
        UINT32 gpuActiveRenderTimeUs;
        UINT32 gpuFrameTimeUs;
        UINT8 rsvd[120];
    } frame_reports[64];
    UINT8 rsvd[32];
} D3D_LATENCY_RESULTS;

MIDL_INTERFACE("f3112584-41f9-348d-a59b-00b7e1d285d6")
ID3DLowLatencyDevice : public IUnknown {
    virtual BOOL STDMETHODCALLTYPE SupportsLowLatency() = 0;

    virtual HRESULT STDMETHODCALLTYPE LatencySleep() = 0;

    virtual HRESULT STDMETHODCALLTYPE SetLatencySleepMode(
        BOOL lowLatencyMode,
        BOOL lowLatencyBoost,
        uint32_t minimumIntervalUs) = 0;

    virtual HRESULT STDMETHODCALLTYPE SetLatencyMarker(
        uint64_t frameID,
        uint32_t markerType) = 0;

    virtual HRESULT STDMETHODCALLTYPE GetLatencyInfo(
        D3D_LATENCY_RESULTS * latencyResults) = 0;
};

#ifndef _MSC_VER
__CRT_UUID_DECL(ID3DLowLatencyDevice, 0xf3112584, 0x41f9, 0x348d, 0xa5, 0x9b, 0x00, 0xb7, 0xe1, 0xd2, 0x85, 0xd6);
#endif
