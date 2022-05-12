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
        typedef void (*PFN_lfx_WaitAndBeginFrame)();
        typedef void (*PFN_lfx_SetTargetFrameTime)(__int64);

        HMODULE m_lfxModule{};
        PFN_lfx_WaitAndBeginFrame m_lfx_WaitAndBeginFrame{};
        PFN_lfx_SetTargetFrameTime m_lfx_SetTargetFrameTime{};
    };
}