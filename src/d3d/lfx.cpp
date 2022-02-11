#include "lfx.h"

#include <windows.h>
#include "../util/util_string.h"
#include "../util/util_log.h"

namespace dxvk {
    Lfx::Lfx() {
        const auto lfxModuleName = "latencyflex_layer.dll";
        const auto lfxModuleNameFallback = "latencyflex_wine.dll";
        auto useFallbackEntrypoints = false;

        m_lfxModule = ::LoadLibraryA(lfxModuleName);
        if (m_lfxModule == nullptr && ::GetLastError() == ERROR_MOD_NOT_FOUND) {
            // Try fallback entrypoints. These were used by versions prior to [9c2836f].
            // The fallback logic can be removed once enough time has passed since the release.
            // [9c2836f]: https://github.com/ishitatsuyuki/LatencyFleX/commit/9c2836faf14196190a915064b53c27e675e47960
            m_lfxModule = ::LoadLibraryA(lfxModuleNameFallback);
            useFallbackEntrypoints = true;
        }

        if (m_lfxModule == nullptr) {
            auto lastError = ::GetLastError();
            if (lastError != ERROR_MOD_NOT_FOUND) // Ignore library not found
                log::write(str::format("Loading ", !useFallbackEntrypoints ? lfxModuleName : lfxModuleNameFallback,
                                       " failed with error code: ", lastError));
            return;
        }

        m_lfx_WaitAndBeginFrame = reinterpret_cast<PFN_lfx_WaitAndBeginFrame>(reinterpret_cast<void*>(
                GetProcAddress(m_lfxModule,
                               !useFallbackEntrypoints ? "lfx_WaitAndBeginFrame" : "winelfx_WaitAndBeginFrame")));
        m_lfx_SetTargetFrameTime = reinterpret_cast<PFN_lfx_SetTargetFrameTime>(reinterpret_cast<void*>(
                GetProcAddress(m_lfxModule,
                               !useFallbackEntrypoints ? "lfx_SetTargetFrameTime" : "winelfx_SetTargetFrameTime")));
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