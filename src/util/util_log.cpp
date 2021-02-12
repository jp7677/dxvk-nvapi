#include "util_log.h"
#include "util_env.h"

namespace dxvk::log {
    void write(const std::string& message) {
        constexpr auto logPathEnvName = "DXVK_NVAPI_LOG_PATH";
        constexpr auto logFileName = "dxvk_nvapi.log";
        static std::ofstream filestream;

        static bool alreadyInitialized = false;
        if (!alreadyInitialized) {
            auto logPath = env::getEnvVariable(logPathEnvName);
            if (!logPath.empty()) {
                if ((*logPath.rbegin()) != '/')
                    logPath += '/';

                filestream = std::ofstream(logPath + logFileName, std::ios::app);
            }

            alreadyInitialized = true;
        }

        std::cerr << message << std::endl;
        if (filestream)
            filestream << message << std::endl;
    }
}
