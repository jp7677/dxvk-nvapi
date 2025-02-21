#pragma once

#include "../nvapi_private.h"
#include "../inc/nvofapi/nvOpticalFlowCommon.h"
#include "util_string.h"
#include "util_log.h"

namespace dxvk {
    inline NvAPI_Status Ok() {
        return NVAPI_OK;
    }

    inline NvAPI_Status Ok(const std::string& logMessage) {
        log::info(str::format("<-", logMessage, ": OK"));
        return NVAPI_OK;
    }

    inline NvAPI_Status Ok(const std::string& logMessage, bool& alreadyLogged) {
        if (log::tracing() || !std::exchange(alreadyLogged, true))
            log::info(str::format("<-", logMessage, ": OK"));

        return NVAPI_OK;
    }

    inline NvAPI_Status Error() {
        return NVAPI_ERROR;
    }

    inline NvAPI_Status Error(const std::string& logMessage) {
        log::info(str::format("<-", logMessage, ": Error"));
        return NVAPI_ERROR;
    }

    inline NvAPI_Status Error(const std::string& logMessage, VkResult vkResult) {
        log::info(str::format("<-", logMessage, ": Error (vr: ", vkResult, ")"));
        return NVAPI_ERROR;
    }

    inline NvAPI_Status Error(const std::string& logMessage, bool& alreadyLogged) {
        if (log::tracing() || !std::exchange(alreadyLogged, true))
            log::info(str::format("<-", logMessage, ": Error"));

        return NVAPI_ERROR;
    }

    inline NvAPI_Status NoImplementation() {
        return NVAPI_NO_IMPLEMENTATION;
    }

    inline NvAPI_Status NoImplementation(const std::string& logMessage) {
        log::info(str::format("<-", logMessage, ": No implementation"));
        return NVAPI_NO_IMPLEMENTATION;
    }

    inline NvAPI_Status NoImplementation(const std::string& logMessage, bool& alreadyLogged) {
        if (log::tracing() || !std::exchange(alreadyLogged, true))
            log::info(str::format("<-", logMessage, ": No implementation"));

        return NVAPI_NO_IMPLEMENTATION;
    }

    inline NvAPI_Status EndEnumeration(const std::string& logMessage) {
        log::info(str::format("<-", logMessage, ": End enumeration"));
        return NVAPI_END_ENUMERATION;
    }

    inline NvAPI_Status ApiNotInitialized(const std::string& logMessage) {
        log::info(str::format("<-", logMessage, ": API not initialized"));
        return NVAPI_API_NOT_INTIALIZED;
    }

    inline NvAPI_Status InvalidPointer(const std::string& logMessage) {
        log::info(str::format("<-", logMessage, ": Invalid pointer"));
        return NVAPI_INVALID_POINTER;
    }

    inline NvAPI_Status InvalidArgument(const std::string& logMessage) {
        log::info(str::format("<-", logMessage, ": Invalid argument"));
        return NVAPI_INVALID_ARGUMENT;
    }

    inline NvAPI_Status ExpectedPhysicalGpuHandle(const std::string& logMessage) {
        log::info(str::format("<-", logMessage, ": Expected physical GPU handle"));
        return NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE;
    }

    inline NvAPI_Status ExpectedLogicalGpuHandle(const std::string& logMessage) {
        log::info(str::format("<-", logMessage, ": Expected logical GPU handle"));
        return NVAPI_EXPECTED_LOGICAL_GPU_HANDLE;
    }

    inline NvAPI_Status IncompatibleStructVersion(const std::string& logMessage, NvU32 version) {
        log::info(str::format("<-", logMessage, ": Incompatible struct version (", version, ")"));
        return NVAPI_INCOMPATIBLE_STRUCT_VERSION;
    }

    inline NvAPI_Status HandleInvalidated(const std::string& logMessage) {
        log::info(str::format("<-", logMessage, ": Handle invalidated"));
        return NVAPI_HANDLE_INVALIDATED;
    }

    inline NvAPI_Status HandleInvalidated(const std::string& logMessage, bool& alreadyLogged) {
        if (log::tracing() || !std::exchange(alreadyLogged, true))
            log::info(str::format("<-", logMessage, ": Handle invalidated"));

        return NVAPI_HANDLE_INVALIDATED;
    }

    inline NvAPI_Status ExpectedDisplayHandle(const std::string& logMessage) {
        log::info(str::format("<-", logMessage, ": Expected display handle"));
        return NVAPI_EXPECTED_DISPLAY_HANDLE;
    }

    inline NvAPI_Status NotSupported(const std::string& logMessage) {
        log::info(str::format("<-", logMessage, ": Not supported"));
        return NVAPI_NOT_SUPPORTED;
    }

    inline NvAPI_Status NotSupported(const std::string& logMessage, bool& alreadyLogged) {
        if (log::tracing() || !std::exchange(alreadyLogged, true))
            log::info(str::format("<-", logMessage, ": Not supported"));

        return NVAPI_NOT_SUPPORTED;
    }

