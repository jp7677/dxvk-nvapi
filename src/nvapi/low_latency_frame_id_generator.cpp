#include "nvapi_d3d_low_latency_device.h"

namespace dxvk {
    LowLatencyFrameIdGenerator::LowLatencyFrameIdGenerator()
        : m_nextLowLatencyDeviceFrameId(1),
          m_applicationIdList({0}) {
        m_lastFrameId.fill(std::numeric_limits<uint64_t>::max());
    }

    LowLatencyFrameIdGenerator::~LowLatencyFrameIdGenerator() = default;

    uint64_t LowLatencyFrameIdGenerator::GetLowLatencyDeviceFrameId(uint64_t applicationFrameId) {
        std::scoped_lock lock(m_frameIdGeneratorMutex);

        auto it = m_applicationIdToDeviceId.find(applicationFrameId);
        if (it != m_applicationIdToDeviceId.end())
            return it->second;

        uint64_t lowLatencyDeviceFrameId = m_nextLowLatencyDeviceFrameId++;
        uint64_t frameIdIndex = (lowLatencyDeviceFrameId - 1) % applicationIdListSize;

        if ((lowLatencyDeviceFrameId - 1) >= applicationIdListSize)
            m_applicationIdToDeviceId.erase(m_applicationIdList[frameIdIndex]);

        m_applicationIdToDeviceId[applicationFrameId] = lowLatencyDeviceFrameId;
        m_applicationIdList[frameIdIndex] = applicationFrameId;

        return lowLatencyDeviceFrameId;
    }

    bool LowLatencyFrameIdGenerator::LowLatencyDeviceFrameIdInWindow(uint64_t lowLatencyDeviceFrameId) const {
        return ((lowLatencyDeviceFrameId < m_nextLowLatencyDeviceFrameId)
            && ((m_nextLowLatencyDeviceFrameId - lowLatencyDeviceFrameId) < applicationIdListSize));
    }

    uint64_t LowLatencyFrameIdGenerator::GetApplicationFrameId(uint64_t lowLatencyDeviceFrameId) {
        std::scoped_lock lock(m_frameIdGeneratorMutex);

        if (!lowLatencyDeviceFrameId || !LowLatencyDeviceFrameIdInWindow(lowLatencyDeviceFrameId))
            return 0;

        return m_applicationIdList[((lowLatencyDeviceFrameId - 1) % applicationIdListSize)];
    }

    bool LowLatencyFrameIdGenerator::IsRepeatedFrame(uint64_t frameID, uint32_t markerType) {
        // Should always be within bounds since we drop unsupported marker types in the entrypoints
        return std::exchange(m_lastFrameId[markerType], frameID) == frameID;
    }
}
