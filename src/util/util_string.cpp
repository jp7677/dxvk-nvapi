#include "util_string.h"

namespace dxvk::str {
    std::string fromws(const WCHAR* ws) {
        auto len = ::WideCharToMultiByte(CP_UTF8, 0, ws, -1, nullptr, 0, nullptr, nullptr);

        if (len <= 1)
            return "";

        len -= 1;

        std::string result;
        result.resize(len);
        ::WideCharToMultiByte(CP_UTF8, 0, ws, -1, &result[0], len, nullptr, nullptr);
        return result;
    }

    void tows(const char* mbs, WCHAR* wcs, size_t wcsLen) {
        ::MultiByteToWideChar(CP_UTF8, 0, mbs, -1, wcs, static_cast<int>(wcsLen));
    }

    std::wstring tows(const char* mbs) {
        auto len = ::MultiByteToWideChar(CP_UTF8, 0, mbs, -1, nullptr, 0);

        if (len <= 1)
            return L"";

        len -= 1;

        std::wstring result;
        result.resize(len);
        ::MultiByteToWideChar(CP_UTF8, 0, mbs, -1, &result[0], len);
        return result;
    }

    std::string fromnvus(NvAPI_UnicodeString nvus) {
        auto w = std::wstring(reinterpret_cast<wchar_t*>(nvus));
        return {w.begin(), w.end()};
    }
}
