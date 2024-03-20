#include "nvapi_d3d_low_latency_device.h"

namespace dxvk {
    LowLatencyFrameIdGenerator::LowLatencyFrameIdGenerator() : m_nextLowLatencyDeviceFrameId(1),
                                                               m_applicationIdList({0}) {}

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

    bool NvapiD3dLowLatencyDevice::SupportsLowLatency(IUnknown* device) {
        auto d3dLowLatencyDevice = GetLowLatencyDevice(device);
        if (d3dLowLatencyDevice == nullptr)
            return false;

        return d3dLowLatencyDevice->SupportsLowLatency();
    }

    bool NvapiD3dLowLatencyDevice::LatencySleep(IUnknown* device) {
        auto d3dLowLatencyDevice = GetLowLatencyDevice(device);
        if (d3dLowLatencyDevice == nullptr)
            return false;

        return SUCCEEDED(d3dLowLatencyDevice->LatencySleep());
    }

    bool NvapiD3dLowLatencyDevice::SetLatencySleepMode(IUnknown* device, bool lowLatencyMode, bool lowLatencyBoost, uint32_t minimumIntervalUs) {
        auto d3dLowLatencyDevice = GetLowLatencyDevice(device);
        if (d3dLowLatencyDevice == nullptr)
            return false;

        return SUCCEEDED(d3dLowLatencyDevice->SetLatencySleepMode(lowLatencyMode, lowLatencyBoost, minimumIntervalUs));
    }

    bool NvapiD3dLowLatencyDevice::GetLatencyInfo(IUnknown* device, D3D_LATENCY_RESULTS* latencyResults) {
        auto d3dLowLatencyDevice = GetLowLatencyDevice(device);
        if (d3dLowLatencyDevice == nullptr)
            return false;

        if (FAILED(d3dLowLatencyDevice->GetLatencyInfo(latencyResults)))
            return false;

        auto frameIdGenerator = GetFrameIdGenerator(device);
        for (auto& frameReport : latencyResults->frame_reports) {
            frameReport.frameID = frameIdGenerator->GetApplicationFrameId(frameReport.frameID);
            if (!frameReport.frameID) {
                memset(latencyResults->frame_reports, 0, sizeof(latencyResults->frame_reports));
                break;
            }
        }

        return true;
    }

    bool NvapiD3dLowLatencyDevice::SetLatencyMarker(IUnknown* device, uint64_t frameID, uint32_t markerType) {
        auto d3dLowLatencyDevice = GetLowLatencyDevice(device);
        if (d3dLowLatencyDevice == nullptr)
            return false;

        return SUCCEEDED(d3dLowLatencyDevice->SetLatencyMarker(
            GetFrameIdGenerator(device)->GetLowLatencyDeviceFrameId(frameID), markerType));
    }

    void NvapiD3dLowLatencyDevice::ClearCacheMaps() {
        std::scoped_lock lock(m_lowLatencyDeviceMutex, m_lowLatencyFrameIdGeneratorMutex);

        m_lowLatencyDeviceMap.clear();
        m_frameIdGeneratorMap.clear();
    }

    Com<ID3DLowLatencyDevice> NvapiD3dLowLatencyDevice::GetLowLatencyDevice(IUnknown* device) {
        if (device == nullptr)
            return nullptr;

        std::scoped_lock lock(m_lowLatencyDeviceMutex);
        auto it = m_lowLatencyDeviceMap.find(device);
        if (it != m_lowLatencyDeviceMap.end())
            return it->second;

        Com<ID3DLowLatencyDevice> d3dLowLatencyDevice;
        if (FAILED(device->QueryInterface(IID_PPV_ARGS(&d3dLowLatencyDevice))))
            return nullptr;

        m_lowLatencyDeviceMap.emplace(device, d3dLowLatencyDevice.ptr());

        return d3dLowLatencyDevice;
    }

    LowLatencyFrameIdGenerator* NvapiD3dLowLatencyDevice::GetFrameIdGenerator(IUnknown* device) {
        std::scoped_lock lock(m_lowLatencyFrameIdGeneratorMutex);
        auto it = m_frameIdGeneratorMap.find(device);
        if (it != m_frameIdGeneratorMap.end())
            return it->second.get();

        return m_frameIdGeneratorMap.emplace(device, std::make_unique<LowLatencyFrameIdGenerator>()).first->second.get();
    }
}
