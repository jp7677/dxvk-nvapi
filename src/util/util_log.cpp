#include "util_log.h"
#include "util_env.h"
#include "util_string.h"

using PFN_wineDbgOutput = int(__cdecl*)(const char*);

namespace dxvk::log {
    constexpr auto logLevelEnvName = "DXVK_NVAPI_LOG_LEVEL";
    constexpr auto logPathEnvName = "DXVK_NVAPI_LOG_PATH";
    constexpr auto latencyMarkerLogEnvName = "DXVK_NVAPI_LATENCY_MARKER_LOG";
    constexpr auto logFileName = DXVK_NVAPI_TARGET_NAME ".log";

    static const auto logLevel = env::getEnvVariable(logLevelEnvName);
    static const auto traceEnabled = logLevel == "trace";
    static const auto latencyMarkerLogEnabled = env::getEnvVariable(latencyMarkerLogEnvName) == "1";

    static PFN_wineDbgOutput wineDbgOutput = nullptr;
    static std::mutex fileStreamMutex;

    struct Timestamp {
        LONGLONG seconds;
        LONGLONG milliseconds;
        LONGLONG microseconds;
    };

    Timestamp queryPerformanceTimestamp() {
        LARGE_INTEGER ticks, tickPerSecond;
        QueryPerformanceCounter(&ticks);
        QueryPerformanceFrequency(&tickPerSecond);

        auto seconds = ticks.QuadPart / tickPerSecond.QuadPart;
        auto remainder = ticks.QuadPart % tickPerSecond.QuadPart;
        auto milliseconds = (remainder * 1000) / tickPerSecond.QuadPart;
        auto microseconds = seconds * 1000000 + (remainder * 1000000) / tickPerSecond.QuadPart;

        return {seconds, milliseconds, microseconds};
    }

    void print(const std::string& logMessage) {
        auto line = logMessage + '\n'; // Do not flush buffers
        if (wineDbgOutput)
            wineDbgOutput(line.c_str());
        else
            std::cerr << line;
    }

    void initializeOutput() {
#if defined(_WIN32)
        static bool outputInitialized = false;
        if (std::exchange(outputInitialized, true))
            return;

        if (auto ntdllModule = ::GetModuleHandleA("ntdll.dll"))
            wineDbgOutput = reinterpret_cast<PFN_wineDbgOutput>(reinterpret_cast<void*>(GetProcAddress(ntdllModule, "__wine_dbg_output")));
#endif
    }

    void initialize(std::ofstream& filestream, bool& skipAllLogging) {
        initializeOutput();

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

    bool latencyMarkerLogging() {
        return latencyMarkerLogEnabled;
    }

    void latencyMarker(const std::string& message) {
        static bool alreadyInitialized = false;
        if (!latencyMarkerLogging())
            return;

        if (!std::exchange(alreadyInitialized, true))
            initializeOutput();

        auto timestamp = queryPerformanceTimestamp();
        auto logMessage = str::format(
            timestamp.seconds, ".",
            std::setfill('0'), std::setw(3), timestamp.milliseconds, ":",
            std::setfill('0'), std::setw(4), std::hex, ::GetCurrentProcessId(), ":",
            std::setfill('0'), std::setw(4), std::hex, ::GetCurrentThreadId(), ":",
            "latency-marker:" DXVK_NVAPI_TARGET_NAME ":",
            "qpcUs=", std::dec, timestamp.microseconds, " ",
            message);

        print(logMessage);
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

        auto timestamp = queryPerformanceTimestamp();

        auto logMessage = str::format(
            timestamp.seconds, ".",
            std::setfill('0'), std::setw(3), timestamp.milliseconds, ":",
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