    inline NvAPI_Status InvalidDisplayId(const std::string& logMessage) {
        log::info(str::format("<-", logMessage, ": Invalid display ID"));
        return NVAPI_INVALID_DISPLAY_ID;
    }

    inline NvAPI_Status MosaicNotActive(const std::string& logMessage) {
        log::info(str::format("<-", logMessage, ": Mosaic not active"));
        return NVAPI_MOSAIC_NOT_ACTIVE;
    }

    inline NvAPI_Status NvidiaDeviceNotFound(const std::string& logMessage) {
        log::info(str::format("<-", logMessage, ": NVIDIA or other suitable device not found or initialization failed"));
        return NVAPI_NVIDIA_DEVICE_NOT_FOUND;
    }

    inline NvAPI_Status ProfileNotFound(const std::string& logMessage) {
        log::info(str::format("<-", logMessage, ": Profile not found"));
        return NVAPI_PROFILE_NOT_FOUND;
    }

    inline NvAPI_Status ExecutableNotFound(const std::string& logMessage) {
        log::info(str::format("<-", logMessage, ": Executable not found"));
        return NVAPI_EXECUTABLE_NOT_FOUND;
    }

    inline NvAPI_Status SettingNotFound(const std::string& logMessage) {
        log::info(str::format("<-", logMessage, ": Setting not found"));
        return NVAPI_SETTING_NOT_FOUND;
    }

    inline NvAPI_Status InsufficientBuffer(const std::string& logMessage) {
        log::info(str::format("<-", logMessage, ": Insufficient Buffer"));
        return NVAPI_INSUFFICIENT_BUFFER;
    }

    inline NvAPI_Status NoActiveSliTopology(const std::string& logMessage, bool& alreadyLogged) {
        if (log::tracing() || !std::exchange(alreadyLogged, true))
            log::info(str::format("<-", logMessage, ": No active SLI topology"));
        return NVAPI_NO_ACTIVE_SLI_TOPOLOGY;
    }

    inline NV_OF_STATUS Success() {
        return NV_OF_SUCCESS;
    }

    inline NV_OF_STATUS Success(const std::string& logMessage) {
        log::info(str::format("<-", logMessage, ": Success"));
        return NV_OF_SUCCESS;
    }

    inline NV_OF_STATUS Success(const std::string& logMessage, bool& alreadyLogged) {
        if (log::tracing() || !std::exchange(alreadyLogged, true))
            log::info(str::format("<-", logMessage, ": Success"));

        return NV_OF_SUCCESS;
    }

    inline NV_OF_STATUS OFNotAvailable() {
        return NV_OF_ERR_OF_NOT_AVAILABLE;
    }

    inline NV_OF_STATUS OFNotAvailable(const std::string& logMessage) {
        log::info(str::format("<-", logMessage, ": OpticalFlow Not Available"));
        return NV_OF_ERR_OF_NOT_AVAILABLE;
    }

    inline NV_OF_STATUS OFNotAvailable(const std::string& logMessage, bool& alreadyLogged) {
        if (log::tracing() || !std::exchange(alreadyLogged, true))
            log::info(str::format("<-", logMessage, ": OpticalFlow Not Available"));

        return NV_OF_ERR_OF_NOT_AVAILABLE;
    }

    inline NV_OF_STATUS UnsupportedDevice() {
        return NV_OF_ERR_UNSUPPORTED_DEVICE;
    }

    inline NV_OF_STATUS UnsupportedDevice(const std::string& logMessage) {
        log::info(str::format("<-", logMessage, ": Unsupported Device"));
        return NV_OF_ERR_UNSUPPORTED_DEVICE;
    }

    inline NV_OF_STATUS UnsupportedDevice(const std::string& logMessage, bool& alreadyLogged) {
        if (log::tracing() || !std::exchange(alreadyLogged, true))
            log::info(str::format("<-", logMessage, ": Unsupported Device"));

        return NV_OF_ERR_UNSUPPORTED_DEVICE;
    }

    inline NV_OF_STATUS DeviceDoesNotExist() {
        return NV_OF_ERR_DEVICE_DOES_NOT_EXIST;
    }

    inline NV_OF_STATUS DeviceDoesNotExist(const std::string& logMessage) {
        log::info(str::format("<-", logMessage, ": Device Does Not Exist"));
        return NV_OF_ERR_DEVICE_DOES_NOT_EXIST;
    }

    inline NV_OF_STATUS DeviceDoesNotExist(const std::string& logMessage, bool& alreadyLogged) {
        if (log::tracing() || !std::exchange(alreadyLogged, true))
            log::info(str::format("<-", logMessage, ": Device Does Not Exist"));
        return NV_OF_ERR_DEVICE_DOES_NOT_EXIST;
    }

    inline NV_OF_STATUS InvalidPtr() {
        return NV_OF_ERR_INVALID_PTR;
    }

    inline NV_OF_STATUS InvalidPtr(const std::string& logMessage) {
        log::info(str::format("<-", logMessage, ": Invalid Pointer"));
        return NV_OF_ERR_INVALID_PTR;
    }

