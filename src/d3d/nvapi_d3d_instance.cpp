#include "../util/util_log.h"
#include "nvapi_d3d_low_latency_device.h"
#include "nvapi_d3d_instance.h"

namespace dxvk {
    NvapiD3dInstance::NvapiD3dInstance(ResourceFactory& resourceFactory)
        : m_resourceFactory(resourceFactory) {}

    NvapiD3dInstance::~NvapiD3dInstance() = default;

    void NvapiD3dInstance::Initialize() {
        m_lfx = m_resourceFactory.CreateLfx();
        if (m_lfx->IsAvailable())
            log::write("LatencyFleX loaded and initialized successfully");
    }

    bool NvapiD3dInstance::IsReflexAvailable(IUnknown* device) {
        return NvapiD3dLowLatencyDevice::SupportsLowLatency(device) || m_lfx->IsAvailable();
    }

    bool NvapiD3dInstance::IsLowLatencyEnabled() const {
        return m_isLowLatencyEnabled;
    }

    bool NvapiD3dInstance::IsUsingLfx() const {
        return m_lfx->IsAvailable();
    }

    bool NvapiD3dInstance::SetReflexMode(IUnknown* device, bool enable, bool boost, uint32_t frameTimeUs) {
        bool result = true;

        if (IsReflexAvailable(device))
            m_isLowLatencyEnabled = enable;

        if (m_lfx->IsAvailable() && enable)
            m_lfx->SetTargetFrameTime(frameTimeUs * kNanoInMicro);
        else if (NvapiD3dLowLatencyDevice::SupportsLowLatency(device))
            result = NvapiD3dLowLatencyDevice::SetLatencySleepMode(device, enable, boost, frameTimeUs);

        return result;
    }

    bool NvapiD3dInstance::Sleep(IUnknown* device) {
        bool result = true;

        if (m_lfx->IsAvailable() && m_isLowLatencyEnabled)
            m_lfx->WaitAndBeginFrame();
        else if (NvapiD3dLowLatencyDevice::SupportsLowLatency(device))
            result = NvapiD3dLowLatencyDevice::LatencySleep(device);

        return result;
    }
}