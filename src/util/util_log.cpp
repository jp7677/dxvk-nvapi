#include "util_log.h"
#include "util_env.h"

namespace dxvk::log {
    void initialize(std::ofstream& filestream, bool& skipAllLogging, bool& alreadyInitialized) {
        constexpr auto logLevelEnvName = "DXVK_NVAPI_LOG_LEVEL";
        constexpr auto logPathEnvName = "DXVK_NVAPI_LOG_PATH";
        constexpr auto logFileName = "dxvk-nvapi.log";
        alreadyInitialized = true;

        auto logLevel = env::getEnvVariable(logLevelEnvName);
        if (logLevel == "full") {
            skipAllLogging = false;
            return;
        }

        auto logPath = env::getEnvVariable(logPathEnvName);
        if (logPath.empty())
            return;

        if ((*logPath.rbegin()) != '/')
            logPath += '/';

        auto fullPath = logPath + logFileName;
        filestream = std::ofstream(fullPath, std::ios::app);
        filestream << "---------- " << env::getCurrentDateTime() << " ----------" << std::endl;
        std::cerr << logPathEnvName << " is set to '" << logPath << "', appending log statements to " << fullPath << std::endl;
    }

    void write(const std::string& message) {
        static bool alreadyInitialized = false;
        static bool skipAllLogging = true;
        static std::ofstream filestream;
        if (!alreadyInitialized)
            initialize(filestream, skipAllLogging, alreadyInitialized);

        if (skipAllLogging)
            return;

        std::cerr << message << std::endl;
        if (filestream)
            filestream << message << std::endl;
    }
}
