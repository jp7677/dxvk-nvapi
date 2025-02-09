#include "util_drs.h"
#include "util_env.h"

namespace dxvk::drs {
    bool parsedrssetting(const std::string_view& str, NvU32& setting) {
        if (auto it = settings.find(str); it != settings.end()) {
            setting = it->second;
            return true;
        } else {
            return str::parsedword(str, setting);
        }
    }

    bool parsedrsdwordvalue(NvU32 key, const std::string_view& str, NvU32& value) {
        if (auto it = values.find(key); it != values.end()) {
            auto& values = it->second;
            if (auto it = values.find(str); it != values.end()) {
                value = it->second;
                return true;
            }
        }

        return str::parsedword(str, value);
    }

    std::unordered_map<NvU32, NvU32> parsedrsdwordsettings(const std::string& str) {
        std::unordered_map<NvU32, NvU32> result;

        if (str.empty())
            return result;

        auto entries = str::split<std::vector<std::string_view>>(str, std::regex(","));

        for (auto entry : entries) {
            auto eq = entry.find('=');

            if (eq == entry.npos || eq == 0 || eq == entry.size() - 1)
                continue;

            NvU32 key, value;

            if (parsedrssetting(entry.substr(0, eq), key) && parsedrsdwordvalue(key, entry.substr(eq + 1), value))
                result[key] = value;
        }

        return result;
    }

    std::unordered_map<NvU32, NvU32> enrichwithenv(std::unordered_map<NvU32, NvU32> map, const char* prefix) {
        for (auto& [name, key] : settings) {
            auto envname = std::string(prefix).append(name);
            auto envvalue = env::getEnvVariable(envname);

            if (envvalue.empty())
                continue;

            NvU32 value;
            if (parsedrsdwordvalue(key, envvalue, value))
                map[key] = value;
        }

        return map;
    }
}
