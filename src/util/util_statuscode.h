#pragma once

#include "../nvapi_private.h"

namespace dxvk {
    inline NvAPI_Status Ok() {
        return NVAPI_OK;
    }

    inline NvAPI_Status Ok(const std::string& logMessage) {
        std::cerr << logMessage << ": OK" << std::endl;
        return NVAPI_OK;
    }

    inline NvAPI_Status Ok(const std::string& logMessage, bool& alreadyLogged) {
        if (alreadyLogged)
            return NVAPI_OK;

        alreadyLogged = true;

        std::cerr << logMessage << ": OK" << std::endl;
        return NVAPI_OK;
    }

    inline NvAPI_Status Error() {
        return NVAPI_ERROR;
    }

    inline NvAPI_Status Error(const std::string& logMessage) {
        std::cerr << logMessage << ": Error" << std::endl;
        return NVAPI_ERROR;
    }

    inline NvAPI_Status Error(const std::string& logMessage, bool& alreadyLogged) {
        if (alreadyLogged)
            return NVAPI_ERROR;

        alreadyLogged = true;

        std::cerr << logMessage << ": Error" << std::endl;
        return NVAPI_ERROR;
    }

    inline NvAPI_Status NoImplementation() {
        return NVAPI_NO_IMPLEMENTATION;
    }

    inline NvAPI_Status NoImplementation(const std::string& logMessage) {
        std::cerr << logMessage << ": No implementation" << std::endl;
        return NVAPI_NO_IMPLEMENTATION;
    }

    inline NvAPI_Status NoImplementation(const std::string& logMessage, bool& alreadyLogged) {
        if (alreadyLogged)
            return NVAPI_NO_IMPLEMENTATION;

        alreadyLogged = true;

        std::cerr << logMessage << ": No implementation" << std::endl;
        return NVAPI_NO_IMPLEMENTATION;
    }

    inline NvAPI_Status EndEnumeration(const std::string& logMessage) {
        std::cerr << logMessage << ": End enumeration" << std::endl;
        return NVAPI_END_ENUMERATION;
    }

    inline NvAPI_Status ApiNotInitialized(const std::string& logMessage) {
        std::cerr << logMessage << ": API not initialized" << std::endl;
        return NVAPI_API_NOT_INTIALIZED;
    }

    inline NvAPI_Status InvalidArgument(const std::string& logMessage) {
        std::cerr << logMessage << ": Invalid argument" << std::endl;
        return NVAPI_INVALID_ARGUMENT;
    }

    inline NvAPI_Status ExpectedPhysicalGpuHandle(const std::string& logMessage) {
        std::cerr << logMessage << ": Expected physical GPU handle" << std::endl;
        return NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE;
    }

    inline NvAPI_Status IncompatibleStructVersion(const std::string& logMessage) {
        std::cerr << logMessage << ": Incompatible struct version" << std::endl;
        return NVAPI_INCOMPATIBLE_STRUCT_VERSION;
    }

    inline NvAPI_Status ExpectedDisplayHandle(const std::string& logMessage) {
        std::cerr << logMessage << ": Expected display handle" << std::endl;
        return NVAPI_EXPECTED_DISPLAY_HANDLE;
    }

    inline NvAPI_Status InvalidDisplayId(const std::string& logMessage) {
        std::cerr << logMessage << ": Invalid display ID" << std::endl;
        return NVAPI_INVALID_DISPLAY_ID;
    }

    inline NvAPI_Status MosaicNotActive(const std::string& logMessage) {
        std::cerr << logMessage << ": Mosaic not active" << std::endl;
        return NVAPI_MOSAIC_NOT_ACTIVE;
    }

    inline NvAPI_Status NvidiaDeviceNotFound(const std::string& logMessage) {
        std::cerr << logMessage << ": NVIDIA device not found" << std::endl;
        return NVAPI_NVIDIA_DEVICE_NOT_FOUND;
    }
}
