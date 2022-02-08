#include "../util/util_log.h"
#include "nvapi_d3d_instance.h"

namespace dxvk {
    NvapiD3dInstance::NvapiD3dInstance(ResourceFactory &resourceFactory)
            : m_resourceFactory(resourceFactory) {}

    NvapiD3dInstance::~NvapiD3dInstance() = default;

    void NvapiD3dInstance::Initialize() {
        m_lfx = m_resourceFactory.CreateLfx();
        if (m_lfx->IsAvailable())
            log::write("LatencyFleX loaded and initialized successfully");
    }

    bool NvapiD3dInstance::IsReflexAvailable() {
        return m_lfx->IsAvailable();
    }

    bool NvapiD3dInstance::IsReflexEnabled() const {
        return m_isLfxEnabled;
    }

    void NvapiD3dInstance::SetReflexEnabled(bool value) {
        m_isLfxEnabled = value;
    }

    void NvapiD3dInstance::Sleep() {
        if (m_isLfxEnabled)
            m_lfx->WaitAndBeginFrame();
    }

    void NvapiD3dInstance::SetTargetFrameTime(uint64_t frameTimeUs) {
        constexpr uint64_t kNanoInMicro = 1000;
        m_lfx->SetTargetFrameTime(frameTimeUs * kNanoInMicro);
    }
}