    inline NV_OF_STATUS InvalidPtr(const std::string& logMessage, bool& alreadyLogged) {
        if (log::tracing() || !std::exchange(alreadyLogged, true))
            log::info(str::format("<-", logMessage, ": Invalid Pointer"));
        return NV_OF_ERR_INVALID_PTR;
    }

    inline NV_OF_STATUS InvalidParam() {
        return NV_OF_ERR_INVALID_PARAM;
    }

    inline NV_OF_STATUS InvalidParam(const std::string& logMessage) {
        log::info(str::format("<-", logMessage, ": Invalid Parameter"));
        return NV_OF_ERR_INVALID_PARAM;
    }

    inline NV_OF_STATUS InvalidParam(const std::string& logMessage, bool& alreadyLogged) {
        if (log::tracing() || !std::exchange(alreadyLogged, true))
            log::info(str::format("<-", logMessage, ": Invalid Parameter"));
        return NV_OF_ERR_INVALID_PARAM;
    }

    inline NV_OF_STATUS InvalidCall() {
        return NV_OF_ERR_INVALID_CALL;
    }

    inline NV_OF_STATUS InvalidCall(const std::string& logMessage) {
        log::info(str::format("<-", logMessage, ": Invalid Call"));
        return NV_OF_ERR_INVALID_CALL;
    }

    inline NV_OF_STATUS InvalidCall(const std::string& logMessage, bool& alreadyLogged) {
        if (log::tracing() || !std::exchange(alreadyLogged, true))
            log::info(str::format("<-", logMessage, ": Invalid Call"));
        return NV_OF_ERR_INVALID_CALL;
    }

    inline NV_OF_STATUS InvalidVersion() {
        return NV_OF_ERR_INVALID_VERSION;
    }

    inline NV_OF_STATUS InvalidVersion(const std::string& logMessage) {
        log::info(str::format("<-", logMessage, ": Invalid Version"));
        return NV_OF_ERR_INVALID_VERSION;
    }

    inline NV_OF_STATUS InvalidVersion(const std::string& logMessage, bool& alreadyLogged) {
        if (log::tracing() || !std::exchange(alreadyLogged, true))
            log::info(str::format("<-", logMessage, ": Invalid Version"));
        return NV_OF_ERR_INVALID_VERSION;
    }

    inline NV_OF_STATUS OutOfMemory() {
        return NV_OF_ERR_OUT_OF_MEMORY;
    }

    inline NV_OF_STATUS OutOfMemory(const std::string& logMessage) {
        log::info(str::format("<-", logMessage, ": Out of Memory"));
        return NV_OF_ERR_OUT_OF_MEMORY;
    }

    inline NV_OF_STATUS OutOfMemory(const std::string& logMessage, bool& alreadyLogged) {
        if (log::tracing() || !std::exchange(alreadyLogged, true))
            log::info(str::format("<-", logMessage, ": Out of Memory"));
        return NV_OF_ERR_OUT_OF_MEMORY;
    }

    inline NV_OF_STATUS NotInitialized() {
        return NV_OF_ERR_NOT_INITIALIZED;
    }

    inline NV_OF_STATUS NotInitialized(const std::string& logMessage) {
        log::info(str::format("<-", logMessage, ": Not Initialized"));
        return NV_OF_ERR_NOT_INITIALIZED;
    }

    inline NV_OF_STATUS NotInitialized(const std::string& logMessage, bool& alreadyLogged) {
        if (log::tracing() || !std::exchange(alreadyLogged, true))
            log::info(str::format("<-", logMessage, ": Not Initialized"));
        return NV_OF_ERR_NOT_INITIALIZED;
    }

    inline NV_OF_STATUS UnsupportedFeature() {
        return NV_OF_ERR_UNSUPPORTED_FEATURE;
    }

    inline NV_OF_STATUS UnsupportedFeature(const std::string& logMessage) {
        log::info(str::format("<-", logMessage, ": Unsupported Feature"));
        return NV_OF_ERR_UNSUPPORTED_FEATURE;
    }

    inline NV_OF_STATUS UnsupportedFeature(const std::string& logMessage, bool& alreadyLogged) {
        if (log::tracing() || !std::exchange(alreadyLogged, true))
            log::info(str::format("<-", logMessage, ": Unsupported Feature"));
        return NV_OF_ERR_UNSUPPORTED_FEATURE;
    }

    inline NV_OF_STATUS ErrorGeneric() {
        return NV_OF_ERR_GENERIC;
    }

    inline NV_OF_STATUS ErrorGeneric(const std::string& logMessage) {
        log::info(str::format("<-", logMessage, ": Error"));
        return NV_OF_ERR_GENERIC;
    }

    inline NV_OF_STATUS ErrorGeneric(const std::string& logMessage, bool& alreadyLogged) {
        if (log::tracing() || !std::exchange(alreadyLogged, true))
            log::info(str::format("<-", logMessage, ": Error"));
        return NV_OF_ERR_GENERIC;
    }
}
