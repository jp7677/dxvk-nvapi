#include "util_log.h"
#include "util_env.h"

namespace dxvk::log {
    void initialize(std::ofstream& filestream, bool& alreadyInitialized) {
        constexpr auto logPathEnvName = "DXVK_NVAPI_LOG_PATH";
        constexpr auto logFileName = "dxvk_nvapi.log";

        alreadyInitialized = true;
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
        static std::ofstream filestream;
        static bool alreadyInitialized = false;
        if (!alreadyInitialized)
            initialize(filestream, alreadyInitialized);

        std::cerr << message << std::endl;
        if (filestream)
            filestream << message << std::endl;
    }
}
