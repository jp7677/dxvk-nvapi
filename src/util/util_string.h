#pragma once

#include "../nvapi_private.h"

namespace dxvk::str {
    std::string fromws(const WCHAR* ws);

    void tows(const char* mbs, WCHAR* wcs, size_t wcsLen);

    template <size_t N>
    void tows(const char* mbs, WCHAR (&wcs)[N]) {
        return tows(mbs, wcs, N);
    }

    std::wstring tows(const char* mbs);

    std::string fromnvus(NvAPI_UnicodeString nvus);

    void tonvss(NvAPI_ShortString nvss, std::string str);

    inline void append(std::stringstream&) {}

    template <typename... Tx>
    void append(std::stringstream& str, const WCHAR* arg, const Tx&... args) {
        str << fromws(arg);
        append(str, args...);
    }

    template <typename T, typename... Tx>
    void append(std::stringstream& str, const T& arg, const Tx&... args) {
        str << arg;
        append(str, args...);
    }

    template <typename... Args>
    std::string format(const Args&... args) {
        std::stringstream stream;
        append(stream, args...);
        return stream.str();
    }
}
