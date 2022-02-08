#include "lfx.h"

#include <windows.h>
#include "../util/util_string.h"
#include "../util/util_log.h"

namespace dxvk {
    Lfx::Lfx() {
        const auto lfxModuleName = "latencyflex_layer.dll";
        const auto lfxModuleNameFallback = "latencyflex_wine.dll";
        m_lfxModule = ::LoadLibraryA(lfxModuleName);
        if (m_lfxModule) {
            m_lfx_WaitAndBeginFrame = reinterpret_cast<PFN_lfx_WaitAndBeginFrame>(GetProcAddress(m_lfxModule,
                                                                                                 "lfx_WaitAndBeginFrame"));
            m_lfx_SetTargetFrameTime = reinterpret_cast<PFN_lfx_SetTargetFrameTime>(GetProcAddress(m_lfxModule,
                                                                                                   "lfx_SetTargetFrameTime"));
        } else {
            auto lastError = ::GetLastError();
            if (lastError != ERROR_MOD_NOT_FOUND) {
                log::write(str::format("Loading ", lfxModuleName, " failed with error code: ", lastError));
            } else {
                // Try fallback entrypoints. These were used by an older version of LatencyFleX.
                m_lfxModule = ::LoadLibraryA(lfxModuleNameFallback);
                if (m_lfxModule) {
                    m_lfx_WaitAndBeginFrame = reinterpret_cast<PFN_lfx_WaitAndBeginFrame>(GetProcAddress(m_lfxModule,
                                                                                                         "winelfx_WaitAndBeginFrame"));
                    m_lfx_SetTargetFrameTime = reinterpret_cast<PFN_lfx_SetTargetFrameTime>(GetProcAddress(m_lfxModule,
                                                                                                           "winelfx_SetTargetFrameTime"));
                } else {
                    lastError = ::GetLastError();
                    if (lastError != ERROR_MOD_NOT_FOUND) // Ignore library not found
                        log::write(
                                str::format("Loading ", lfxModuleNameFallback, " failed with error code: ", lastError));
                }
            }
        }
    }

    Lfx::~Lfx() {
        if (m_lfxModule == nullptr) return;
        ::FreeLibrary(m_lfxModule);
        m_lfxModule = nullptr;
    }

    bool Lfx::IsAvailable() const {
        return m_lfxModule != nullptr;
    }

    void Lfx::WaitAndBeginFrame() {
        if (m_lfx_WaitAndBeginFrame)
            m_lfx_WaitAndBeginFrame();
    }

    void Lfx::SetTargetFrameTime(uint64_t frame_time_ns) {
        if (m_lfx_SetTargetFrameTime)
            m_lfx_SetTargetFrameTime(static_cast<__int64>(frame_time_ns));
    }
}