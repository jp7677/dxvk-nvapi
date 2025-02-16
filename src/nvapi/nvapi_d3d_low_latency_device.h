#pragma once

#include "../nvapi_private.h"
#include "../interfaces/shared_interfaces.h"
#include "low_latency_frame_id_generator.h"

namespace dxvk {
    class NvapiD3dLowLatencyDevice {

      public:
        static void Reset();
        [[nodiscard]] static NvapiD3dLowLatencyDevice* GetOrCreate(IUnknown* device);
        [[nodiscard]] static std::optional<uint32_t> ToMarkerType(NV_LATENCY_MARKER_TYPE markerType);

        explicit NvapiD3dLowLatencyDevice(ID3DLowLatencyDevice* m_d3dLowLatencyDevice);

        [[nodiscard]] bool SupportsLowLatency() const;
        [[nodiscard]] bool LatencySleep() const;
        bool SetLatencySleepMode(bool lowLatencyMode, bool lowLatencyBoost, uint32_t minimumIntervalUs);
        bool GetLatencyInfo(D3D_LATENCY_RESULTS* latencyResults);
        [[nodiscard]] bool SetLatencyMarker(uint64_t frameID, uint32_t markerType);
        [[nodiscard]] bool GetLowLatencyMode() const;

      private:
        [[nodiscard]] static NvapiD3dLowLatencyDevice* Get(IUnknown*);

        static std::unordered_map<IUnknown*, std::shared_ptr<NvapiD3dLowLatencyDevice>> m_lowLatencyDeviceMap;
        static std::mutex m_mutex;

        ID3DLowLatencyDevice* m_d3dLowLatencyDevice{};
        LowLatencyFrameIdGenerator m_frameIdGenerator;
        bool m_lowLatencyMode{};
    };
}