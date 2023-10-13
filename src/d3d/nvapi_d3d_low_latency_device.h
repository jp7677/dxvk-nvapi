#pragma once

#include "../nvapi_private.h"
#include "../shared/shared_interfaces.h"
#include "../util/com_pointer.h"

namespace dxvk {
    class NvapiD3dLowLatencyDevice {
      public:
        static bool SupportsLowLatency(IUnknown* device);
        static bool LatencySleep(IUnknown* device);
        static bool SetLatencySleepMode(IUnknown* device, bool lowLatencyMode, bool lowLatencyBoost, uint32_t minimumIntervalUs);
        static bool GetLatencyInfo(IUnknown* device, D3D_LATENCY_RESULTS* latencyResults);
        static bool SetLatencyMarker(IUnknown* device, uint64_t frameID, uint32_t markerType);

        static void ClearCacheMaps();

      private:
        inline static std::unordered_map<IUnknown*, ID3DLowLatencyDevice*> m_lowLatencyDeviceMap;

        inline static std::mutex m_LowLatencyDeviceMutex;

        [[nodiscard]] static Com<ID3DLowLatencyDevice> GetLowLatencyDevice(IUnknown* device);
    };
}