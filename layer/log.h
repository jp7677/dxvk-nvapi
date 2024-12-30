#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#if defined(_WIN32)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <libloaderapi.h>
#include <processthreadsapi.h>
#include <profileapi.h>
#define GetProcessId GetCurrentProcessId
#define GetThreadId GetCurrentThreadId
#define PID_FORMAT "%04x"

static LARGE_INTEGER freq;

#else

#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif
#include <time.h>
#include <unistd.h>
#define GetProcessId getpid
#define GetThreadId gettid
#define PID_FORMAT "%08x"

#endif

#if !defined(LOG_CHANNEL)
#warning LOG_CHANNEL is not defined
#define LOG_CHANNEL "unknown"
#endif

enum LogLevel {
    LogLevel_None = 0,
    LogLevel_Error = 1,
    LogLevel_Warn = 2,
    LogLevel_Info = 3,
    LogLevel_Trace = 4,
    LogLevel_Debug = 5,
};

static LogLevel logLevel = LogLevel_None;
static void (*wineDbgLog)(const char*) = nullptr;

static thread_local char wineDbgLogMessageBuffer[1024];

static inline const char* LogLevelString(LogLevel level) {
    switch (level) {
        case LogLevel_Debug:
            return "debug";
        case LogLevel_Trace:
            return "trace";
        case LogLevel_Info:
            return "info";
        case LogLevel_Warn:
            return "warn";
        case LogLevel_Error:
            return "err";
        case LogLevel_None:
            return "none";
        default:
            return "?";
    }
}

struct LogTimestamp {
    int32_t seconds;
    int32_t milliseconds;
};

static inline struct LogTimestamp GetTimestamp(void) {
#ifdef _WIN32
    LARGE_INTEGER count;
    QueryPerformanceCounter(&count);
    return {
        .seconds = (int32_t)(count.QuadPart / freq.QuadPart),
        .milliseconds = (int32_t)(((count.QuadPart % freq.QuadPart) * 1000) / freq.QuadPart),
    };
#else
    struct timespec timespec;
    clock_gettime(CLOCK_MONOTONIC_RAW, &timespec);
    return {
        .seconds = (int32_t)timespec.tv_sec,
        .milliseconds = (int32_t)(timespec.tv_nsec / 1000000),
    };
#endif
}

#define LOG(level, fmt, ...)                                                                  \
    do {                                                                                      \
        if (logLevel >= level) {                                                              \
            struct LogTimestamp time = GetTimestamp();                                        \
            if (wineDbgLog) {                                                                 \
                snprintf(                                                                     \
                    wineDbgLogMessageBuffer, sizeof(wineDbgLogMessageBuffer),                 \
                    "%3u.%03u:" PID_FORMAT ":" PID_FORMAT ":%s:" LOG_CHANNEL ":%s " fmt "\n", \
                    (int)time.seconds, (int)time.milliseconds,                                \
                    (unsigned int)GetProcessId(), (unsigned int)GetThreadId(),                \
                    LogLevelString(level),                                                    \
                    __func__ __VA_OPT__(, ) __VA_ARGS__);                                     \
                wineDbgLog(wineDbgLogMessageBuffer);                                          \
            } else {                                                                          \
                fprintf(                                                                      \
                    stderr,                                                                   \
                    "%3u.%03u:" PID_FORMAT ":" PID_FORMAT ":%s:" LOG_CHANNEL ":%s " fmt "\n", \
                    (int)time.seconds, (int)time.milliseconds,                                \
                    (unsigned int)GetProcessId(), (unsigned int)GetThreadId(),                \
                    LogLevelString(level),                                                    \
                    __func__ __VA_OPT__(, ) __VA_ARGS__);                                     \
            }                                                                                 \
        }                                                                                     \
    } while (0)

#define ERR(fmt, ...) LOG(LogLevel_Error, fmt, __VA_ARGS__)
#define WARN(fmt, ...) LOG(LogLevel_Warn, fmt, __VA_ARGS__)
#define INFO(fmt, ...) LOG(LogLevel_Info, fmt, __VA_ARGS__)
#define TRACE(fmt, ...) LOG(LogLevel_Trace, fmt, __VA_ARGS__)
#define DBG(fmt, ...) LOG(LogLevel_Debug, fmt, __VA_ARGS__)

static void InitLogger(const char* logLevelEnvName) {
    const char* logLevelEnv = getenv(logLevelEnvName);

    if (logLevelEnv) {
        switch (*logLevelEnv) {
            case 'n':
                logLevel = LogLevel_None;
                break;
            case 'e':
                logLevel = LogLevel_Error;
                break;
            case 'w':
                logLevel = LogLevel_Warn;
                break;
            case 'i':
                logLevel = LogLevel_Info;
                break;
            case 't':
                logLevel = LogLevel_Trace;
                break;
            case 'd':
                logLevel = LogLevel_Debug;
                break;
            default:
                logLevel = (LogLevel)(atoi(logLevelEnv));
                break;
        }
    }

#if defined(_WIN32)
    QueryPerformanceFrequency(&freq);

    HMODULE ntdll = GetModuleHandleA("ntdll.dll");

    if (ntdll)
        *(void**)(&wineDbgLog) = (void*)GetProcAddress(ntdll, "__wine_dbg_output");
#endif
}
