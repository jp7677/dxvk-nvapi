#include "util_env.h"
#include "util_string.h"

namespace dxvk::env {
    std::string getEnvVar(const std::string& name) {
        std::vector<WCHAR> result;
        result.resize(MAX_PATH + 1);

        DWORD len = ::GetEnvironmentVariableW(str::tows(name.c_str()).c_str(), result.data(), MAX_PATH);
        result.resize(len);

        return str::fromws(result.data());
    }

    std::string getExePath() {
        std::vector<WCHAR> exePath;
        exePath.resize(MAX_PATH + 1);

        DWORD length = ::GetModuleFileNameW(nullptr, exePath.data(), MAX_PATH);
        exePath.resize(length);

        return str::fromws(exePath.data());
    }

    std::string getExeName() {
        std::string fullPath = getExePath();
        auto name = fullPath.find_last_of('\\');

        return (name != std::string::npos)
            ? fullPath.substr(name + 1)
            : fullPath;
    }
}
