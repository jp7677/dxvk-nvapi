#pragma once

#include "../nvapi_private.h"
#include "util_log.h"
#include "util_env.h"

namespace dxvk {
    inline void SetNgxDebugOptions() {
        static const auto setNgxIndicatorEnvName = "DXVK_NVAPI_SET_NGX_DEBUG_OPTIONS";
        static const std::unordered_map<std::string_view, std::string_view> settings = {
            {"EnableConsoleLogging", "EnableConsoleLogging"},
            {"LogLevel", "LogLevel"},
            {"ShowDlssIndicator", "ShowDlssIndicator"},
            {"DLSSG_IndicatorText", "DLSSG_IndicatorText"},
            {"Logging", "EnableConsoleLogging"},
            {"DLSSIndicator", "ShowDlssIndicator"},
            {"DLSSGIndicator", "DLSSG_IndicatorText"},
        };
        static const auto setNgxIndicator = env::getEnvVariable(setNgxIndicatorEnvName);

        if (setNgxIndicator.empty())
            return;

        std::set<std::string_view, str::CaseInsensitiveCompare<std::string_view>> keys;
        std::transform(settings.begin(), settings.end(), std::inserter(keys, keys.begin()), [](const auto& s) { return s.first; });

        auto setNgxIndicatorMap = str::parsekeydwords(setNgxIndicator, keys);

        if (setNgxIndicatorMap.empty()) {
            log::info(str::format(setNgxIndicatorEnvName, " is set to an invalid value, please use the format 'key=value,key=value'"));
            return;
        }

        HKEY key{};
        auto status = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\NVIDIA Corporation\\Global\\NGXCore", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, &key);
        if (status != NO_ERROR || !key) {
            log::info(str::format(R"(Failed to open SOFTWARE\NVIDIA Corporation\Global\NGXCore registry key: )", status));
            return;
        }

        std::for_each(settings.begin(), settings.end(),
            [&](const auto& pair) {
                if (!setNgxIndicatorMap.contains(pair.first))
                    return;

                auto value = setNgxIndicatorMap.at(pair.first);
                log::info(str::format("Set ", pair.second, " in registry to 0x", std::hex, value));

                std::wstring name = std::wstring(pair.second.begin(), pair.second.end());
                if (RegSetValueExW(key, name.c_str(), 0, REG_DWORD, reinterpret_cast<const BYTE*>(&value), sizeof(DWORD)) != NO_ERROR)
                    log::info(str::format("Failed to set ", pair.second, " registry key"));
            });

        RegCloseKey(key);
    }
}
