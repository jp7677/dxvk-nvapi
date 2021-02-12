#include "util_log.h"
#include "util_env.h"
#include "util_string.h"

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

                auto fullPath = logPath + logFileName;
                filestream = std::ofstream(fullPath, std::ios::app);
                std::cerr << str::format(logPathEnvName, " is set to '", logPath,"', appending log statements to ", fullPath) << std::endl;
            }

            alreadyInitialized = true;
        }

        std::cerr << message << std::endl;
        if (filestream)
            filestream << message << std::endl;
    }
}
