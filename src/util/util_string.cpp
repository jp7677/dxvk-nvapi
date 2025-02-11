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

    void copynvus(NvAPI_UnicodeString dst, NvAPI_UnicodeString src) {
        auto w = std::wstring(reinterpret_cast<wchar_t*>(src));
        w.copy(reinterpret_cast<wchar_t*>(dst), std::min(w.size(), static_cast<size_t>(NVAPI_UNICODE_STRING_MAX)));
    }

    void tonvss(NvAPI_ShortString nvss, std::string str) {
        str.resize(NVAPI_SHORT_STRING_MAX - 1);
        strcpy(nvss, str.c_str());
    }

    std::string fromnullable(const char* str) {
        return str ? std::string(str) : std::string();
    }

    bool parsedword(const std::string_view& str, NvU32& value) {
        if (str.empty())
            return false;

        auto end = str.data() + str.size();
        auto result = str.size() > 2 && str[0] == '0' && (str[1] == 'x' || str[1] == 'X')
            ? std::from_chars(str.data() + 2, end, value, 16)
            : std::from_chars(str.data(), end, value, 10);

        return result.ec == std::errc() && result.ptr == end;
    }

    std::unordered_map<NvU32, NvU32> parsedwords(const std::string& str) {
        std::unordered_map<NvU32, NvU32> result;

        if (str.empty())
            return result;

        auto entries = split<std::vector<std::string_view>>(str, std::regex(","));

        for (auto entry : entries) {
            auto eq = entry.find('=');

            if (eq == entry.npos || eq == 0 || eq == entry.size() - 1)
                continue;

            NvU32 key, value;

            if (parsedword(entry.substr(0, eq), key) && parsedword(entry.substr(eq + 1), value))
                result[key] = value;
        }

        return result;
    }

    std::unordered_map<std::string_view, NvU32> parsekeydwords(const std::string& str, const std::set<std::string_view, str::CaseInsensitiveCompare<std::string_view>>& keys) {
        std::unordered_map<std::string_view, NvU32> result;
        auto entries = str::split<std::vector<std::string_view>>(str, std::regex(","));

        for (auto entry : entries) {
            auto eq = entry.find('=');

            if (eq == entry.npos || eq == 0 || eq == entry.size() - 1)
                continue;

            auto key = entry.substr(0, eq);
            auto it = keys.find(key);
            if (it == keys.end())
                continue;

            NvU32 value;
            if (str::parsedword(entry.substr(eq + 1), value))
                result[*it] = value;
        }

        return result;
    }
}
