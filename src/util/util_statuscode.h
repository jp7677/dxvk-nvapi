#pragma once

namespace dxvk {
    inline NvAPI_Status Ok() {
        return NVAPI_OK;
    }

    inline NvAPI_Status Ok(const std::string& logMessage) {
        log::write(str::format(logMessage, ": OK"));
        return NVAPI_OK;
    }

    inline NvAPI_Status Ok(const std::string& logMessage, bool& alreadyLogged) {
        if (alreadyLogged)
            return NVAPI_OK;

        alreadyLogged = true;

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
        if (alreadyLogged)
            return NVAPI_ERROR;

        alreadyLogged = true;

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
        if (alreadyLogged)
            return NVAPI_NO_IMPLEMENTATION;

        alreadyLogged = true;

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

    inline NvAPI_Status InvalidArgument(const std::string& logMessage) {
        log::write(str::format(logMessage, ": Invalid argument"));
        return NVAPI_INVALID_ARGUMENT;
    }

    inline NvAPI_Status ExpectedPhysicalGpuHandle(const std::string& logMessage) {
        log::write(str::format(logMessage, ": Expected physical GPU handle"));
        return NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE;
    }

    inline NvAPI_Status IncompatibleStructVersion(const std::string& logMessage) {
        log::write(str::format(logMessage, ": Incompatible struct version"));
        return NVAPI_INCOMPATIBLE_STRUCT_VERSION;
    }

    inline NvAPI_Status ExpectedDisplayHandle(const std::string& logMessage) {
        log::write(str::format(logMessage, ": Expected display handle"));
        return NVAPI_EXPECTED_DISPLAY_HANDLE;
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
        log::write(str::format(logMessage, ": NVIDIA device not found"));
        return NVAPI_NVIDIA_DEVICE_NOT_FOUND;
    }
}
