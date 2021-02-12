#include "util_env.h"
#include "util_string.h"

namespace dxvk::env {
    std::string getEnvVariable(const std::string& name) {
        std::vector<WCHAR> variable;
        variable.resize(MAX_PATH + 1);

        auto length = ::GetEnvironmentVariableW(str::tows(name.c_str()).c_str(), variable.data(), MAX_PATH);
        variable.resize(length);

        return str::fromws(variable.data());
    }

    std::string getExecutablePath() {
        std::vector<WCHAR> path;
        path.resize(MAX_PATH + 1);

        auto length = ::GetModuleFileNameW(nullptr, path.data(), MAX_PATH);
        path.resize(length);

        return str::fromws(path.data());
    }

    std::string getExecutableName() {
        auto path = getExecutablePath();
        auto name = path.find_last_of('\\');

        return (name != std::string::npos)
            ? path.substr(name + 1)
            : path;
    }
}
