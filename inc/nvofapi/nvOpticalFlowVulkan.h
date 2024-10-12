/*
 * SPDX-FileCopyrightText: Copyright (c) 2018-2024 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
/**
* \file nvOpticalFlowVulkan.h
*   NVIDIA GPUs - Turing and above contains a hardware-based optical flow engine
*   which provides fully-accelerated hardware-based optical flow.
*   nvOpticalFlowVulkan.h provides Vulkan specific enums, structure definitions and function prototypes.
*   Vulkan optical flow is not supported for the WSL(Window subsystem for Linux) architecture.
*   Vulkan optical flow is not supported for the Stereo disparity mode.
*   Vulkan optical flow is not supported on the Turing GPU.
* */
#ifndef _NV_OPTICALFLOW_VULKAN_H_
#define _NV_OPTICALFLOW_VULKAN_H_
#include "nvOpticalFlowCommon.h"
#include <vulkan/vulkan.h>
#ifdef _WIN32
#include <vulkan/vulkan_win32.h>
#endif

#if defined(__cplusplus)
extern "C"
{
#endif /* __cplusplus */
    /** \struct NV_OF_SYNC_VK
    * Semaphore and value for synchronization.
    */
    typedef struct _NV_OF_SYNC_VK
    {
        VkSemaphore                     semaphore;                      /**< [in]: Timeline semaphore object used in synchronization. */
        uint64_t                        value;                          /**< [in]: Value the fence to reach or exceed before the GPU operation or
                                                                                   Value to set the fence to after the GPU operation. */
    } NV_OF_SYNC_VK;

    /**
    * \struct NV_OF_REGISTER_RESOURCE_PARAMS_VK
    * Parameters used in registering a Vulkan resource
    */
    typedef struct _NV_OF_REGISTER_RESOURCE_PARAMS_VK
    {
		VkImage                         image;                          /**< [in]: Vulkan resource that need to be registered. */
		VkFormat                        format;                         /**< [in]: Vulkan resource format. */
        NvOFGPUBufferHandle*            hOFGpuBuffer;                   /**< [in]: Pointer to the opaque handle of the registered resource. */
    } NV_OF_REGISTER_RESOURCE_PARAMS_VK;

    /**
    * \struct NV_OF_UNREGISTER_RESOURCE_PARAMS_VK
    * Parameters used in unregistering a Vulkan resource
    */
    typedef struct _NV_OF_UNREGISTER_RESOURCE_PARAMS_VK
    {
        NvOFGPUBufferHandle             hOFGpuBuffer;                   /**< [in]: Registered handle of the resource that need to be unregistered. */
    } NV_OF_UNREGISTER_RESOURCE_PARAMS_VK;

    /**
    * \struct NV_OF_EXECUTE_INPUT_PARAMS_VK
    * Parameters which are sent per frame for optical flow execution.
    */
    typedef struct _NV_OF_EXECUTE_INPUT_PARAMS_VK
    {
        NvOFGPUBufferHandle             inputFrame;                      /**< [in]: If ::NV_OF_INIT_PARAMS::mode is ::NV_OF_MODE_OPTICALFLOW, this specifies the handle to the buffer containing the input frame. */
        NvOFGPUBufferHandle             referenceFrame;                  /**< [in]: If ::NV_OF_INIT_PARAMS::mode is ::NV_OF_MODE_OPTICALFLOW, this specifies the handle to the buffer containing the reference frame. */
        NvOFGPUBufferHandle             externalHints;                   /**< [in]: It is an optional input, This field will be considered if client had set ::NV_OF_INIT_PARAMS::enableExternalHint flag.
                                                                                    Client can pass some available predictors as hints. 
                                                                                    Optical flow driver will search around those hints to optimize flow vectors quality.
                                                                                    Expected hint buffer format is ::NV_OF_FLOW_VECTOR for ::NV_OF_MODE_OPTICALFLOW
                                                                                    each ::NV_OF_INIT_PARAMS::hintGridSize in a frame. */
        NV_OF_BOOL                      disableTemporalHints;            /**< [in]: Temporal hints yield better accuracy flow vectors when running on successive frames of a continuous video (without major scene changes).
                                                                                    When disableTemporalHints = 0, optical flow vectors from previous NvOFExecute call are automatically used as hints for the current NvOFExecute call.
                                                                                    However, when running optical flow on pairs of images which are completely independent of each other, temporal hints are useless
                                                                                    and in fact, they will degrade the quality. Therefore, it is recommended to set disableTemporalHints = 1 in this case.*/
        uint32_t                        padding;                         /**< [in]: Padding.  Must be set to 0. */
        NvOFPrivDataHandle              hPrivData;                       /**< [in]: Optical flow private data handle. It is reserved field and should be set to NULL. */
        uint32_t                        padding2;                        /**< [in]: Padding.  Must be set to 0. */
        uint32_t                        numRois;                         /**< [in]: Number of ROIs. */
        NV_OF_ROI_RECT*                 roiData;                         /**< [in]: Pointer to the NV_OF_ROI_RECTs data.  Size of this buffer should be atleast numROIs * sizeof(NV_OF_ROI_RECT). */
        uint32_t                        padding3;                        /**< [in]: Padding.  Must be set to 0. */
        uint32_t                        numWaitSyncs;                    /**< [in]: Number of semaphores to reach or exceed before GPU operation starts.
                                                                                    Client can use different fence points for different buffers syncrhonization.  Client shall speicify all the fence points
                                                                                    used for the buffers synchronization which will be used in Optical Flow estimation. */
        NV_OF_SYNC_VK*                  pWaitSyncs;                      /**< [in]: Pointer to an array of semaphores to reach or exceed before GPU operation starts. */
    } NV_OF_EXECUTE_INPUT_PARAMS_VK;

    /**
    * \struct NV_OF_EXECUTE_OUTPUT_PARAMS_VK
    * Parameters which are received per frame for optical flow/stereo disparity execution.
    */
    typedef struct _NV_OF_EXECUTE_OUTPUT_PARAMS_VK
    {
        NvOFGPUBufferHandle             outputBuffer;                     /**< [in]: Specifies the pointer to optical flow buffer handle.
                                                                                    ::outputBuffer will be populated with optical flow in
                                                                                    ::NV_OF_FLOW_VECTOR format for each ::NV_OF_OUTPUT_VECTOR_GRID_SIZE::outGridSize in a frame.*/
        NvOFGPUBufferHandle             outputCostBuffer;                 /**< [in]: Specifies the pointer to output cost calculation buffer handle. */
        NvOFPrivDataHandle              hPrivData;                        /**< [in]: Optical flow private data handle. It is reserved field and should be set to NULL. */
        NvOFGPUBufferHandle             bwdOutputBuffer;                  /**< [in]: When ::NV_OF_INIT_PARAMS::predDirection is ::NV_OF_PRED_DIRECTION_BOTH,
                                                                                    this specifies the pointer to backward optical flow buffer handle.
                                                                                    ::bwOutputBuffer will be populated with optical flow in ::NV_OF_FLOW_VECTOR format
                                                                                    for each ::NV_OF_VECTOR_GRID_SIZE::outGidSize in a frame. */
        NvOFGPUBufferHandle             bwdOutputCostBuffer;              /**< [in]: When ::NV_OF_INIT_PARAMS::predDirection is ::NV_OF_PRED_DIRECTION_BOTH and
                                                                                    ::NV_OF_INIT_PARAMS::enableOutputCost is NV_TRUE, this specifies the pointer
                                                                                    to the backward output cost calculation buffer handle. */
        NvOFGPUBufferHandle             globalFlowBuffer;                 /**< [in]: When ::NV_OF_INIT_PARAMS::enableGlobalFlow is set to NV_true, this specifies the
                                                                                    pointer to the global flow buffer handle. ::globalFlowBuffer will be populated in
                                                                                    ::NV_OF_FLOW_VECTOR format. */
        uint32_t                        padding[2];                       /**< [in]: Reserved.  Must be set to 0. */
        NV_OF_SYNC_VK*                  pSignalSync;                      /**< [in]: Pointer to a single semaphore to signal after GPU operation. */
    } NV_OF_EXECUTE_OUTPUT_PARAMS_VK;

    /**
    * \brief Create an instance of NvOFHandle object.
    *
    * This function creates an instance of NvOFHandle object and returns status.
    * Client is expected to release NvOFHandle resource using Destroy function call.
    *
    * \param [in] instance
    *   Client must set this to the vulkan instance used by the client.
    * \param [in] physicalDevice
    *   Client must set this to the vulkan physical device used by the client.
    * \param [in] device
    *   Client must set this to the vulkan device created by the client.
    * \param [out] NvOFHandle*
    *   Pointer of class ::NvOFHandle object.
    *
    * \return
    * ::NV_OF_SUCCESS \n
    * ::NV_OF_ERR_OUT_OF_MEMORY \n
    * ::NV_OF_ERR_INVALID_VERSION \n
    * ::NV_OF_ERR_UNSUPPORTED_PARAM \n
    * ::NV_OF_ERR_OF_NOT_AVAILABLE \n
    */
    typedef NV_OF_STATUS(NVOFAPI* PFNNVCREATEOPTICALFLOWVK) (VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device, NvOFHandle* hOFInstance);

    /**
    * \brief Populate the count of the VkFormat's supported.
    *
    * This function returns the count of the VkFormat's supported for the given NV_OF_BUFFER_USAGE
    * and the NV_OF_MODE_OPTICALFLOW mode. Client can use this count to allocate a sufficiently large array for 
    * querying the VkFormat's supported.
    *
    * \param [in] hOf
    *   Object of ::NvOFHandle type.
    * \param [in] bufUsage
    *  ::NV_OF_BUFFER_USAGE needs to be provided by the client.
    * \param [in] ofMode
    *  ::NV_OF_MODE needs to be provided by the client. Note that NV_OF_MODE_STEREODISPARITY mode is not supported for the Vulkan.
    * \param [out] pCount
    *  The count is returned by uint32_t* pCount
    *
    * \return
    * ::NV_OF_SUCCESS \n
    * ::NV_OF_ERR_INVALID_DEVICE \n
    * ::NV_OF_ERR_DEVICE_DOES_NOT_EXIST \n
    * ::NV_OF_ERR_UNSUPPORTED_PARAM \n
    * ::NV_OF_ERR_INVALID_PARAM \n
    * ::NV_OF_ERR_GENERIC \n
    */
    typedef NV_OF_STATUS(NVOFAPI* PFNNVOFGETSURFACEFORMATCOUNTVK) (NvOFHandle hOf, const NV_OF_BUFFER_USAGE bufUsage, const NV_OF_MODE ofMode, uint32_t* const pCount);

    /**
    * \brief Populate the array of the VkFormat's supported.
    *
    * This function returns the array of the VkFormat's supported for the given NV_OF_BUFFER_USAGE and ::NV_OF_MODE_OPTICALFLOW mode.
    * Client must populate surface format count prior to this call.
    * Client must allocate sufficiently large array size for this function call.
    *
    * \param [in] hOf
    *   Object of ::NvOFHandle type.
    * \param [in] bufUsage
    *  ::NV_OF_BUFFER_USAGE needs to be provided by the client.
    * \param [in] ofMode
    *  ::NV_OF_MODE needs to be provided by the client. Note that NV_OF_MODE_STEREODISPARITY mode is not supported for the Vulkan.
    * \param [out] pFormat
    *  The list is returned through client allocated array of VkFormat's.
    *
    * \return
    * ::NV_OF_SUCCESS \n
    * ::NV_OF_ERR_INVALID_DEVICE \n
    * ::NV_OF_ERR_DEVICE_DOES_NOT_EXIST \n
    * ::NV_OF_ERR_UNSUPPORTED_PARAM \n
    * ::NV_OF_ERR_INVALID_PARAM \n
    * ::NV_OF_ERR_GENERIC \n
    */
    typedef NV_OF_STATUS(NVOFAPI* PFNNVOFGETSURFACEFORMATVK) (NvOFHandle hOf, const NV_OF_BUFFER_USAGE bufUsage, const NV_OF_MODE ofMode, VkFormat* const pFormat);

    /**
    * \brief Register the app allocated vulkan resource with the NvOFAPI.
    *
    * This function registers the app allocated vulkan resource with the NvOFAPI and gets an opaque handle NvOFGPUBufferHandle
    * in return. The client should call ::NvOFUnregisterResourceVk with NvOFGPUBufferHandle to unregister the resource.
    * Client need to register the vulkan resource with OFAPI to be able to use it in the ::NvOFExecute call.
    *
    * \param [in] hOf
    *   Object of ::NvOFHandle type.
    * \param [in/out] registerParams
    *   pointer to the ::NV_OF_REGISTER_RESOURCE_PARAMS_VK structure.
    *
    * \return
    * ::NV_OF_SUCCESS \n
    * ::NV_OF_ERR_INVALID_DEVICE \n
    * ::NV_OF_ERR_DEVICE_DOES_NOT_EXIST \n
    * ::NV_OF_ERR_UNSUPPORTED_PARAM \n
    * ::NV_OF_ERR_INVALID_PARAM \n
    * ::NV_OF_ERR_OUT_OF_MEMORY \n
    * ::NV_OF_ERR_GENERIC \n
    */
    typedef NV_OF_STATUS(NVOFAPI* PFNNVOFREGISTERRESOURCEVK) (NvOFHandle hOf, NV_OF_REGISTER_RESOURCE_PARAMS_VK* registerParams);

    /**
    * \brief Unregister the app allocated vulkan resource with the NvOFAPI.
    *
    * This function unregisters the app allocated vulkan resource which is already registered with the NvOFAPI.
    * Client needs to provied the NvOFGPUBufferHandle, which it has received in ::NvOFRegisterResourceVk call.

    * \param [in] unregisterParams
    *   pointer to the ::NV_OF_UNREGISTER_RESOURCE_PARAMS_VK strcture
    *
    * \return
    * ::NV_OF_SUCCESS \n
    * ::NV_OF_ERR_INVALID_DEVICE \n
    * ::NV_OF_ERR_DEVICE_DOES_NOT_EXIST \n
    * ::NV_OF_ERR_UNSUPPORTED_PARAM \n
    * ::NV_OF_ERR_INVALID_PARAM \n
    * ::NV_OF_ERR_OUT_OF_MEMORY \n
    * ::NV_OF_ERR_GENERIC \n
    */
    typedef NV_OF_STATUS(NVOFAPI* PFNNVOFUNREGISTERRESOURCEVK) (NV_OF_UNREGISTER_RESOURCE_PARAMS_VK* unregisterParams);

    /**
    * \brief Kick off computation of optical flow between input and reference frame.
    *
    * This is asynchronous function call which kicks off computation of optical flow.
    * between ::NV_OF_EXECUTE_INPUT_PARAMS_VK::inputFrame and ::NV_OF_EXECUTE_INPUT_PARAMS_VK::referenceFrame and returns
    * after submitting  execute paramaters to optical flow engine. 
    * ::NV_OF_EXECUTE_OUTPUT_PARAMS_VK::outputBuffer will be populated with optical flow.
    * based on ::NV_OF_INIT_PARAMS:mode is NV_OF_MODE_OPTICALFLOW.
    *
    * \param [in] hOf
    *   Object of ::NvOFHandle type.
    * \param [in] executeInParams
    *   pointer to the ::NV_OF_EXECUTE_INPUT_PARAMS_VK structure.
    * \param [out] executeOutParams
    *   pointer to the ::NV_OF_EXECUTE_OUTPUT_PARAMS_VK structure.
    *
    * \return
    * ::NV_OF_SUCCESS \n
    * ::NV_OF_ERR_INVALID_PTR \n
    * ::NV_OF_ERR_INVALID_DEVICE \n
    * ::NV_OF_ERR_DEVICE_DOES_NOT_EXIST \n
    * ::NV_OF_ERR_UNSUPPORTED_PARAM \n
    * ::NV_OF_ERR_OUT_OF_MEMORY \n
    * ::NV_OF_ERR_INVALID_PARAM \n
    * ::NV_OF_ERR_INVALID_VERSION \n
    * ::NV_OF_ERR_OF_NOT_INITIALIZED \n
    * ::NV_OF_ERR_GENERIC \n
    */
    typedef NV_OF_STATUS(NVOFAPI* PFNNVOFEXECUTEVK) (NvOFHandle hOf, const NV_OF_EXECUTE_INPUT_PARAMS_VK *executeInParams, NV_OF_EXECUTE_OUTPUT_PARAMS_VK *executeOutParams);

    /*
    * Defining Vulkan specific API function pointers.
    */
    typedef struct _NV_OF_VK_API_FUNCTION_LIST
    {
        PFNNVCREATEOPTICALFLOWVK                                   nvCreateOpticalFlowVk;
        PFNNVOFINIT                                                nvOFInit;
        PFNNVOFGETSURFACEFORMATCOUNTVK                             nvOFGetSurfaceFormatCountVk;
        PFNNVOFGETSURFACEFORMATVK                                  nvOFGetSurfaceFormatVk;
        PFNNVOFREGISTERRESOURCEVK                                  nvOFRegisterResourceVk;
        PFNNVOFUNREGISTERRESOURCEVK                                nvOFUnregisterResourceVk;
        PFNNVOFEXECUTEVK                                           nvOFExecuteVk;
        PFNNVOFDESTROY                                             nvOFDestroy;
        PFNNVOFGETLASTERROR                                        nvOFGetLastError;
        PFNNVOFGETCAPS                                             nvOFGetCaps;
    } NV_OF_VK_API_FUNCTION_LIST;

    /**
    * \brief ::NvOFAPICreateInstanceVk() API is the entry point to the NvOFAPI interface.
    *
    * ::NvOFAPICreateInstanceVk() API populates functionList with function pointers to the API routines implemented by the
    * NvOFAPI interface.
    *
    * \return
    * ::NV_OF_SUCCESS \n
    * ::NV_OF_ERR_INVALID_VERSION \n
    * ::NV_OF_ERR_INVALID_PTR \n
    */

	NV_OF_STATUS NvOFAPICreateInstanceVk(uint32_t apiVer, NV_OF_VK_API_FUNCTION_LIST* functionList);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif
