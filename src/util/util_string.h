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

    std::string fromnullable(const char* str);

    template <typename T>
        requires std::default_initializable<T>
        && std::constructible_from<typename T::value_type, std::string::const_iterator, std::string::const_iterator>
        && requires(T t, T::iterator it, T::value_type&& s) {
               requires std::input_iterator<typename T::iterator>;
               { t.end() } -> std::same_as<typename T::iterator>;
               { t.insert(it, s) } -> std::same_as<typename T::iterator>;
           }
    T split(const std::string& str, const std::regex& separator) {
        static const std::sregex_token_iterator end;

        T result;

        if (str.empty())
            return result;

        std::transform(
            std::sregex_token_iterator(str.begin(), str.end(), separator, -1),
            end,
            std::inserter(result, result.end()),
            [](const std::ssub_match& match) {
                return typename T::value_type(match.first, match.second);
            });

        return result;
    }

    template <typename T>
        requires requires(T t) {
            requires std::input_iterator<typename T::iterator>;
            { t.begin() } -> std::same_as<typename T::iterator>;
            { t.end() } -> std::same_as<typename T::iterator>;
        }
    struct CaseInsensitiveCompare {
        bool operator()(const T& lhs, const T& rhs) const {
            return std::lexicographical_compare(
                lhs.begin(),
                lhs.end(),
                rhs.begin(),
                rhs.end(),
                [](const auto& l, const auto& r) {
                    return std::toupper(l, std::locale::classic()) < std::toupper(r, std::locale::classic());
                });
        }
    };
}
