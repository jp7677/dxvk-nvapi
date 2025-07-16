#pragma once

#include "../nvapi_private.h"

namespace dxvk {
    class LowLatencyFrameIdGenerator {

      public:
        LowLatencyFrameIdGenerator();
        virtual ~LowLatencyFrameIdGenerator();
        [[nodiscard]] bool LowLatencyDeviceFrameIdInWindow(uint64_t lowLatencyDeviceFrameId) const;
        uint64_t GetLowLatencyDeviceFrameId(uint64_t applicationFrameId);
        uint64_t GetApplicationFrameId(uint64_t lowLatencyDeviceFrameId);

      private:
        std::mutex m_frameIdGeneratorMutex;

        uint64_t m_nextLowLatencyDeviceFrameId;
        std::unordered_map<uint64_t, uint64_t> m_applicationIdToDeviceId;

        static constexpr uint32_t applicationIdListSize = 1000;
        std::array<uint64_t, applicationIdListSize> m_applicationIdList;
    };
}
