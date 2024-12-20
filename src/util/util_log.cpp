#include "util_log.h"
#include "util_env.h"
#include "util_string.h"

using PFN_wineDbgOutput = int(__cdecl*)(const char*);

static PFN_wineDbgOutput wineDbgOutput = nullptr;
static std::mutex fileStreamMutex;
static bool traceEnabled = false;

namespace dxvk::log {
    void print(const std::string& logMessage) {
        auto line = logMessage + '\n'; // Do not flush buffers
        if (wineDbgOutput)
            wineDbgOutput(line.c_str());
        else
            std::cerr << line;
    }

    void initialize(std::ofstream& filestream, bool& skipAllLogging) {
#ifdef _WIN32
        auto ntdllModule = ::GetModuleHandleA("ntdll.dll");
        if (ntdllModule != nullptr)
            wineDbgOutput = reinterpret_cast<PFN_wineDbgOutput>(reinterpret_cast<void*>(GetProcAddress(ntdllModule, "__wine_dbg_output")));
#endif

        constexpr auto logLevelEnvName = "DXVK_NVAPI_LOG_LEVEL";
        constexpr auto logPathEnvName = "DXVK_NVAPI_LOG_PATH";
        constexpr auto logFileName = DXVK_NVAPI_TARGET_NAME ".log";

        auto logLevel = env::getEnvVariable(logLevelEnvName);
        if (logLevel != "info" && logLevel != "trace") {
            skipAllLogging = true;
            return;
        }

        traceEnabled = logLevel == "trace";
        if (traceEnabled)
            print(str::format(logLevelEnvName, " is set to 'trace', writing all log statements, this has severe impact on performance"));

        auto logPath = env::getEnvVariable(logPathEnvName);
        if (logPath.empty())
            return;

        if ((*logPath.rbegin()) != '/')
            logPath += '/';

        auto fullPath = logPath + logFileName;
        filestream = std::ofstream(fullPath, std::ios::app);
        filestream << "---------- " << env::getCurrentDateTime() << " ----------" << std::endl;
        print(str::format(logPathEnvName, " is set to '", logPath, "', appending log statements to ", fullPath));
    }

    bool tracing() {
        // Before this method we need to call write() first to set `traceEnabled`, accept this
        // glitch since NvAPI_Initialize() logs initially without checking for tracing.
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
