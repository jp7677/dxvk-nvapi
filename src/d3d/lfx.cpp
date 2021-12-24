#include "lfx.h"

#include <windows.h>
#include "../util/util_string.h"
#include "../util/util_log.h"

namespace dxvk {
    Lfx::Lfx() {
        const auto lfxModuleName = "latencyflex_wine.dll";
        m_lfxModule = ::LoadLibraryA(lfxModuleName);
        if (m_lfxModule == nullptr) {
            auto lastError = ::GetLastError();
            if (lastError != ERROR_MOD_NOT_FOUND) // Ignore library not found
                log::write(str::format("Loading ", lfxModuleName, " failed with error code: ", lastError));

            return;
        }

        m_winelfx_WaitAndBeginFrame = reinterpret_cast<PFN_winelfx_WaitAndBeginFrame>(reinterpret_cast<void*>(GetProcAddress(m_lfxModule, "winelfx_WaitAndBeginFrame")));
        m_winelfx_SetTargetFrameTime = reinterpret_cast<PFN_winelfx_SetTargetFrameTime>(reinterpret_cast<void*>(GetProcAddress(m_lfxModule, "winelfx_SetTargetFrameTime")));
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
        if (m_winelfx_WaitAndBeginFrame)
            m_winelfx_WaitAndBeginFrame();
    }

    void Lfx::SetTargetFrameTime(uint64_t frame_time_ns) {
        if (m_winelfx_SetTargetFrameTime)
            m_winelfx_SetTargetFrameTime(static_cast<__int64>(frame_time_ns));
    }
}