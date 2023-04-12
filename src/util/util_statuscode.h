#pragma once

#include "../nvapi_private.h"
#include "util_string.h"
#include "util_log.h"

namespace dxvk {
    inline NvAPI_Status Ok() {
        return NVAPI_OK;
    }

    inline NvAPI_Status Ok(const std::string& logMessage) {
        log::write(str::format(logMessage, ": OK"));
        return NVAPI_OK;
    }

    inline NvAPI_Status Ok(const std::string& logMessage, bool& alreadyLogged) {
        if (std::exchange(alreadyLogged, true))
            return NVAPI_OK;

        log::write(str::format(logMessage, ": OK"));
        return NVAPI_OK;
    }

    inline NvAPI_Status Error() {
        return NVAPI_ERROR;
    }

    inline NvAPI_Status Error(const std::string& logMessage) {
        log::write(str::format(logMessage, ": Error"));
        return NVAPI_ERROR;
    }

    inline NvAPI_Status Error(const std::string& logMessage, bool& alreadyLogged) {
        if (std::exchange(alreadyLogged, true))
            return NVAPI_ERROR;

        log::write(str::format(logMessage, ": Error"));
        return NVAPI_ERROR;
    }

    inline NvAPI_Status NoImplementation() {
        return NVAPI_NO_IMPLEMENTATION;
    }

    inline NvAPI_Status NoImplementation(const std::string& logMessage) {
        log::write(str::format(logMessage, ": No implementation"));
        return NVAPI_NO_IMPLEMENTATION;
    }

    inline NvAPI_Status NoImplementation(const std::string& logMessage, bool& alreadyLogged) {
        if (std::exchange(alreadyLogged, true))
            return NVAPI_NO_IMPLEMENTATION;

        log::write(str::format(logMessage, ": No implementation"));
        return NVAPI_NO_IMPLEMENTATION;
    }

    inline NvAPI_Status EndEnumeration(const std::string& logMessage) {
        log::write(str::format(logMessage, ": End enumeration"));
        return NVAPI_END_ENUMERATION;
    }

    inline NvAPI_Status ApiNotInitialized(const std::string& logMessage) {
        log::write(str::format(logMessage, ": API not initialized"));
        return NVAPI_API_NOT_INTIALIZED;
    }

    inline NvAPI_Status InvalidPointer(const std::string& logMessage) {
        log::write(str::format(logMessage, ": Invalid pointer"));
        return NVAPI_INVALID_POINTER;
    }

    inline NvAPI_Status InvalidArgument(const std::string& logMessage) {
        log::write(str::format(logMessage, ": Invalid argument"));
        return NVAPI_INVALID_ARGUMENT;
    }

    inline NvAPI_Status ExpectedPhysicalGpuHandle(const std::string& logMessage) {
        log::write(str::format(logMessage, ": Expected physical GPU handle"));
        return NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE;
    }

    inline NvAPI_Status ExpectedLogicalGpuHandle(const std::string& logMessage) {
        log::write(str::format(logMessage, ": Expected logical GPU handle"));
        return NVAPI_EXPECTED_LOGICAL_GPU_HANDLE;
    }

    inline NvAPI_Status IncompatibleStructVersion(const std::string& logMessage) {
        log::write(str::format(logMessage, ": Incompatible struct version"));
        return NVAPI_INCOMPATIBLE_STRUCT_VERSION;
    }

    inline NvAPI_Status HandleInvalidated(const std::string& logMessage) {
        log::write(str::format(logMessage, ": Handle invalidated"));
        return NVAPI_HANDLE_INVALIDATED;
    }

    inline NvAPI_Status HandleInvalidated(const std::string& logMessage, bool& alreadyLogged) {
        if (std::exchange(alreadyLogged, true))
            return NVAPI_HANDLE_INVALIDATED;

        log::write(str::format(logMessage, ": Handle invalidated"));
        return NVAPI_HANDLE_INVALIDATED;
    }

    inline NvAPI_Status ExpectedDisplayHandle(const std::string& logMessage) {
        log::write(str::format(logMessage, ": Expected display handle"));
        return NVAPI_EXPECTED_DISPLAY_HANDLE;
    }

    inline NvAPI_Status NotSupported(const std::string& logMessage) {
        log::write(str::format(logMessage, ": Not supported"));
        return NVAPI_NOT_SUPPORTED;
    }

    inline NvAPI_Status NotSupported(const std::string& logMessage, bool& alreadyLogged) {
        if (std::exchange(alreadyLogged, true))
            return NVAPI_NOT_SUPPORTED;

        log::write(str::format(logMessage, ": Not supported"));
        return NVAPI_NOT_SUPPORTED;
    }

    inline NvAPI_Status InvalidDisplayId(const std::string& logMessage) {
        log::write(str::format(logMessage, ": Invalid display ID"));
        return NVAPI_INVALID_DISPLAY_ID;
    }

    inline NvAPI_Status MosaicNotActive(const std::string& logMessage) {
        log::write(str::format(logMessage, ": Mosaic not active"));
        return NVAPI_MOSAIC_NOT_ACTIVE;
    }

    inline NvAPI_Status NvidiaDeviceNotFound(const std::string& logMessage) {
        log::write(str::format(logMessage, ": NVIDIA or other suitable device not found or initialization failed"));
        return NVAPI_NVIDIA_DEVICE_NOT_FOUND;
    }

    inline NvAPI_Status ProfileNotFound(const std::string& logMessage) {
        log::write(str::format(logMessage, ": Profile not found"));
        return NVAPI_PROFILE_NOT_FOUND;
    }

    inline NvAPI_Status ExecutableNotFound(const std::string& logMessage) {
        log::write(str::format(logMessage, ": Executable not found"));
        return NVAPI_EXECUTABLE_NOT_FOUND;
    }

    inline NvAPI_Status SettingNotFound(const std::string& logMessage) {
        log::write(str::format(logMessage, ": Setting not found"));
        return NVAPI_SETTING_NOT_FOUND;
    }

    inline NvAPI_Status InsufficientBuffer(const std::string& logMessage) {
        log::write(str::format(logMessage, ": Insufficient Buffer"));
        return NVAPI_INSUFFICIENT_BUFFER;
    }
}
