#pragma once

#include "../nvapi_private.h"
#include "util_latency_marker_code.h"
#include "util_string.h"

namespace dxvk::log {
    namespace fmt {
#ifdef _MSC_VER
        constexpr auto hex_prefix = "0x";
#else
        constexpr auto hex_prefix = "";
#endif

        inline std::string hnd(const void* h) {
            if (h == nullptr)
                return "hnd=0x0";

            return str::format("hnd=", hex_prefix, std::hex, h);
        }

        inline std::string ptr(const void* p) {
            if (p == nullptr)
                return "nullptr";

            return str::format("ptr=", hex_prefix, std::hex, p);
        }

        inline std::string flt(const float f) {
            if (f == 0)
                return "0.0";

            if (f == 1)
                return "1.0";

            return str::format(f);
        }

        inline std::string flags(const uint32_t h) {
            if (h == 0)
                return "0x0";

            return str::format("flags=0x", std::setfill('0'), std::setw(4), std::hex, h);
        }

        inline std::string nv_latency_marker_params(NV_LATENCY_MARKER_PARAMS* p) {
            if (p == nullptr)
                return "nullptr";

            return str::format("{version=", p->version, ",frameID=", p->frameID, ",markerType=", fromLatencyMarkerType(p->markerType), ",rsvd}");
        }

        inline std::string nv_async_frame_marker_params(NV_ASYNC_FRAME_MARKER_PARAMS* p) {
            if (p == nullptr)
                return "nullptr";

            return str::format("{version=", p->version, ",frameID=", p->frameID, ",markerType=", fromLatencyMarkerType(p->markerType), ",presentFrameID=", p->presentFrameID, ",rsvd}");
        }

        inline std::string nv_vk_get_sleep_status_params(NV_VULKAN_GET_SLEEP_STATUS_PARAMS* p) {
            if (p == nullptr)
                return "nullptr";

            return str::format("{version=", p->version, ",...,rsvd}");
        }

        inline std::string nv_vk_set_sleep_status_params(NV_VULKAN_SET_SLEEP_MODE_PARAMS* p) {
            if (p == nullptr)
                return "nullptr";

            return str::format("{version=", p->version, ",bLowLatencyMode=", static_cast<bool>(p->bLowLatencyMode), ",bLowLatencyBoost=", static_cast<bool>(p->bLowLatencyBoost), ",minimumIntervalUs=", p->minimumIntervalUs, ",rsvd}");
        }

        inline std::string nv_vk_latency_result_params(NV_VULKAN_LATENCY_RESULT_PARAMS* p) {
            if (p == nullptr)
                return "nullptr";

            return str::format("{version=", p->version, ",...,rsvd}");
        }

        inline std::string nv_vk_latency_marker_params(NV_VULKAN_LATENCY_MARKER_PARAMS* p) {
            if (p == nullptr)
                return "nullptr";

            return str::format("{version=", p->version, ",frameID=", p->frameID, ",markerType=", p->markerType, ",rsvd}");
        }

        inline std::string d3d12_cpu_descriptor_handle(D3D12_CPU_DESCRIPTOR_HANDLE h) {
            return str::format("{ptr=", hex_prefix, std::hex, h.ptr, "}");
        }
    }

    bool tracing();

    void write(const std::string& level, const std::string& message);

    inline void info(const std::string& message) {
        log::write("info", message);
    }

    template <typename T>
    void append(std::stringstream& str, const T& arg) {
        str << arg;
    }

    template <typename T, typename... Tx>
    void append(std::stringstream& str, const T& arg, const Tx&... args) {
        str << arg << ", ";
        append(str, args...);
    }

    template <typename... Args>
    void trace(const std::string& name, const Args&... args) {
        std::stringstream stream;
        append(stream, args...);
        log::write("trace", str::format(name, " (", stream.str(), ")"));
    }
}
