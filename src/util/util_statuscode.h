#pragma once

namespace dxvk {
    inline NvAPI_Status Ok() {
        return NVAPI_OK;
    }

    inline NvAPI_Status Ok(std::string logMessage) {
        std::cerr << logMessage << ": OK" << std::endl;
        return NVAPI_OK;
    }

    inline NvAPI_Status Error() {
        return NVAPI_ERROR;
    }

    inline NvAPI_Status Error(std::string logMessage) {
        std::cerr << logMessage << ": Error" << std::endl;
        return NVAPI_ERROR;
    }

    inline NvAPI_Status NoImplementation() {
        return NVAPI_NO_IMPLEMENTATION;
    }

    inline NvAPI_Status NoImplementation(std::string logMessage) {
        std::cerr << logMessage << ": No implementation" << std::endl;
        return NVAPI_NO_IMPLEMENTATION;
    }

    inline NvAPI_Status ApiNotInitialized(std::string logMessage) {
        std::cerr << logMessage << ": API not initialized" << std::endl;
        return NVAPI_API_NOT_INTIALIZED;
    }

    inline NvAPI_Status InvalidArgument(std::string logMessage) {
        std::cerr << logMessage << ": Invalid argument" << std::endl;
        return NVAPI_INVALID_ARGUMENT;
    }

    inline NvAPI_Status ExpectedPhysicalGpuHandle(std::string logMessage) {
        std::cerr << logMessage << ": Expected physical GPU handle" << std::endl;
        return NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE;
    }

    inline NvAPI_Status IncompatibleStructVersion(std::string logMessage) {
        std::cerr << logMessage << ": Incompatible struct version" << std::endl;
        return NVAPI_INCOMPATIBLE_STRUCT_VERSION;
    }

    inline NvAPI_Status ExpectedDisplayHandle(std::string logMessage) {
        std::cerr << logMessage << ": Expected display handle" << std::endl;
        return NVAPI_EXPECTED_DISPLAY_HANDLE;
    }

    inline NvAPI_Status InvalidDisplayId(std::string logMessage) {
        std::cerr << logMessage << ": Invalid display ID" << std::endl;
        return NVAPI_INVALID_DISPLAY_ID;
    }

    inline NvAPI_Status NvidiaDeviceNotFound(std::string logMessage) {
        std::cerr << logMessage << ": NVIDIA device not found" << std::endl;
        return NVAPI_NVIDIA_DEVICE_NOT_FOUND;
    }    
}
