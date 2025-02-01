#pragma once

#include "../nvapi_private.h"
#include "util_log.h"
#include "util_env.h"

namespace dxvk {
    inline void setDlssIndicator() {
        constexpr static auto showDLSSIndicatorEnvName = "DXVK_NVAPI_SET_DLSS_INDICATOR";
        const static auto showDLSSIndicator = env::getEnvVariable(showDLSSIndicatorEnvName);

        if (showDLSSIndicator.empty())
            return;

        NvU32 showDLSSIndicatorValue;
        if (!str::parsedword(std::string_view(showDLSSIndicator), showDLSSIndicatorValue)) {
            log::info(str::format(showDLSSIndicatorEnvName, " is set to '", showDLSSIndicator, "', but this value is invalid, please set a number"));
            return;
        }

        if (showDLSSIndicatorValue != 0)
            log::info(str::format(showDLSSIndicatorEnvName, " is set, enable ShowDlssIndicator in registry"));
        else
            log::info(str::format(showDLSSIndicatorEnvName, " is set to 0, disable ShowDlssIndicator in registry"));

        HKEY key{};
        auto status = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\NVIDIA Corporation\\Global\\NGXCore", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, &key);
        if (status != NO_ERROR || !key) {
            log::info(str::format(R"(Failed to open SOFTWARE\NVIDIA Corporation\Global\NGXCore registry key: )", status));
            return;
        }

        if (RegSetValueExW(key, L"ShowDlssIndicator", 0, REG_DWORD, reinterpret_cast<const BYTE*>(&showDLSSIndicatorValue), sizeof(DWORD)) != NO_ERROR)
            log::info(str::format("Failed to set ShowDlssIndicator registry key"));

        RegCloseKey(key);
    }
}
