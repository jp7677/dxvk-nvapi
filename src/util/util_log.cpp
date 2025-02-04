#include "util_log.h"
#include "util_env.h"
#include "util_string.h"

using PFN_wineDbgOutput = int(__cdecl*)(const char*);

namespace dxvk::log {
    constexpr auto logLevelEnvName = "DXVK_NVAPI_LOG_LEVEL";
    constexpr auto logPathEnvName = "DXVK_NVAPI_LOG_PATH";
    constexpr auto logFileName = DXVK_NVAPI_TARGET_NAME ".log";

    static const auto logLevel = env::getEnvVariable(logLevelEnvName);
    static const auto traceEnabled = logLevel == "trace";

    static PFN_wineDbgOutput wineDbgOutput = nullptr;
    static std::mutex fileStreamMutex;

    void print(const std::string& logMessage) {
        auto line = logMessage + '\n'; // Do not flush buffers
        if (wineDbgOutput)
            wineDbgOutput(line.c_str());
        else
            std::cerr << line;
    }

    void initialize(std::ofstream& filestream, bool& skipAllLogging) {
#ifdef _WIN32
        if (auto ntdllModule = ::GetModuleHandleA("ntdll.dll"))
            wineDbgOutput = reinterpret_cast<PFN_wineDbgOutput>(reinterpret_cast<void*>(GetProcAddress(ntdllModule, "__wine_dbg_output")));
#endif

        if (logLevel != "info" && logLevel != "trace") {
            skipAllLogging = true;
            return;
        }

        if (traceEnabled)
            print(str::format(logLevelEnvName, " is set to 'trace', writing all log statements, this has severe impact on performance"));

        auto logPath = env::getEnvVariable(logPathEnvName);
        if (logPath.empty())
            return;

        if (*logPath.rbegin() != '/')
            logPath += '/';

        auto fullPath = logPath + logFileName;
        filestream = std::ofstream(fullPath, std::ios::app);
        filestream << "---------- " << env::getCurrentDateTime() << " ----------" << std::endl;
        print(str::format(logPathEnvName, " is set to '", logPath, "', appending log statements to ", fullPath));
    }

    bool tracing() {
        return traceEnabled;
    }

    void write(const std::string& level, const std::string& message) {
        static bool alreadyInitialized = false;
        static bool skipAllLogging = false;
        static std::ofstream filestream;
        if (!std::exchange(alreadyInitialized, true))
            initialize(filestream, skipAllLogging);

        if (skipAllLogging)
            return;

        if (level == "trace" && !tracing())
            return;

        LARGE_INTEGER ticks, tickPerSecond;
        QueryPerformanceCounter(&ticks);
        QueryPerformanceFrequency(&tickPerSecond);
        auto seconds = ticks.QuadPart / tickPerSecond.QuadPart;
        auto milliseconds = ((ticks.QuadPart % tickPerSecond.QuadPart) * 1000) / tickPerSecond.QuadPart;

        auto logMessage = str::format(
            seconds, ".",
            std::setfill('0'), std::setw(3), milliseconds, ":",
            std::setfill('0'), std::setw(4), std::hex, ::GetCurrentProcessId(), ":",
            std::setfill('0'), std::setw(4), std::hex, ::GetCurrentThreadId(), ":",
            level, ":" DXVK_NVAPI_TARGET_NAME ":",
            message);

        print(logMessage);
        if (filestream) {
            std::scoped_lock lock(fileStreamMutex);
            filestream << logMessage << std::endl;
        }
    }
}
