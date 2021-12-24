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

        typedef void (*PFN_winelfx_WaitAndBeginFrame)();
        typedef void (*PFN_winelfx_SetTargetFrameTime)(__int64);

        PFN_winelfx_WaitAndBeginFrame m_winelfx_WaitAndBeginFrame{};
        PFN_winelfx_SetTargetFrameTime m_winelfx_SetTargetFrameTime{};
    };
}