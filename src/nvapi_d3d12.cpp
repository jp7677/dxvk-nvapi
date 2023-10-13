#include "nvapi_private.h"
#include "nvapi_globals.h"
#include "d3d/nvapi_d3d_low_latency_device.h"
#include "d3d12/nvapi_d3d12_device.h"
#include "util/util_statuscode.h"
#include "util/util_op_code.h"
#include "util/util_pso_extension.h"
#include "util/util_string.h"

extern "C" {
    using namespace dxvk;

    NvAPI_Status __cdecl NvAPI_D3D12_IsNvShaderExtnOpCodeSupported(ID3D12Device* pDevice, NvU32 opCode, bool* pSupported) {
        constexpr auto n = __func__;

        if (pDevice == nullptr || pSupported == nullptr)
            return InvalidArgument(n);

        // VKD3D-Proton does not know any NVIDIA intrinsics
        *pSupported = false;

        return Ok(str::format(n, " (", opCode, "/", fromCode(opCode), ")"));
    }

    NvAPI_Status __cdecl NvAPI_D3D12_EnumerateMetaCommands(ID3D12Device* pDevice, NvU32* pNumMetaCommands, NVAPI_META_COMMAND_DESC* pDescs) {
        return NotSupported(__func__);
    }

    NvAPI_Status __cdecl NvAPI_D3D12_CreateCubinComputeShaderEx(ID3D12Device* pDevice, const void* cubinData, NvU32 cubinSize, NvU32 blockX, NvU32 blockY, NvU32 blockZ, NvU32 smemSize, const char* shaderName, NVDX_ObjectHandle* pShader) {
        constexpr auto n = __func__;
        static bool alreadyLoggedError = false;
        static bool alreadyLoggedOk = false;

        if (pDevice == nullptr)
            return InvalidArgument(n);

        if (!NvapiD3d12Device::CreateCubinComputeShaderEx(pDevice, cubinData, cubinSize, blockX, blockY, blockZ, smemSize, shaderName, pShader))
            return Error(n, alreadyLoggedError);

        return Ok(n, alreadyLoggedOk);
    }

    NvAPI_Status __cdecl NvAPI_D3D12_CreateCubinComputeShaderWithName(ID3D12Device* pDevice, const void* cubinData, NvU32 cubinSize, NvU32 blockX, NvU32 blockY, NvU32 blockZ, const char* shaderName, NVDX_ObjectHandle* pShader) {
        constexpr auto n = __func__;
        static bool alreadyLoggedError = false;
        static bool alreadyLoggedOk = false;

        if (pDevice == nullptr)
            return InvalidArgument(n);

        if (!NvapiD3d12Device::CreateCubinComputeShaderWithName(pDevice, cubinData, cubinSize, blockX, blockY, blockZ, shaderName, pShader))
            return Error(n, alreadyLoggedError);

        return Ok(n, alreadyLoggedOk);
    }

    NvAPI_Status __cdecl NvAPI_D3D12_CreateCubinComputeShader(ID3D12Device* pDevice, const void* cubinData, NvU32 cubinSize, NvU32 blockX, NvU32 blockY, NvU32 blockZ, NVDX_ObjectHandle* pShader) {
        constexpr auto n = __func__;
        static bool alreadyLoggedError = false;
        static bool alreadyLoggedOk = false;

        if (pDevice == nullptr)
            return InvalidArgument(n);

        if (!NvapiD3d12Device::CreateCubinComputeShaderWithName(pDevice, cubinData, cubinSize, blockX, blockY, blockZ, "", pShader))
            return Error(n, alreadyLoggedError);

        return Ok(n, alreadyLoggedOk);
    }

    NvAPI_Status __cdecl NvAPI_D3D12_DestroyCubinComputeShader(ID3D12Device* pDevice, NVDX_ObjectHandle pShader) {
        constexpr auto n = __func__;
        static bool alreadyLoggedError = false;
        static bool alreadyLoggedOk = false;

        if (pDevice == nullptr)
            return InvalidArgument(n);

        if (!NvapiD3d12Device::DestroyCubinComputeShader(pDevice, pShader))
            return Error(n, alreadyLoggedError);

        return Ok(n, alreadyLoggedOk);
    }

    NvAPI_Status __cdecl NvAPI_D3D12_GetCudaTextureObject(ID3D12Device* pDevice, D3D12_CPU_DESCRIPTOR_HANDLE srvHandle, D3D12_CPU_DESCRIPTOR_HANDLE samplerHandle, NvU32* cudaTextureHandle) {
        constexpr auto n = __func__;
        static bool alreadyLoggedError = false;
        static bool alreadyLoggedOk = false;

        if (pDevice == nullptr)
            return InvalidArgument(n);

        if (!NvapiD3d12Device::GetCudaTextureObject(pDevice, srvHandle, samplerHandle, cudaTextureHandle))
            return Error(n, alreadyLoggedError);

        return Ok(n, alreadyLoggedOk);
    }

    NvAPI_Status __cdecl NvAPI_D3D12_GetCudaSurfaceObject(ID3D12Device* pDevice, D3D12_CPU_DESCRIPTOR_HANDLE uavHandle, NvU32* cudaSurfaceHandle) {
        constexpr auto n = __func__;
        static bool alreadyLoggedError = false;
        static bool alreadyLoggedOk = false;

        if (pDevice == nullptr)
            return InvalidArgument(n);

        if (!NvapiD3d12Device::GetCudaSurfaceObject(pDevice, uavHandle, cudaSurfaceHandle))
            return Error(n, alreadyLoggedError);

        return Ok(n, alreadyLoggedOk);
    }

    NvAPI_Status __cdecl NvAPI_D3D12_LaunchCubinShader(ID3D12GraphicsCommandList* pCmdList, NVDX_ObjectHandle pShader, NvU32 blockX, NvU32 blockY, NvU32 blockZ, const void* params, NvU32 paramSize) {
        constexpr auto n = __func__;
        static bool alreadyLoggedError = false;
        static bool alreadyLoggedOk = false;

        if (pCmdList == nullptr)
            return InvalidArgument(n);

        if (!NvapiD3d12Device::LaunchCubinShader(pCmdList, pShader, blockX, blockY, blockZ, params, paramSize))
            return Error(n, alreadyLoggedError);

        return Ok(n, alreadyLoggedOk);
    }

    NvAPI_Status __cdecl NvAPI_D3D12_CaptureUAVInfo(ID3D12Device* pDevice, NVAPI_UAV_INFO* pUAVInfo) {
        constexpr auto n = __func__;
        static bool alreadyLoggedError = false;
        static bool alreadyLoggedOk = false;

        if (pDevice == nullptr)
            return InvalidArgument(n);

        if (!NvapiD3d12Device::CaptureUAVInfo(pDevice, pUAVInfo))
            return Error(n, alreadyLoggedError);

        return Ok(n, alreadyLoggedOk);
    }

    NvAPI_Status __cdecl NvAPI_D3D12_GetGraphicsCapabilities(IUnknown* pDevice, NvU32 structVersion, NV_D3D12_GRAPHICS_CAPS* pGraphicsCaps) {
        constexpr auto n = __func__;

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (pDevice == nullptr)
            return InvalidArgument(n);

        if (structVersion != NV_D3D12_GRAPHICS_CAPS_VER1)
            return IncompatibleStructVersion(n);

        pGraphicsCaps->majorSMVersion = 0;
        pGraphicsCaps->minorSMVersion = 0;
        // all Vulkan drivers are expected to support ZBC clear without padding
        pGraphicsCaps->bFastUAVClearSupported = true;

        NvapiAdapter* adapter = nullptr;
        auto luid = NvapiD3d12Device::GetLuid(pDevice);
        if (luid.has_value())
            adapter = nvapiAdapterRegistry->FindAdapter(luid.value());

        if (adapter == nullptr || !adapter->HasNvProprietaryDriver())
            return Ok(str::format(n, " (sm_0)"));

        // From https://arnon.dk/matching-sm-architectures-arch-and-gencode-for-various-nvidia-cards/
        // Note: One might think that SM here is D3D12 Shader Model, in fact it is the "Streaming Multiprocessor" architecture name
        switch (adapter->GetArchitectureId()) {
            case NV_GPU_ARCHITECTURE_AD100:
                pGraphicsCaps->majorSMVersion = 8;
                pGraphicsCaps->minorSMVersion = 9;
                break;
            case NV_GPU_ARCHITECTURE_GA100:
                pGraphicsCaps->majorSMVersion = 8;
                pGraphicsCaps->minorSMVersion = 6; // Take the risk that no one uses an NVIDIA A100 with this implementation
                break;
            case NV_GPU_ARCHITECTURE_TU100:
                pGraphicsCaps->majorSMVersion = 7;
                pGraphicsCaps->minorSMVersion = 5;
                break;
            case NV_GPU_ARCHITECTURE_GV100:
                pGraphicsCaps->majorSMVersion = 7;
                pGraphicsCaps->minorSMVersion = 0;
                break;
            case NV_GPU_ARCHITECTURE_GP100:
                pGraphicsCaps->majorSMVersion = 6;
                pGraphicsCaps->minorSMVersion = 0;
                break;
            case NV_GPU_ARCHITECTURE_GM200:
                pGraphicsCaps->majorSMVersion = 5;
                pGraphicsCaps->minorSMVersion = 0;
                break;
            default:
                break;
        }

        // These fields should be derivable from architecture, but not sure yet what the consequences are
        // Currently we do not require these fields, we can implement them later.
        // pGraphicsCaps->bExclusiveScissorRectsSupported = ;
        // pGraphicsCaps->bVariablePixelRateShadingSupported = ; // Should be supported on Turing and newer

        return Ok(str::format(n, " (sm_", pGraphicsCaps->majorSMVersion, pGraphicsCaps->minorSMVersion, ")"));
    }

    NvAPI_Status __cdecl NvAPI_D3D12_IsFatbinPTXSupported(ID3D12Device* pDevice, bool* isSupported) {
        constexpr auto n = __func__;
        static bool alreadyLoggedError = false;
        static bool alreadyLoggedOk = false;

        if (pDevice == nullptr)
            return InvalidArgument(n);

        *isSupported = NvapiD3d12Device::IsFatbinPTXSupported(pDevice);
        if (!*isSupported)
            return Error(n, alreadyLoggedError);

        return Ok(n, alreadyLoggedOk);
    }

    NvAPI_Status __cdecl NvAPI_D3D12_CreateGraphicsPipelineState(ID3D12Device* pDevice, const D3D12_GRAPHICS_PIPELINE_STATE_DESC* pPSODesc, NvU32 numExtensions, const NVAPI_D3D12_PSO_EXTENSION_DESC** ppExtensions, ID3D12PipelineState** ppPSO) {
        constexpr auto n = __func__;
        static bool alreadyLoggedOk = false;

        if (pDevice == nullptr)
            return InvalidArgument(n);

        if (numExtensions == 0)
            return InvalidArgument(n);

        if (ppExtensions[0]->baseVersion != NV_PSO_EXTENSION_DESC_VER_1)
            return IncompatibleStructVersion(n);

        std::string extensionNames;
        for (auto i = 0U; i < numExtensions; i++)
            extensionNames += str::format(fromPsoExtension(ppExtensions[i]->psoExtension), ",");

        extensionNames.pop_back();

        if (!NvapiD3d12Device::CreateGraphicsPipelineState(pDevice, pPSODesc, numExtensions, ppExtensions, ppPSO))
            return NotSupported(str::format(n, " (", numExtensions, "/", extensionNames, ")"));

        return Ok(str::format(n, " (", numExtensions, "/", extensionNames, ")"), alreadyLoggedOk);
    }

    NvAPI_Status __cdecl NvAPI_D3D12_SetDepthBoundsTestValues(ID3D12GraphicsCommandList* pCommandList, const float minDepth, const float maxDepth) {
        constexpr auto n = __func__;
        static bool alreadyLoggedError = false;
        static bool alreadyLoggedOk = false;

        if (pCommandList == nullptr)
            return InvalidArgument(n);

        if (!NvapiD3d12Device::SetDepthBoundsTestValues(pCommandList, minDepth, maxDepth))
            return Error(n, alreadyLoggedError);

        return Ok(n, alreadyLoggedOk);
    }

    NvAPI_Status __cdecl NvAPI_D3D12_GetRaytracingCaps(ID3D12Device* pDevice, NVAPI_D3D12_RAYTRACING_CAPS_TYPE type, void* pData, size_t dataSize) {
        constexpr auto n = __func__;

        if (pDevice == nullptr || pData == nullptr)
            return InvalidPointer(n);

        switch (type) {
            case NVAPI_D3D12_RAYTRACING_CAPS_TYPE_THREAD_REORDERING:
                if (dataSize != sizeof(NVAPI_D3D12_RAYTRACING_THREAD_REORDERING_CAPS))
                    return InvalidArgument(n);

                // let's hope that NvAPI_D3D12_IsNvShaderExtnOpCodeSupported returning false is enough to discourage games from attempting to use Shader Execution Reordering
                *(NVAPI_D3D12_RAYTRACING_THREAD_REORDERING_CAPS*)pData = NVAPI_D3D12_RAYTRACING_THREAD_REORDERING_CAP_NONE;
                break;

            case NVAPI_D3D12_RAYTRACING_CAPS_TYPE_OPACITY_MICROMAP:
                if (dataSize != sizeof(NVAPI_D3D12_RAYTRACING_OPACITY_MICROMAP_CAPS))
                    return InvalidArgument(n);

                *(NVAPI_D3D12_RAYTRACING_OPACITY_MICROMAP_CAPS*)pData = NVAPI_D3D12_RAYTRACING_OPACITY_MICROMAP_CAP_NONE;
                break;

            case NVAPI_D3D12_RAYTRACING_CAPS_TYPE_DISPLACEMENT_MICROMAP:
                if (dataSize != sizeof(NVAPI_D3D12_RAYTRACING_DISPLACEMENT_MICROMAP_CAPS))
                    return InvalidArgument(n);

                *(NVAPI_D3D12_RAYTRACING_DISPLACEMENT_MICROMAP_CAPS*)pData = NVAPI_D3D12_RAYTRACING_DISPLACEMENT_MICROMAP_CAP_NONE;
                break;

            default:
                return InvalidArgument(n);
        }

        return Ok(str::format(n, " (", type, ")"));
    }

    static bool ConvertBuildRaytracingAccelerationStructureInputs(const NVAPI_D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS_EX* nvDesc, std::vector<D3D12_RAYTRACING_GEOMETRY_DESC>& geometryDescs, D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS* d3dDesc) {
        d3dDesc->Type = nvDesc->type;
        // assume that OMM via VK_EXT_opacity_micromap and DMM via VK_NV_displacement_micromap are not supported, allow only standard flags to be passed
        d3dDesc->Flags = static_cast<D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS>(nvDesc->flags & 0x3f);
        d3dDesc->NumDescs = nvDesc->numDescs;
        d3dDesc->DescsLayout = nvDesc->descsLayout;

        if (d3dDesc->Type == D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL) {
            d3dDesc->InstanceDescs = nvDesc->instanceDescs;
            return true;
        }

        if (d3dDesc->Type == D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL && d3dDesc->DescsLayout == D3D12_ELEMENTS_LAYOUT_ARRAY_OF_POINTERS) {
            d3dDesc->ppGeometryDescs = reinterpret_cast<const D3D12_RAYTRACING_GEOMETRY_DESC* const*>(nvDesc->ppGeometryDescs);
            return true;
        }

        if (d3dDesc->Type == D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL && d3dDesc->DescsLayout == D3D12_ELEMENTS_LAYOUT_ARRAY) {
            geometryDescs.resize(d3dDesc->NumDescs);

            for (unsigned i = 0; i < d3dDesc->NumDescs; ++i) {
                auto& d3dGeoDesc = geometryDescs[i];
                auto& nvGeoDesc = *reinterpret_cast<const NVAPI_D3D12_RAYTRACING_GEOMETRY_DESC_EX*>(reinterpret_cast<const std::byte*>(nvDesc->pGeometryDescs) + (i * nvDesc->geometryDescStrideInBytes));

                d3dGeoDesc.Flags = nvGeoDesc.flags;

                switch (nvGeoDesc.type) {
                    case NVAPI_D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES_EX:
                        d3dGeoDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
                        d3dGeoDesc.Triangles = nvGeoDesc.triangles;
                        break;
                    case NVAPI_D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS_EX:
                        d3dGeoDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS;
                        d3dGeoDesc.AABBs = nvGeoDesc.aabbs;
                        break;
                    case NVAPI_D3D12_RAYTRACING_GEOMETRY_TYPE_OMM_TRIANGLES_EX: // GetRaytracingCaps reports no OMM caps, we shouldn't reach this
                        log::write("Triangles with OMM attachment passed to acceleration structure build when OMM is not supported");
                        return false;
                    case NVAPI_D3D12_RAYTRACING_GEOMETRY_TYPE_DMM_TRIANGLES_EX: // GetRaytracingCaps reports no DMM caps, we shouldn't reach this
                        log::write("Triangles with DMM attachment passed to acceleration structure build when DMM is not supported");
                        return false;
                    default:
                        log::write(str::format("Unknown NVAPI_D3D12_RAYTRACING_GEOMETRY_TYPE_EX: ", nvGeoDesc.type));
                        return false;
                }
            }

            d3dDesc->pGeometryDescs = geometryDescs.data();
            return true;
        }

        return false;
    }

    NvAPI_Status __cdecl NvAPI_D3D12_GetRaytracingAccelerationStructurePrebuildInfoEx(ID3D12Device5* pDevice, NVAPI_GET_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO_EX_PARAMS* pParams) {
        constexpr auto n = __func__;
        static bool alreadyLoggedOk = false;

        if (pDevice == nullptr || pParams == nullptr)
            return InvalidArgument(n);

        if (pParams->version != NVAPI_GET_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO_EX_PARAMS_VER1)
            return IncompatibleStructVersion(n);

        if (pParams->pDesc == nullptr || pParams->pInfo == nullptr)
            return InvalidArgument(n);

        std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> geometryDescs{};
        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS desc{};

        if (!ConvertBuildRaytracingAccelerationStructureInputs(pParams->pDesc, geometryDescs, &desc))
            return InvalidArgument(n);

        pDevice->GetRaytracingAccelerationStructurePrebuildInfo(&desc, pParams->pInfo);

        return Ok(n, alreadyLoggedOk);
    }

    NvAPI_Status __cdecl NvAPI_D3D12_BuildRaytracingAccelerationStructureEx(ID3D12GraphicsCommandList4* pCommandList, const NVAPI_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_EX_PARAMS* pParams) {
        constexpr auto n = __func__;
        static bool alreadyLoggedOk = false;

        if (pCommandList == nullptr || pParams == nullptr)
            return InvalidArgument(n);

        if (pParams->version != NVAPI_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_EX_PARAMS_VER1)
            return IncompatibleStructVersion(n);

        if (pParams->pDesc == nullptr || (pParams->numPostbuildInfoDescs != 0 && pParams->pPostbuildInfoDescs == nullptr))
            return InvalidArgument(n);

        std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> geometryDescs{};
        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC desc = {
            .DestAccelerationStructureData = pParams->pDesc->destAccelerationStructureData,
            .Inputs = {},
            .SourceAccelerationStructureData = pParams->pDesc->sourceAccelerationStructureData,
            .ScratchAccelerationStructureData = pParams->pDesc->scratchAccelerationStructureData,
        };

        if (!ConvertBuildRaytracingAccelerationStructureInputs(&pParams->pDesc->inputs, geometryDescs, &desc.Inputs))
            return InvalidArgument(n);

        pCommandList->BuildRaytracingAccelerationStructure(&desc, pParams->numPostbuildInfoDescs, pParams->pPostbuildInfoDescs);

        return Ok(n, alreadyLoggedOk);
    }

    NvAPI_Status __cdecl NvAPI_D3D12_NotifyOutOfBandCommandQueue(ID3D12CommandQueue* pCommandQueue, NV_OUT_OF_BAND_CQ_TYPE cqType) {
        constexpr auto n = __func__;
        static bool alreadyLoggedError = false;
        static bool alreadyLoggedOk = false;

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (pCommandQueue == nullptr)
            return InvalidArgument(n);

        ID3D12Device* pDevice;
        if (FAILED(pCommandQueue->GetDevice(IID_PPV_ARGS(&pDevice))))
            return InvalidArgument(n);

        if (nvapiD3dInstance->IsUsingLfx() || !NvapiD3dLowLatencyDevice::SupportsLowLatency(pDevice))
            return NoImplementation(n);

        if (!NvapiD3d12Device::NotifyOutOfBandCommandQueue(pCommandQueue, static_cast<D3D12_OUT_OF_BAND_CQ_TYPE>(cqType)))
            return Error(n, alreadyLoggedError);

        return Ok(n, alreadyLoggedOk);
    }

    NvAPI_Status __cdecl NvAPI_D3D12_SetAsyncFrameMarker(ID3D12CommandQueue* pCommandQueue, NV_LATENCY_MARKER_PARAMS* pSetLatencyMarkerParams) {
        constexpr auto n = __func__;
        static bool alreadyLoggedError = false;
        static bool alreadyLoggedOk = false;

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (pSetLatencyMarkerParams->version != NV_LATENCY_MARKER_PARAMS_VER1)
            return IncompatibleStructVersion(n);

        if (pCommandQueue == nullptr)
            return InvalidArgument(n);

        ID3D12Device* pDevice;
        if (FAILED(pCommandQueue->GetDevice(IID_PPV_ARGS(&pDevice))))
            return InvalidArgument(n);

        if (nvapiD3dInstance->IsUsingLfx() || !NvapiD3dLowLatencyDevice::SupportsLowLatency(pDevice))
            return NoImplementation(n);

        if (!NvapiD3dLowLatencyDevice::SetLatencyMarker(pDevice, pSetLatencyMarkerParams->frameID, pSetLatencyMarkerParams->markerType))
            return Error(n, alreadyLoggedError);

        return Ok(n, alreadyLoggedOk);
    }
}
