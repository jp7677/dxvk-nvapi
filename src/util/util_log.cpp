#include "util_log.h"
#include "util_env.h"

namespace dxvk::log {
    void write(const std::string& message) {
        constexpr auto logPathEnvName = "DXVK_NVAPI_LOG_PATH";
        static std::ofstream filestream;

        static bool alreadyInitialized = false;
        if (!alreadyInitialized) {
            auto logPath = env::getEnvVariable(logPathEnvName);
            if (!logPath.empty())
                filestream = std::ofstream(logPath + "/dxvk_nvapi.log", std::ios::app);

            alreadyInitialized = true;
        }

        std::cerr << message << std::endl;
        if (filestream)
            filestream << message << std::endl;
    }
}
