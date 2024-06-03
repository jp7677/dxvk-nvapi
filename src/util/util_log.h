#pragma once

#include "../nvapi_private.h"
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

        inline std::string d3d12_cpu_descriptor_handle(D3D12_CPU_DESCRIPTOR_HANDLE h) {
            return str::format("{ptr=", hex_prefix, std::hex, h.ptr, "}");
        }
    }

    bool tracing();

    void write(const std::string& level, const std::string& message);

    inline void info(const std::string& message) {
        log::write("info", message);
    }

    inline void trace1(std::stringstream&) {}

    template <typename T, typename... Tx>
    void trace1(std::stringstream& str, const T& arg, const Tx&... args) {
        str << arg << ", ";
        trace1(str, args...);
    }

    template <typename... Args>
    void trace(const std::string& name, const Args&... args) {
        std::stringstream stream;
        trace1(stream, args...);
        auto str = stream.str();
        if (str.ends_with(", "))
            str.erase(str.length() - 2);

        log::write("trace", str::format(name, " (", str, ")"));
    }
}
