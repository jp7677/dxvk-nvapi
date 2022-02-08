#pragma once

#include <minwindef.h>
#include <cstdint>

namespace dxvk {
    class Lfx {
    public:
        Lfx();
        virtual ~Lfx();

        [[nodiscard]] virtual bool IsAvailable() const;
        virtual void WaitAndBeginFrame();
        virtual void SetTargetFrameTime(uint64_t frame_time_ns);

    private:
        HMODULE m_lfxModule{};
        bool m_enabled = false;

        typedef void (*PFN_lfx_WaitAndBeginFrame)();
        typedef void (*PFN_lfx_SetTargetFrameTime)(__int64);

        PFN_lfx_WaitAndBeginFrame m_lfx_WaitAndBeginFrame{};
        PFN_lfx_SetTargetFrameTime m_lfx_SetTargetFrameTime{};
    };
}