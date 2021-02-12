#include "util_env.h"
#include "util_string.h"

namespace dxvk::env {
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
