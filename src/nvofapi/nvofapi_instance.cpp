/*
 * SPDX-FileCopyrightText: Copyright (c) 2024 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
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

#include "nvofapi_image.h"
#include "nvofapi_instance.h"
#include "../util/util_log.h"

namespace dxvk {

    typedef struct NV_OF_PRIV_DATA {
        uint32_t size;
        uint32_t id;
        void* data;
    } NV_OF_PRIV_DATA;

    typedef struct NV_OF_EXECUTE_PRIV_DATA_INPUT_MIPS {
        NvOFGPUBufferHandle input[6];
        NvOFGPUBufferHandle reference[6];
        uint8_t reserved[100];
    } NV_OF_EXECUTE_PRIV_DATA_INPUT_MIPS;

    constexpr uint32_t NV_OF_EXECUTE_PRIV_DATA_ID_INPUT_MIPS = 6;

    uint32_t NvOFInstance::GetVkOFAQueue() const {
        uint32_t count = 0;
        m_vkGetPhysicalDeviceQueueFamilyProperties(m_vkPhysicalDevice, &count, nullptr);
        auto queueFamProps = std::vector<VkQueueFamilyProperties>(count);
        m_vkGetPhysicalDeviceQueueFamilyProperties(m_vkPhysicalDevice, &count, queueFamProps.data());

        for (uint32_t i = 0; i < count; i++) {
            if (queueFamProps[i].queueFlags & VK_QUEUE_OPTICAL_FLOW_BIT_NV) {
                return i;
            }
        }
        return -1;
    }

    namespace {
        VkOpticalFlowGridSizeFlagBitsNV ToVkOutputGridSize(NV_OF_OUTPUT_VECTOR_GRID_SIZE grid) {
            switch (grid) {
                case NV_OF_OUTPUT_VECTOR_GRID_SIZE_1:
                    return VK_OPTICAL_FLOW_GRID_SIZE_1X1_BIT_NV;
                case NV_OF_OUTPUT_VECTOR_GRID_SIZE_2:
                    return VK_OPTICAL_FLOW_GRID_SIZE_2X2_BIT_NV;
                case NV_OF_OUTPUT_VECTOR_GRID_SIZE_4:
                default:
                    return VK_OPTICAL_FLOW_GRID_SIZE_4X4_BIT_NV;
            }
        }

        VkOpticalFlowGridSizeFlagBitsNV ToVkHintGridSize(NV_OF_HINT_VECTOR_GRID_SIZE grid) {
            switch (grid) {
                case NV_OF_HINT_VECTOR_GRID_SIZE_1:
                    return VK_OPTICAL_FLOW_GRID_SIZE_1X1_BIT_NV;
                case NV_OF_HINT_VECTOR_GRID_SIZE_2:
                    return VK_OPTICAL_FLOW_GRID_SIZE_2X2_BIT_NV;
                case NV_OF_HINT_VECTOR_GRID_SIZE_4:
                    return VK_OPTICAL_FLOW_GRID_SIZE_4X4_BIT_NV;
                case NV_OF_HINT_VECTOR_GRID_SIZE_8:
                    return VK_OPTICAL_FLOW_GRID_SIZE_8X8_BIT_NV;
                default:
                    return VK_OPTICAL_FLOW_GRID_SIZE_4X4_BIT_NV;
            }
        }
    }

    bool NvOFInstance::QueryOpticalFlowProperties(VkPhysicalDeviceOpticalFlowPropertiesNV& props) const {
        if (!m_vk || !m_vk->IsAvailable() || !m_vkPhysicalDevice)
            return false;

        props = {};
        props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPTICAL_FLOW_PROPERTIES_NV;

        VkPhysicalDeviceProperties2 props2{};
        props2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        props2.pNext = &props;

        m_vk->GetPhysicalDeviceProperties2(m_vkInstance, m_vkPhysicalDevice, &props2);
        return true;
    }

    NV_OF_STATUS NvOFInstance::InitSession(const NV_OF_INIT_PARAMS* initParams) {
        static constexpr auto n = FUNC;

        if (!initParams)
            return NV_OF_ERR_INVALID_PTR;

        if (log::tracing())
            log::trace(n,
                initParams->width,
                initParams->height,
                initParams->outGridSize,
                initParams->hintGridSize,
                initParams->mode,
                initParams->perfLevel,
                initParams->enableExternalHints,
                initParams->enableOutputCost,
                log::fmt::hnd(initParams->hPrivData),
                initParams->enableRoi,
                initParams->predDirection,
                initParams->enableGlobalFlow,
                initParams->inputBufferFormat);

        VkOpticalFlowSessionCreateInfoNV createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_OPTICAL_FLOW_SESSION_CREATE_INFO_NV;
        createInfo.width = initParams->width;
        createInfo.height = initParams->height;
        createInfo.outputGridSize = ToVkOutputGridSize(initParams->outGridSize);

        if (!initParams->hPrivData) {
            createInfo.flowVectorFormat = VK_FORMAT_R16G16_SINT;
        }
        if (initParams->enableOutputCost == NV_OF_TRUE) {
            createInfo.flags |= VK_OPTICAL_FLOW_SESSION_CREATE_ENABLE_COST_BIT_NV;
            createInfo.costFormat = VK_FORMAT_R8_UINT;
        }
        if (initParams->predDirection == NV_OF_PRED_DIRECTION_BOTH) {
            createInfo.flags |= VK_OPTICAL_FLOW_SESSION_CREATE_BOTH_DIRECTIONS_BIT_NV;
        }
        if (initParams->enableGlobalFlow == NV_OF_TRUE) {
            createInfo.flags |= VK_OPTICAL_FLOW_SESSION_CREATE_ENABLE_GLOBAL_FLOW_BIT_NV;
        }
        if (initParams->enableRoi == NV_OF_TRUE) {
            createInfo.flags |= VK_OPTICAL_FLOW_SESSION_CREATE_ALLOW_REGIONS_BIT_NV;
        }
        if (initParams->enableExternalHints == NV_OF_TRUE) {
            createInfo.flags |= VK_OPTICAL_FLOW_SESSION_CREATE_ENABLE_HINT_BIT_NV;
            createInfo.hintGridSize = ToVkHintGridSize(initParams->hintGridSize);
        }

        switch (initParams->perfLevel) {
            case NV_OF_PERF_LEVEL_SLOW:
                createInfo.performanceLevel = VK_OPTICAL_FLOW_PERFORMANCE_LEVEL_SLOW_NV;
                break;
            case NV_OF_PERF_LEVEL_MEDIUM:
                createInfo.performanceLevel = VK_OPTICAL_FLOW_PERFORMANCE_LEVEL_MEDIUM_NV;
                break;
            case NV_OF_PERF_LEVEL_FAST:
                createInfo.performanceLevel = VK_OPTICAL_FLOW_PERFORMANCE_LEVEL_FAST_NV;
                break;
            default:
                createInfo.performanceLevel = VK_OPTICAL_FLOW_PERFORMANCE_LEVEL_UNKNOWN_NV;
                break;
        }

        switch (initParams->inputBufferFormat) {
            case NV_OF_BUFFER_FORMAT_GRAYSCALE8:
                createInfo.imageFormat = VK_FORMAT_R8_UNORM;
                break;
            case NV_OF_BUFFER_FORMAT_NV12:
                createInfo.imageFormat = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
                break;
            case NV_OF_BUFFER_FORMAT_ABGR8:
                createInfo.imageFormat = VK_FORMAT_B8G8R8A8_UNORM;
                break;
            default:
                createInfo.imageFormat = VK_FORMAT_UNDEFINED;
                break;
        }

        // Need to get the size/id for the private data to pass it along to VK...
        VkOpticalFlowSessionCreatePrivateDataInfoNV privData{};

        if (initParams->hPrivData) {
            privData.sType = VK_STRUCTURE_TYPE_OPTICAL_FLOW_SESSION_CREATE_PRIVATE_DATA_INFO_NV;
            privData.size = reinterpret_cast<NV_OF_PRIV_DATA*>(initParams->hPrivData)->size;
            privData.id = reinterpret_cast<NV_OF_PRIV_DATA*>(initParams->hPrivData)->id;
            privData.pPrivateData = reinterpret_cast<NV_OF_PRIV_DATA*>(initParams->hPrivData)->data;

            createInfo.pNext = &privData;
        }

        auto ret = m_vkCreateOpticalFlowSessionNV(m_vkDevice, &createInfo, nullptr, &m_vkOfaSession);

        if (ret == VK_SUCCESS) {
            return NV_OF_SUCCESS;
        }

        return NV_OF_ERR_GENERIC;
    }

    NV_OF_STATUS NvOFInstance::BindImageToSession(NvOFGPUBufferHandle hBuffer, VkOpticalFlowSessionBindingPointNV bindingPoint) const {
        auto nvOFImage = reinterpret_cast<NvOFImage*>(hBuffer);

        if (!nvOFImage)
            return NV_OF_ERR_GENERIC;

        auto ret = m_vkBindOpticalFlowSessionImageNV(m_vkDevice,
            m_vkOfaSession,
            bindingPoint,
            nvOFImage->ImageView(),
            VK_IMAGE_LAYOUT_GENERAL);

        if (ret != VK_SUCCESS)
            return NV_OF_ERR_GENERIC;

        return NV_OF_SUCCESS;
    }

    NV_OF_STATUS NvOFInstance::GetCaps(NV_OF_CAPS param, uint32_t* capsVal, uint32_t* size) const {
        if (!size)
            return NV_OF_ERR_INVALID_PTR;

        VkPhysicalDeviceOpticalFlowPropertiesNV props{};
        const bool hasProps = QueryOpticalFlowProperties(props);

        auto writeGridSizes = [&](const uint32_t* grids, uint32_t count) -> NV_OF_STATUS {
            if (!capsVal) {
                *size = count;
                return NV_OF_SUCCESS;
            }
            if (count > 0 && *size == 0)
                return NV_OF_ERR_INVALID_PARAM;

            const uint32_t toCopy = std::min(*size, count);
            for (uint32_t i = 0; i < toCopy; ++i)
                capsVal[i] = grids[i];
            *size = toCopy;
            return NV_OF_SUCCESS;
        };

        auto writeScalar = [&](uint32_t value) -> NV_OF_STATUS {
            if (!capsVal) {
                *size = 1;
                return NV_OF_SUCCESS;
            }
            if (*size == 0)
                return NV_OF_ERR_INVALID_PARAM;

            *capsVal = value;
            *size = 1;
            return NV_OF_SUCCESS;
        };

        switch (param) {
            case NV_OF_CAPS_SUPPORTED_OUTPUT_GRID_SIZES: {
                const VkOpticalFlowGridSizeFlagsNV mask = hasProps ? props.supportedOutputGridSizes : 0;
                uint32_t grids[3]{};
                uint32_t count = 0;
                if (mask & VK_OPTICAL_FLOW_GRID_SIZE_1X1_BIT_NV)
                    grids[count++] = NV_OF_OUTPUT_VECTOR_GRID_SIZE_1;
                if (mask & VK_OPTICAL_FLOW_GRID_SIZE_2X2_BIT_NV)
                    grids[count++] = NV_OF_OUTPUT_VECTOR_GRID_SIZE_2;
                if (mask & VK_OPTICAL_FLOW_GRID_SIZE_4X4_BIT_NV)
                    grids[count++] = NV_OF_OUTPUT_VECTOR_GRID_SIZE_4;
                if (count == 0)
                    grids[count++] = NV_OF_OUTPUT_VECTOR_GRID_SIZE_4;
                return writeGridSizes(grids, count);
            }
            case NV_OF_CAPS_SUPPORTED_HINT_GRID_SIZES: {
                const VkOpticalFlowGridSizeFlagsNV mask = hasProps ? props.supportedHintGridSizes : 0;
                uint32_t grids[4]{};
                uint32_t count = 0;
                if (mask & VK_OPTICAL_FLOW_GRID_SIZE_1X1_BIT_NV)
                    grids[count++] = NV_OF_HINT_VECTOR_GRID_SIZE_1;
                if (mask & VK_OPTICAL_FLOW_GRID_SIZE_2X2_BIT_NV)
                    grids[count++] = NV_OF_HINT_VECTOR_GRID_SIZE_2;
                if (mask & VK_OPTICAL_FLOW_GRID_SIZE_4X4_BIT_NV)
                    grids[count++] = NV_OF_HINT_VECTOR_GRID_SIZE_4;
                if (mask & VK_OPTICAL_FLOW_GRID_SIZE_8X8_BIT_NV)
                    grids[count++] = NV_OF_HINT_VECTOR_GRID_SIZE_8;
                return writeGridSizes(grids, count);
            }
            case NV_OF_CAPS_SUPPORT_HINT_WITH_OF_MODE:
                return hasProps ? writeScalar(props.hintSupported ? 1u : 0u) : NV_OF_ERR_GENERIC;
            case NV_OF_CAPS_SUPPORT_HINT_WITH_ST_MODE:
                return hasProps ? writeScalar(0u) : NV_OF_ERR_GENERIC;
            case NV_OF_CAPS_WIDTH_MIN:
                return hasProps ? writeScalar(props.minWidth) : NV_OF_ERR_GENERIC;
            case NV_OF_CAPS_HEIGHT_MIN:
                return hasProps ? writeScalar(props.minHeight) : NV_OF_ERR_GENERIC;
            case NV_OF_CAPS_WIDTH_MAX:
                return hasProps ? writeScalar(props.maxWidth) : NV_OF_ERR_GENERIC;
            case NV_OF_CAPS_HEIGHT_MAX:
                return hasProps ? writeScalar(props.maxHeight) : NV_OF_ERR_GENERIC;
            case NV_OF_CAPS_SUPPORT_ROI:
                return hasProps ? writeScalar(props.maxNumRegionsOfInterest > 0 ? 1u : 0u) : NV_OF_ERR_GENERIC;
            case NV_OF_CAPS_SUPPORT_ROI_MAX_NUM:
                return hasProps ? writeScalar(props.maxNumRegionsOfInterest) : NV_OF_ERR_GENERIC;
            default:
                return NV_OF_ERR_GENERIC;
        }
    }

    void NvOFInstance::RegisterBuffer(const NV_OF_REGISTER_RESOURCE_PARAMS_VK* registerParams) const {
        auto nvOFImage = new NvOFImage(m_vkDevice, registerParams->image, registerParams->format);
        nvOFImage->Initialize(m_vkCreateImageView, m_vkDestroyImageView);
        *registerParams->hOFGpuBuffer = reinterpret_cast<NvOFGPUBufferHandle>(nvOFImage);
    }

    void NvOFInstance::RecordCmdBuf(const NV_OF_EXECUTE_INPUT_PARAMS_VK* inParams, NV_OF_EXECUTE_OUTPUT_PARAMS_VK* outParams, VkCommandBuffer cmdBuf) const {
        BindImageToSession(inParams->inputFrame, VK_OPTICAL_FLOW_SESSION_BINDING_POINT_INPUT_NV);
        BindImageToSession(inParams->referenceFrame, VK_OPTICAL_FLOW_SESSION_BINDING_POINT_REFERENCE_NV);
        BindImageToSession(inParams->externalHints, VK_OPTICAL_FLOW_SESSION_BINDING_POINT_HINT_NV);
        BindImageToSession(outParams->outputBuffer, VK_OPTICAL_FLOW_SESSION_BINDING_POINT_FLOW_VECTOR_NV);
        BindImageToSession(outParams->outputCostBuffer, VK_OPTICAL_FLOW_SESSION_BINDING_POINT_COST_NV);
        BindImageToSession(outParams->bwdOutputBuffer, VK_OPTICAL_FLOW_SESSION_BINDING_POINT_BACKWARD_FLOW_VECTOR_NV);
        BindImageToSession(outParams->bwdOutputCostBuffer, VK_OPTICAL_FLOW_SESSION_BINDING_POINT_BACKWARD_COST_NV);
        BindImageToSession(outParams->globalFlowBuffer, VK_OPTICAL_FLOW_SESSION_BINDING_POINT_GLOBAL_FLOW_NV);
        // Support INPUT_MIPS execute priv data
        if (inParams->hPrivData && reinterpret_cast<NV_OF_PRIV_DATA*>(inParams->hPrivData)->id == NV_OF_EXECUTE_PRIV_DATA_ID_INPUT_MIPS) {
            auto mipData = static_cast<NV_OF_EXECUTE_PRIV_DATA_INPUT_MIPS*>(reinterpret_cast<NV_OF_PRIV_DATA*>(inParams->hPrivData)->data);
            for (uint32_t i = 0; i < 6; i++) {
                if (mipData->input[i] && mipData->reference[i]) {
                    BindImageToSession(mipData->input[i], VK_OPTICAL_FLOW_SESSION_BINDING_POINT_INPUT_NV);
                    BindImageToSession(mipData->reference[i], VK_OPTICAL_FLOW_SESSION_BINDING_POINT_REFERENCE_NV);
                }
            }
        }
        auto regions = std::vector<VkRect2D>(inParams->numRois);
        for (uint32_t i = 0; i < inParams->numRois; i++) {
            regions[i].offset.x = inParams->roiData[i].start_x;
            regions[i].offset.y = inParams->roiData[i].start_y;
            regions[i].extent.width = inParams->roiData[i].width;
            regions[i].extent.height = inParams->roiData[i].height;
        }

        VkOpticalFlowExecuteInfoNV ofaExecuteInfo{};
        ofaExecuteInfo.sType = VK_STRUCTURE_TYPE_OPTICAL_FLOW_EXECUTE_INFO_NV;
        ofaExecuteInfo.regionCount = inParams->numRois;
        ofaExecuteInfo.pRegions = regions.data();

        if (inParams->disableTemporalHints) {
            ofaExecuteInfo.flags |= VK_OPTICAL_FLOW_EXECUTE_DISABLE_TEMPORAL_HINTS_BIT_NV;
        }

        m_vkCmdOpticalFlowExecuteNV(cmdBuf, m_vkOfaSession, &ofaExecuteInfo);
    }
}
