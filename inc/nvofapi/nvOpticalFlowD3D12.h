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
* \file nvOpticalFlowD3D12.h
*   NVIDIA GPUs - Turing and above contains a hardware-based optical flow engine
*   which provides fully-accelerated hardware-based optical flow and stereo estimation.
*   nvOpticalFlowD3D12.h provides D3D12 specific enums, structure definitions and function prototypes.
* */
#ifndef _NV_OPTICALFLOW_D3D12_H_
#define _NV_OPTICALFLOW_D3D12_H_
#include "nvOpticalFlowCommon.h"
#include <d3d12.h>

#if defined(__cplusplus)
extern "C"
{
#endif /* __cplusplus */
    /** \struct NV_OF_FENCE_POINT
    * Fence and value for synchronization.
    */
    typedef struct _NV_OF_FENCE_POINT
    {
        ID3D12Fence*                    fence;                          /**< [in]: Fence object used in synchronization. */
        uint64_t                        value;                          /**< [in]: Value the fence to reach or exceed before the GPU operation or
                                                                                   Value to set the fence to after the GPU operation. */
    } NV_OF_FENCE_POINT;

    /**
    * \struct NV_OF_REGISTER_RESOURCE_PARAMS_D3D12
    * Parameters used in registering a ID3D12Resource
    */
    typedef struct _NV_OF_REGISTER_RESOURCE_PARAMS_D3D12
    {
        ID3D12Resource*                 resource;                       /**< [in]: ID3D12Resource that need to be registered. */
        NV_OF_FENCE_POINT               inputFencePoint;                /**< [in]: GPU wait until the specified fence reaches or exceeds the specified value. */
        NvOFGPUBufferHandle*            hOFGpuBuffer;                   /**< [in]: Pointer to the opaque handle of the registered resource. */
        NV_OF_FENCE_POINT               outputFencePoint;               /**< [in]: Updates the specified fence to the specified value after GPU operation. */
    } NV_OF_REGISTER_RESOURCE_PARAMS_D3D12;

    /**
    * \struct NV_OF_UNREGISTER_RESOURCE_PARAMS_D3D12
    * Parameters used in unregistering a ID3D12Resource
    */
    typedef struct _NV_OF_UNREGISTER_RESOURCE_PARAMS_D3D12
    {
        NvOFGPUBufferHandle             hOFGpuBuffer;                   /**< [in]: Registered handle of the resource that need to be unregistered. */
    } NV_OF_UNREGISTER_RESOURCE_PARAMS_D3D12;

    /**
    * \struct NV_OF_EXECUTE_INPUT_PARAMS_D3D12
    * Parameters which are sent per frame for optical flow/stereo disparity execution.
    */
    typedef struct _NV_OF_EXECUTE_INPUT_PARAMS_D3D12
    {
        NvOFGPUBufferHandle             inputFrame;                      /**< [in]: If ::NV_OF_INIT_PARAMS::mode is ::NV_OF_MODE_OPTICALFLOW, this specifies the handle to the buffer containing the input frame.
                                                                                    If ::NV_OF_INIT_PARAMS::mode is ::NV_OF_MODE_STEREODISPARITY, this specifies the handle to the buffer containing the rectified left view. */
        NvOFGPUBufferHandle             referenceFrame;                  /**< [in]: If ::NV_OF_INIT_PARAMS::mode is ::NV_OF_MODE_OPTICALFLOW, this specifies the handle to the buffer containing the reference frame.
                                                                                    If ::NV_OF_INIT_PARAMS::mode is ::NV_OF_MODE_STEREODISPARITY, this specifies the handle to the buffer containing the rectified right view. */
        NvOFGPUBufferHandle             externalHints;                   /**< [in]: It is an optional input, This field will be considered if client had set ::NV_OF_INIT_PARAMS::enableExternalHint flag.
                                                                                    Client can pass some available predictors as hints. 
                                                                                    Optical flow driver will search around those hints to optimize flow vectors quality.
                                                                                    Expected hint buffer format is ::NV_OF_FLOW_VECTOR, ::NV_OF_STEREO_DISPARITY 
                                                                                    for ::NV_OF_MODE_OPTICALFLOW, ::NV_OF_MODE_STEREODISPARITY modes respectively for
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
        uint32_t                        numFencePoints;                  /**< [in]: Number of fence points to reach or exceed before GPU operation starts.
                                                                                    Client can use different fence points for different buffers syncrhonization.  Client shall speicify all the fence points
                                                                                    used for the buffers synchronization which will be used in Optical Flow estimation. */
        NV_OF_FENCE_POINT*              fencePoint;                      /**< [in]: Pointer to an array of fence points to reach or exceed before GPU operation starts. */
    } NV_OF_EXECUTE_INPUT_PARAMS_D3D12;

    /**
    * \struct NV_OF_EXECUTE_OUTPUT_PARAMS_D3D12
    * Parameters which are received per frame for optical flow/stereo disparity execution.
    */
    typedef struct _NV_OF_EXECUTE_OUTPUT_PARAMS_D3D12
    {
        NvOFGPUBufferHandle             outputBuffer;                     /**< [in]: Specifies the pointer to optical flow or stereo disparity buffer handle.
                                                                                    ::outputBuffer will be populated with optical flow in
                                                                                    ::NV_OF_FLOW_VECTOR format or stereo disparity in 
                                                                                    ::NV_OF_STEREO_DISPARITY format for each 
                                                                                    ::NV_OF_OUTPUT_VECTOR_GRID_SIZE::outGridSize in a frame.*/
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
        NV_OF_FENCE_POINT*              fencePoint;                       /**< [in]: Pointer to a single fence point to set after GPU operation. */
    } NV_OF_EXECUTE_OUTPUT_PARAMS_D3D12;

    /**
    * \brief Create an instance of NvOFHandle object.
    *
    * This function creates an instance of NvOFHandle object and returns status.
    * Client is expected to release NvOFHandle resource using Destroy function call.
    *
    * \param [in] pD3D12Device
    *   Client must provide set this to the ID3D12Device created by the client.
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
    typedef NV_OF_STATUS(NVOFAPI* PFNNVCREATEOPTICALFLOWD3D12) (ID3D12Device* const pD3D12Device, NvOFHandle* hOFInstance);

    /**
    * \brief Populate the count of the DXGI_FORMATs supported.
    *
    * This function returns the count of the DXGI_FORMAT supported for the given NV_OF_BUFFER_USAGE
    * and the NV_OF_MODE. Client can use this count to allocate a sufficiently large array for 
    * querying the DXGI_FORMATS supported.
    *
    * \param [in] hOf
    *   Object of ::NvOFHandle type.
    * \param [in] bufUsage
    *  ::NV_OF_BUFFER_USAGE needs to be provided by the client.
    * \param [in] ofMode
    *  ::NV_OF_MODE needs to be provided by the client.
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
    typedef NV_OF_STATUS(NVOFAPI* PFNNVOFGETSURFACEFORMATCOUNTD3D12) (NvOFHandle hOf, const NV_OF_BUFFER_USAGE bufUsage, const NV_OF_MODE ofMode, uint32_t* const pCount);

    /**
    * \brief Populate the array of the DXGI_FORMATs supported.
    *
    * This function returns the array of the DXGI_FORMATs supported for the given NV_OF_BUFFER_USAGE
    * and the NV_OF_MODE. Client must populate surface format count prior to this call.
    * Client must allocate sufficiently large array size for this function call.
    *
    * \param [in] hOf
    *   Object of ::NvOFHandle type.
    * \param [in] bufUsage
    *  ::NV_OF_BUFFER_USAGE needs to be provided by the client.
    * \param [in] ofMode
    *  ::NV_OF_MODE needs to be provided by the client.
    * \param [out] pFormat
    *  The list is returned through client allocated array of DXGI_FORMAT.
    *
    * \return
    * ::NV_OF_SUCCESS \n
    * ::NV_OF_ERR_INVALID_DEVICE \n
    * ::NV_OF_ERR_DEVICE_DOES_NOT_EXIST \n
    * ::NV_OF_ERR_UNSUPPORTED_PARAM \n
    * ::NV_OF_ERR_INVALID_PARAM \n
    * ::NV_OF_ERR_GENERIC \n
    */
    typedef NV_OF_STATUS(NVOFAPI* PFNNVOFGETSURFACEFORMATD3D12) (NvOFHandle hOf, const NV_OF_BUFFER_USAGE bufUsage, const NV_OF_MODE ofMode, DXGI_FORMAT* const pFormat);

    /**
    * \brief Register the app allocated ID3D12Resource with the NvOFAPI.
    *
    * This function registers the app allocated ID3D12Resource with the NvOFAPI and gets an opaque handle NvOFGPUBufferHandle
    * in return. The client should call ::NvOFUnregisterResourceD3D12 with NvOFGPUBufferHandle to unregister the resource.
    * Client need to register the ID3D12Resource with OFAPI to be able to use it in the ::NvOFExecute call.
    *
    * \param [in] hOf
    *   Object of ::NvOFHandle type.
    * \param [in/out] registerParams
    *   pointer to the ::NV_OF_REGISTER_RESOURCE_PARAMS_D3D12 strcture.
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
    typedef NV_OF_STATUS(NVOFAPI* PFNNVOFREGISTERRESOURCED3D12) (NvOFHandle hOf, NV_OF_REGISTER_RESOURCE_PARAMS_D3D12* registerParams);

    /**
    * \brief Unregister the app allocated ID3D12Resource with the NvOFAPI.
    *
    * This function unregisters the app allocated ID3D12Resource which is already registered with the NvOFAPI.
    * Client needs to provied the NvOFGPUBufferHandle, which it has received in ::NvOFRegisterResourceD3D12 call.

    * \param [in] unregisterParams
    *   pointer to the ::NV_OF_UNREGISTER_RESOURCE_PARAMS_D3D12 strcture
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
    typedef NV_OF_STATUS(NVOFAPI* PFNNVOFUNREGISTERRESOURCED3D12) (NV_OF_UNREGISTER_RESOURCE_PARAMS_D3D12* unregisterParams);

    /**
    * \brief Kick off computation of optical flow between input and reference frame.
    *
    * This is asynchronous function call which kicks off computation of optical flow or stereo disparity
    * between ::NV_OF_EXECUTE_INPUT_PARAMS_D3D12::inputFrame and ::NV_OF_EXECUTE_INPUT_PARAMS_D3D12::referenceFrame and returns
    * after submitting  execute paramaters to optical flow engine. 
    * ::NV_OF_EXECUTE_OUTPUT_PARAMS_D3D12::outputBuffer will be populated with optical flow or stereo disparity
    * based on ::NV_OF_INIT_PARAMS:mode is NV_OF_MODE_OPTICALFLOW or NV_OF_MODE_STEREODISPARITY respectively.
    *
    * \param [in] hOf
    *   Object of ::NvOFHandle type.
    * \param [in] executeInParams
    *   pointer to the ::NV_OF_EXECUTE_INPUT_PARAMS_D3D12 structure.
    * \param [out] executeOutParams
    *   pointer to the ::NV_OF_EXECUTE_OUTPUT_PARAMS_D3D12 structure.
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
    typedef NV_OF_STATUS(NVOFAPI* PFNNVOFEXECUTED3D12) (NvOFHandle hOf, const NV_OF_EXECUTE_INPUT_PARAMS_D3D12 *executeInParams, NV_OF_EXECUTE_OUTPUT_PARAMS_D3D12 *executeOutParams);

    /*
    * Defining D3D12 specific API function pointers.
    */
    typedef struct _NV_OF_D3D12_API_FUNCTION_LIST
    {
        PFNNVCREATEOPTICALFLOWD3D12                                nvCreateOpticalFlowD3D12;
        PFNNVOFINIT                                                nvOFInit;
        PFNNVOFGETSURFACEFORMATCOUNTD3D12                          nvOFGetSurfaceFormatCountD3D12;
        PFNNVOFGETSURFACEFORMATD3D12                               nvOFGetSurfaceFormatD3D12;
        PFNNVOFREGISTERRESOURCED3D12                               nvOFRegisterResourceD3D12;
        PFNNVOFUNREGISTERRESOURCED3D12                             nvOFUnregisterResourceD3D12;
        PFNNVOFEXECUTED3D12                                        nvOFExecuteD3D12;
        PFNNVOFDESTROY                                             nvOFDestroy;
        PFNNVOFGETLASTERROR                                        nvOFGetLastError;
        PFNNVOFGETCAPS                                             nvOFGetCaps;
    } NV_OF_D3D12_API_FUNCTION_LIST;


    /**
    * \brief ::NvOFAPICreateInstanceD3D12() API is the entry point to the NvOFAPI interface.
    *
    * ::NvOFAPICreateInstanceD3D12() API populates functionList with function pointers to the API routines implemented by the
    * NvOFAPI interface.
    *
    * \return
    * ::NV_OF_SUCCESS \n
    * ::NV_OF_ERR_INVALID_VERSION \n
    * ::NV_OF_ERR_INVALID_PTR \n
    */
    NV_OF_STATUS NVOFAPI NvOFAPICreateInstanceD3D12(uint32_t apiVer, NV_OF_D3D12_API_FUNCTION_LIST* functionList);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif
