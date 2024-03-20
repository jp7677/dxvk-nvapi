#pragma once

#include "../nvapi_private.h"
#include "../shared/shared_interfaces.h"
#include "../util/com_pointer.h"

namespace dxvk {
    class LowLatencyFrameIdGenerator {
      public:
        LowLatencyFrameIdGenerator();
        virtual ~LowLatencyFrameIdGenerator();
        bool LowLatencyDeviceFrameIdInWindow(uint64_t lowLatencyDeviceFrameId) const;
        uint64_t GetLowLatencyDeviceFrameId(uint64_t applicationFrameId);
        uint64_t GetApplicationFrameId(uint64_t lowLatencyDeviceFrameId);

      private:
        std::mutex m_frameIdGeneratorMutex;

        uint64_t m_nextLowLatencyDeviceFrameId;
        std::unordered_map<uint64_t, uint64_t> m_applicationIdToDeviceId;

        static constexpr uint32_t applicationIdListSize = 1000;
        std::array<uint64_t, applicationIdListSize> m_applicationIdList;
    };

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
        inline static std::unordered_map<IUnknown*, std::unique_ptr<LowLatencyFrameIdGenerator>> m_frameIdGeneratorMap;

        inline static std::mutex m_lowLatencyDeviceMutex;
        inline static std::mutex m_lowLatencyFrameIdGeneratorMutex;

        [[nodiscard]] static Com<ID3DLowLatencyDevice> GetLowLatencyDevice(IUnknown* device);
        [[nodiscard]] static LowLatencyFrameIdGenerator* GetFrameIdGenerator(IUnknown* device);
    };
}