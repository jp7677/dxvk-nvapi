#include "nvapi_private.h"
#include "nvapi_globals.h"
#include "nvapi/nvapi_d3d_low_latency_device.h"
#include "nvapi/nvapi_d3d12_device.h"
#include "nvapi/nvapi_d3d12_graphics_command_list.h"
#include "nvapi/nvapi_d3d12_command_queue.h"
#include "util/com_pointer.h"
#include "util/util_statuscode.h"
#include "util/util_op_code.h"
#include "util/util_pso_extension.h"
#include "util/util_string.h"

extern "C" {
    using namespace dxvk;

    NvAPI_Status __cdecl NvAPI_D3D12_IsNvShaderExtnOpCodeSupported(ID3D12Device* pDevice, NvU32 opCode, bool* pSupported) {
        constexpr auto n = __func__;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(pDevice), opCode, log::fmt::ptr(pSupported));

        if (!pDevice || !pSupported)
            return InvalidArgument(n);

        // VKD3D-Proton does not know any NVIDIA intrinsics
        *pSupported = false;

        return Ok(str::format(n, " (", opCode, "/", fromCode(opCode), ")"));
    }

    NvAPI_Status __cdecl NvAPI_D3D12_EnumerateMetaCommands(ID3D12Device* pDevice, NvU32* pNumMetaCommands, NVAPI_META_COMMAND_DESC* pDescs) {
        constexpr auto n = __func__;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(pDevice), log::fmt::ptr(pNumMetaCommands), log::fmt::ptr(pDescs));

        if (!pDevice || !pNumMetaCommands)
            return InvalidArgument(n);

        *pNumMetaCommands = 0; // No meta commands with this implementation

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_D3D12_CreateCubinComputeShaderEx(ID3D12Device* pDevice, const void* cubinData, NvU32 cubinSize, NvU32 blockX, NvU32 blockY, NvU32 blockZ, NvU32 smemSize, const char* shaderName, NVDX_ObjectHandle* pShader) {
        constexpr auto n = __func__;
        thread_local bool alreadyLoggedNoImplementation = false;
        thread_local bool alreadyLoggedError = false;
        thread_local bool alreadyLoggedOk = false;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(pDevice), log::fmt::ptr(cubinData), cubinSize, blockX, blockY, blockZ, smemSize, log::fmt::ptr(shaderName), log::fmt::ptr(pShader));

        if (!pDevice || !shaderName || !pShader)
            return InvalidArgument(n);

        auto device = NvapiD3d12Device::GetOrCreate(pDevice);
        if (!device)
            return NoImplementation(n, alreadyLoggedNoImplementation);

        switch (device->CreateCubinComputeShaderEx(cubinData, cubinSize, blockX, blockY, blockZ, smemSize, shaderName, pShader)) {
            case S_OK:
                return Ok(n, alreadyLoggedOk);
            case E_NOTIMPL:
                return NoImplementation(n, alreadyLoggedNoImplementation);
            default:
                return Error(n, alreadyLoggedError);
        }
    }

    NvAPI_Status __cdecl NvAPI_D3D12_CreateCubinComputeShaderWithName(ID3D12Device* pDevice, const void* cubinData, NvU32 cubinSize, NvU32 blockX, NvU32 blockY, NvU32 blockZ, const char* shaderName, NVDX_ObjectHandle* pShader) {
        constexpr auto n = __func__;
        thread_local bool alreadyLoggedNoImplementation = false;
        thread_local bool alreadyLoggedError = false;
        thread_local bool alreadyLoggedOk = false;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(pDevice), log::fmt::ptr(cubinData), cubinSize, blockX, blockY, blockZ, log::fmt::ptr(shaderName), log::fmt::ptr(pShader));

        if (!pDevice || !shaderName || !pShader)
            return InvalidArgument(n);

        auto device = NvapiD3d12Device::GetOrCreate(pDevice);
        if (!device)
            return NoImplementation(n, alreadyLoggedNoImplementation);

        switch (device->CreateCubinComputeShaderWithName(cubinData, cubinSize, blockX, blockY, blockZ, shaderName, pShader)) {
            case S_OK:
                return Ok(n, alreadyLoggedOk);
            case E_NOTIMPL:
                return NoImplementation(n, alreadyLoggedNoImplementation);
            default:
                return Error(n, alreadyLoggedError);
        }
    }

    NvAPI_Status __cdecl NvAPI_D3D12_CreateCubinComputeShader(ID3D12Device* pDevice, const void* cubinData, NvU32 cubinSize, NvU32 blockX, NvU32 blockY, NvU32 blockZ, NVDX_ObjectHandle* pShader) {
        constexpr auto n = __func__;
        thread_local bool alreadyLoggedNoImplementation = false;
        thread_local bool alreadyLoggedError = false;
        thread_local bool alreadyLoggedOk = false;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(pDevice), log::fmt::ptr(cubinData), cubinSize, blockX, blockY, blockZ, log::fmt::ptr(pShader));

        if (!pDevice || !pShader)
            return InvalidArgument(n);

        auto device = NvapiD3d12Device::GetOrCreate(pDevice);
        if (!device)
            return NoImplementation(n, alreadyLoggedNoImplementation);

        switch (device->CreateCubinComputeShaderWithName(cubinData, cubinSize, blockX, blockY, blockZ, "", pShader)) {
            case S_OK:
                return Ok(n, alreadyLoggedOk);
            case E_NOTIMPL:
                return NoImplementation(n, alreadyLoggedNoImplementation);
            default:
                return Error(n, alreadyLoggedError);
        }
    }

    NvAPI_Status __cdecl NvAPI_D3D12_DestroyCubinComputeShader(ID3D12Device* pDevice, NVDX_ObjectHandle pShader) {
        constexpr auto n = __func__;
        thread_local bool alreadyLoggedNoImplementation = false;
        thread_local bool alreadyLoggedError = false;
        thread_local bool alreadyLoggedOk = false;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(pDevice), log::fmt::hnd(pShader));

        if (!pDevice)
            return InvalidArgument(n);

        auto device = NvapiD3d12Device::GetOrCreate(pDevice);
        if (!device)
            return NoImplementation(n, alreadyLoggedNoImplementation);

        switch (device->DestroyCubinComputeShader(pShader)) {
            case S_OK:
                return Ok(n, alreadyLoggedOk);
            case E_NOTIMPL:
                return NoImplementation(n, alreadyLoggedNoImplementation);
            default:
                return Error(n, alreadyLoggedError);
        }
    }

    NvAPI_Status __cdecl NvAPI_D3D12_GetCudaTextureObject(ID3D12Device* pDevice, D3D12_CPU_DESCRIPTOR_HANDLE srvHandle, D3D12_CPU_DESCRIPTOR_HANDLE samplerHandle, NvU32* cudaTextureHandle) {
        constexpr auto n = __func__;
        thread_local bool alreadyLoggedNoImplementation = false;
        thread_local bool alreadyLoggedError = false;
        thread_local bool alreadyLoggedOk = false;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(pDevice), log::fmt::d3d12_cpu_descriptor_handle(srvHandle), log::fmt::d3d12_cpu_descriptor_handle(samplerHandle), log::fmt::ptr(cudaTextureHandle));

        if (!pDevice || !cudaTextureHandle)
            return InvalidArgument(n);

        auto device = NvapiD3d12Device::GetOrCreate(pDevice);
        if (!device)
            return NoImplementation(n, alreadyLoggedNoImplementation);

        switch (device->GetCudaTextureObject(srvHandle, samplerHandle, cudaTextureHandle)) {
            case S_OK:
                return Ok(n, alreadyLoggedOk);
            case E_NOTIMPL:
                return NoImplementation(n, alreadyLoggedNoImplementation);
            default:
                return Error(n, alreadyLoggedError);
        }
    }

    NvAPI_Status __cdecl NvAPI_D3D12_GetCudaSurfaceObject(ID3D12Device* pDevice, D3D12_CPU_DESCRIPTOR_HANDLE uavHandle, NvU32* cudaSurfaceHandle) {
        constexpr auto n = __func__;
        thread_local bool alreadyLoggedNoImplementation = false;
        thread_local bool alreadyLoggedError = false;
        thread_local bool alreadyLoggedOk = false;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(pDevice), log::fmt::d3d12_cpu_descriptor_handle(uavHandle), log::fmt::ptr(cudaSurfaceHandle));

        if (!pDevice || !cudaSurfaceHandle)
            return InvalidArgument(n);

        auto device = NvapiD3d12Device::GetOrCreate(pDevice);
        if (!device)
            return NoImplementation(n, alreadyLoggedNoImplementation);

        switch (device->GetCudaSurfaceObject(uavHandle, cudaSurfaceHandle)) {
            case S_OK:
                return Ok(n, alreadyLoggedOk);
            case E_NOTIMPL:
                return NoImplementation(n, alreadyLoggedNoImplementation);
            default:
                return Error(n, alreadyLoggedError);
        }
    }

    NvAPI_Status __cdecl NvAPI_D3D12_CreateCubinComputeShaderExV2(NVAPI_D3D12_CREATE_CUBIN_SHADER_PARAMS* pParams) {
        static constexpr auto V1StructSize = offsetof(NVAPI_D3D12_CREATE_CUBIN_SHADER_PARAMS, hShader) + sizeof(NVAPI_D3D12_CREATE_CUBIN_SHADER_PARAMS::hShader);
        // static_assert(V1StructSize == sizeof(NVAPI_D3D12_CREATE_CUBIN_SHADER_PARAMS));
        constexpr auto n = __func__;
        thread_local bool alreadyLoggedNoImplementation = false;
        thread_local bool alreadyLoggedError = false;
        thread_local bool alreadyLoggedOk = false;

        if (log::tracing())
            log::trace(n, log::fmt::nvapi_d3d12_create_cubin_shader_params(pParams));

        if (!pParams)
            return InvalidPointer(n);

        pParams->structSizeOut = V1StructSize;

        if (pParams->structSizeIn < V1StructSize)
            return IncompatibleStructVersion(n, pParams->structSizeIn);

        if (!pParams->pDevice || !pParams->pShaderName)
            return InvalidArgument(n);

        auto device = NvapiD3d12Device::GetOrCreate(pParams->pDevice);
        if (!device)
            return NoImplementation(n, alreadyLoggedNoImplementation);

        D3D12_CREATE_CUBIN_SHADER_PARAMS params;
        params.pNext = nullptr;
        params.pCubin = pParams->pCubin;
        params.size = pParams->size;
        params.blockX = pParams->blockX;
        params.blockY = pParams->blockY;
        params.blockZ = pParams->blockZ;
        params.dynSharedMemBytes = pParams->dynSharedMemBytes;
        params.pShaderName = pParams->pShaderName;
        params.flags = pParams->flags;

        switch (device->CreateCubinComputeShaderExV2(&params)) {
            case S_OK:
                pParams->hShader = reinterpret_cast<NVDX_ObjectHandle>(params.hShader);
                return Ok(n, alreadyLoggedOk);
            case E_INVALIDARG:
                return InvalidArgument(n);
            case E_NOTIMPL:
                return NoImplementation(n, alreadyLoggedNoImplementation);
            default:
                return Error(n, alreadyLoggedError);
        }
    }

    NvAPI_Status __cdecl NvAPI_D3D12_GetCudaMergedTextureSamplerObject(NVAPI_D3D12_GET_CUDA_MERGED_TEXTURE_SAMPLER_OBJECT_PARAMS* pParams) {
        static constexpr auto V1StructSize = offsetof(NVAPI_D3D12_GET_CUDA_MERGED_TEXTURE_SAMPLER_OBJECT_PARAMS, textureHandle) + sizeof(NVAPI_D3D12_GET_CUDA_MERGED_TEXTURE_SAMPLER_OBJECT_PARAMS::textureHandle);
        // static_assert(V1StructSize == sizeof(NVAPI_D3D12_GET_CUDA_MERGED_TEXTURE_SAMPLER_OBJECT_PARAMS));
        constexpr auto n = __func__;
        thread_local bool alreadyLoggedNoImplementation = false;
        thread_local bool alreadyLoggedError = false;
        thread_local bool alreadyLoggedOk = false;

        if (log::tracing())
            log::trace(n, log::fmt::nvapi_d3d12_get_cuda_merged_texture_sampler_object_params(pParams));

        if (!pParams)
            return InvalidPointer(n);

        pParams->structSizeOut = V1StructSize;

        if (pParams->structSizeIn < V1StructSize)
            return IncompatibleStructVersion(n, pParams->structSizeIn);

        if (!pParams->pDevice || !pParams->texDesc.ptr)
            return InvalidArgument(n);

        auto device = NvapiD3d12Device::GetOrCreate(pParams->pDevice);
        if (!device)
            return NoImplementation(n, alreadyLoggedNoImplementation);

        D3D12_GET_CUDA_MERGED_TEXTURE_SAMPLER_OBJECT_PARAMS params;
        params.pNext = nullptr;
        params.texDesc = pParams->texDesc.ptr;
        params.smpDesc = pParams->smpDesc.ptr;

        switch (device->GetCudaMergedTextureSamplerObject(&params)) {
            case S_OK:
                pParams->textureHandle = params.textureHandle;
                return Ok(n, alreadyLoggedOk);
            case E_INVALIDARG:
                return InvalidArgument(n);
            case E_NOTIMPL:
                return NoImplementation(n, alreadyLoggedNoImplementation);
            default:
                return Error(n, alreadyLoggedError);
        }
    }

    NvAPI_Status __cdecl NvAPI_D3D12_GetCudaIndependentDescriptorObject(NVAPI_D3D12_GET_CUDA_INDEPENDENT_DESCRIPTOR_OBJECT_PARAMS* pParams) {
        static constexpr auto V1StructSize = offsetof(NVAPI_D3D12_GET_CUDA_INDEPENDENT_DESCRIPTOR_OBJECT_PARAMS, handle) + sizeof(NVAPI_D3D12_GET_CUDA_INDEPENDENT_DESCRIPTOR_OBJECT_PARAMS::handle);
        // static_assert(V1StructSize == sizeof(NVAPI_D3D12_GET_CUDA_INDEPENDENT_DESCRIPTOR_OBJECT_PARAMS));
        constexpr auto n = __func__;
        thread_local bool alreadyLoggedNoImplementation = false;
        thread_local bool alreadyLoggedError = false;
        thread_local bool alreadyLoggedOk = false;

        if (log::tracing())
            log::trace(n, log::fmt::nvapi_d3d12_get_cuda_independent_descriptor_object_params(pParams));

        if (!pParams)
            return InvalidPointer(n);

        pParams->structSizeOut = V1StructSize;

        if (pParams->structSizeIn < V1StructSize)
            return IncompatibleStructVersion(n, pParams->structSizeIn);

        if (!pParams->pDevice || !pParams->desc.ptr)
            return InvalidArgument(n);

        auto device = NvapiD3d12Device::GetOrCreate(pParams->pDevice);
        if (!device)
            return NoImplementation(n, alreadyLoggedNoImplementation);

        D3D12_GET_CUDA_INDEPENDENT_DESCRIPTOR_OBJECT_PARAMS params;
        params.pNext = nullptr;
        params.type = static_cast<D3D12_GET_CUDA_INDEPENDENT_DESCRIPTOR_OBJECT_TYPE>(pParams->type);
        params.desc = pParams->desc.ptr;

        switch (device->GetCudaIndependentDescriptorObject(&params)) {
            case S_OK:
                pParams->handle = params.handle;
                return Ok(n, alreadyLoggedOk);
            case E_INVALIDARG:
                return InvalidArgument(n);
            case E_NOTIMPL:
                return NoImplementation(n, alreadyLoggedNoImplementation);
            default:
                return Error(n, alreadyLoggedError);
        }
    }

    NvAPI_Status __cdecl NvAPI_D3D12_LaunchCubinShader(ID3D12GraphicsCommandList* pCmdList, NVDX_ObjectHandle pShader, NvU32 blockX, NvU32 blockY, NvU32 blockZ, const void* params, NvU32 paramSize) {
        constexpr auto n = __func__;
        thread_local bool alreadyLoggedNoImplementation = false;
        thread_local bool alreadyLoggedError = false;
        thread_local bool alreadyLoggedOk = false;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(pCmdList), log::fmt::hnd(pShader), blockX, blockY, blockZ, log::fmt::ptr(params), paramSize);

        if (!pCmdList)
            return InvalidArgument(n);

        auto device = NvapiD3d12GraphicsCommandList::GetOrCreate(pCmdList);
        if (!device)
            return NoImplementation(n, alreadyLoggedNoImplementation);

        switch (device->LaunchCubinShader(pShader, blockX, blockY, blockZ, params, paramSize)) {
            case S_OK:
                return Ok(n, alreadyLoggedOk);
            case E_NOTIMPL:
                return NoImplementation(n, alreadyLoggedNoImplementation);
            default:
                return Error(n, alreadyLoggedError);
        }
    }

    NvAPI_Status __cdecl NvAPI_D3D12_CaptureUAVInfo(ID3D12Device* pDevice, NVAPI_UAV_INFO* pUAVInfo) {
        constexpr auto n = __func__;
        thread_local bool alreadyLoggedNoImplementation = false;
        thread_local bool alreadyLoggedError = false;
        thread_local bool alreadyLoggedOk = false;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(pDevice), log::fmt::ptr(pUAVInfo));

        if (!pDevice || !pUAVInfo)
            return InvalidArgument(n);

        auto device = NvapiD3d12Device::GetOrCreate(pDevice);
        if (!device)
            return NoImplementation(n, alreadyLoggedNoImplementation);

        switch (device->CaptureUAVInfo(pUAVInfo)) {
            case S_OK:
                return Ok(n, alreadyLoggedOk);
            case E_NOTIMPL:
                return NoImplementation(n, alreadyLoggedNoImplementation);
            default:
                return Error(n, alreadyLoggedError);
        }
    }

    NvAPI_Status __cdecl NvAPI_D3D12_GetGraphicsCapabilities(IUnknown* pDevice, NvU32 structVersion, NV_D3D12_GRAPHICS_CAPS* pGraphicsCaps) {
        constexpr auto n = __func__;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(pDevice), structVersion, log::fmt::ptr(pGraphicsCaps));

        if (!nvapiAdapterRegistry)
            return ApiNotInitialized(n);

        if (!pDevice || !pGraphicsCaps)
            return InvalidArgument(n);

        if (structVersion != NV_D3D12_GRAPHICS_CAPS_VER1)
            return IncompatibleStructVersion(n, structVersion);

        pGraphicsCaps->majorSMVersion = 0;
        pGraphicsCaps->minorSMVersion = 0;
        // All Vulkan drivers are expected to support ZBC clear without padding
        pGraphicsCaps->bFastUAVClearSupported = true;
        pGraphicsCaps->bExclusiveScissorRectsSupported = false;
        pGraphicsCaps->bVariablePixelRateShadingSupported = false;

        NvapiAdapter* adapter = nullptr;

        Com<ID3D12Device> device;
        if (SUCCEEDED(pDevice->QueryInterface(IID_PPV_ARGS(&device))))
            adapter = nvapiAdapterRegistry->FindAdapter(device->GetAdapterLuid());

        if (!adapter)
            return Ok(str::format(n, " (sm_0)"));

        // From https://arnon.dk/matching-sm-architectures-arch-and-gencode-for-various-nvidia-cards/ and https://en.wikipedia.org/wiki/CUDA#GPUs_supported
        // Note: One might think that SM here is D3D12 Shader Model, in fact it is the "Streaming Multiprocessor" architecture version
        // Values are valid for Turing and newer only, due to VK_NV_cuda_kernel_launch not being supported by earlier generations
        auto computeCapability = adapter->GetComputeCapability();
        if (computeCapability.has_value()) {
            pGraphicsCaps->majorSMVersion = computeCapability.value().first;
            pGraphicsCaps->minorSMVersion = computeCapability.value().second;
        } else if (adapter->HasNvProprietaryDriver()) {
            // Fallback because older than Turing or no support for VK_NV_cuda_kernel_launch
            // Based on https://gitlab.freedesktop.org/mesa/mesa/-/blob/main/src/nouveau/winsys/nouveau_device.c
            auto architectureId = adapter->GetArchitectureId();
            if (architectureId >= NV_GPU_ARCHITECTURE_GB200) {
                pGraphicsCaps->majorSMVersion = 12; // Report highest known version also for future generations
                pGraphicsCaps->minorSMVersion = 0;
            } else if (architectureId == NV_GPU_ARCHITECTURE_AD100) {
                pGraphicsCaps->majorSMVersion = 8;
                pGraphicsCaps->minorSMVersion = 9;
            } else if (architectureId == NV_GPU_ARCHITECTURE_GA100) {
                pGraphicsCaps->majorSMVersion = 8;
                pGraphicsCaps->minorSMVersion = 6; // Take the risk that no one uses an NVIDIA A100 with this implementation
            } else if (architectureId == NV_GPU_ARCHITECTURE_TU100) {
                pGraphicsCaps->majorSMVersion = 7;
                pGraphicsCaps->minorSMVersion = 5;
            } else if (architectureId == NV_GPU_ARCHITECTURE_GV100) {
                pGraphicsCaps->majorSMVersion = 7;
                pGraphicsCaps->minorSMVersion = 0;
            } else if (architectureId == NV_GPU_ARCHITECTURE_GP100) {
                pGraphicsCaps->majorSMVersion = 6;
                pGraphicsCaps->minorSMVersion = 0;
            } else if (architectureId == NV_GPU_ARCHITECTURE_GM200) {
                pGraphicsCaps->majorSMVersion = 5;
                pGraphicsCaps->minorSMVersion = 2;
            } else if (architectureId == NV_GPU_ARCHITECTURE_GM000) {
                pGraphicsCaps->majorSMVersion = 5;
                pGraphicsCaps->minorSMVersion = 0;
            }
        }

        // Might be related to VK_NV_scissor_exclusive (which isn't used by VKD3D-Proton), but unknown in the context of D3D12
        // pGraphicsCaps->bExclusiveScissorRectsSupported = adapter->IsVkDeviceExtensionSupported(VK_NV_SCISSOR_EXCLUSIVE_EXTENSION_NAME);

        // Note that adapter->IsVkDeviceExtensionSupported returns the extensions supported by DXVK, not by VKD3D-Proton,
        // so we might be wrong here in case of an old VKD3D-Proton version or when VKD3D_DISABLE_EXTENSIONS is in use
        pGraphicsCaps->bVariablePixelRateShadingSupported = adapter->IsVkDeviceExtensionSupported(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);

        return Ok(str::format(n, " (sm_", pGraphicsCaps->majorSMVersion, pGraphicsCaps->minorSMVersion, ")"));
    }

    NvAPI_Status __cdecl NvAPI_D3D12_IsFatbinPTXSupported(ID3D12Device* pDevice, bool* isSupported) {
        constexpr auto n = __func__;
        thread_local bool alreadyLoggedNoImplementation = false;
        thread_local bool alreadyLoggedError = false;
        thread_local bool alreadyLoggedOk = false;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(pDevice), log::fmt::ptr(isSupported));

        if (!pDevice || !isSupported)
            return InvalidArgument(n);

        auto device = NvapiD3d12Device::GetOrCreate(pDevice);
        if (!device)
            return NoImplementation(n, alreadyLoggedNoImplementation);

        *isSupported = device->IsFatbinPTXSupported();
        if (!*isSupported)
            return Error(n, alreadyLoggedError);

        return Ok(str::format(n, "(", *isSupported ? "Supported" : "Unsupported", ")"), alreadyLoggedOk);
    }

    bool CreateGraphicsPipelineState(ID3D12Device* device, const D3D12_GRAPHICS_PIPELINE_STATE_DESC* pipelineStateDescription, NvU32 numberOfExtensions, const NVAPI_D3D12_PSO_EXTENSION_DESC** extensions, ID3D12PipelineState** pipelineState) {
        if (numberOfExtensions != 1 || extensions[0]->psoExtension != NV_PSO_ENABLE_DEPTH_BOUND_TEST_EXTENSION)
            return false;

        return SUCCEEDED(device->CreateGraphicsPipelineState(pipelineStateDescription, __uuidof(ID3D12PipelineState), reinterpret_cast<void**>(pipelineState)));
    }

    NvAPI_Status __cdecl NvAPI_D3D12_CreateGraphicsPipelineState(ID3D12Device* pDevice, const D3D12_GRAPHICS_PIPELINE_STATE_DESC* pPSODesc, NvU32 numExtensions, const NVAPI_D3D12_PSO_EXTENSION_DESC** ppExtensions, ID3D12PipelineState** ppPSO) {
        constexpr auto n = __func__;
        thread_local bool alreadyLoggedOk = false;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(pDevice), log::fmt::ptr(pPSODesc), numExtensions, log::fmt::ptr(ppExtensions), log::fmt::ptr(ppPSO));

        if (!pDevice || !pPSODesc || !ppExtensions || !ppPSO)
            return InvalidArgument(n);

        if (numExtensions == 0)
            return InvalidArgument(n);

        if (ppExtensions[0]->baseVersion != NV_PSO_EXTENSION_DESC_VER_1)
            return IncompatibleStructVersion(n, ppExtensions[0]->baseVersion);

        std::string extensionNames;
        for (auto i = 0U; i < numExtensions; i++)
            extensionNames += str::format(fromPsoExtension(ppExtensions[i]->psoExtension), ",");

        extensionNames.pop_back();

        if (!CreateGraphicsPipelineState(pDevice, pPSODesc, numExtensions, ppExtensions, ppPSO))
            return NotSupported(str::format(n, " (", numExtensions, "/", extensionNames, ")"));

        return Ok(str::format(n, " (", numExtensions, "/", extensionNames, ")"), alreadyLoggedOk);
    }

    NvAPI_Status __cdecl NvAPI_D3D12_SetNvShaderExtnSlotSpace(IUnknown* pDevice, NvU32 uavSlot, NvU32 uavSpace) {
        constexpr auto n = __func__;
        thread_local bool alreadyLoggedNoImplementation = false;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(pDevice), uavSlot, uavSpace);

        if (!pDevice)
            return InvalidArgument(n);

        return NoImplementation(n, alreadyLoggedNoImplementation);
    }

    NvAPI_Status __cdecl NvAPI_D3D12_SetNvShaderExtnSlotSpaceLocalThread(IUnknown* pDev, NvU32 uavSlot, NvU32 uavSpace) {
        constexpr auto n = __func__;
        thread_local bool alreadyLoggedNoImplementation = false;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(pDev), uavSlot, uavSpace);

        if (!pDev)
            return InvalidArgument(n);

        return NoImplementation(n, alreadyLoggedNoImplementation);
    }

    bool SetDepthBoundsTestValues(ID3D12GraphicsCommandList* commandList, const float minDepth, const float maxDepth) {
        Com<ID3D12GraphicsCommandList1> commandList1;
        if (FAILED(commandList->QueryInterface(IID_PPV_ARGS(&commandList1)))) // There is no VKD3D-Proton version out there that does not implement ID3D12GraphicsCommandList1, this should always succeed
            return false;

        commandList1->OMSetDepthBounds(minDepth, maxDepth);
        return true;
    }

    NvAPI_Status __cdecl NvAPI_D3D12_SetDepthBoundsTestValues(ID3D12GraphicsCommandList* pCommandList, const float minDepth, const float maxDepth) {
        constexpr auto n = __func__;
        thread_local bool alreadyLoggedError = false;
        thread_local bool alreadyLoggedOk = false;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(pCommandList), log::fmt::flt(minDepth), log::fmt::flt(maxDepth));

        if (!pCommandList)
            return InvalidArgument(n);

        if (!SetDepthBoundsTestValues(pCommandList, minDepth, maxDepth))
            return Error(n, alreadyLoggedError);

        return Ok(n, alreadyLoggedOk);
    }

    NvAPI_Status __cdecl NvAPI_D3D12_GetRaytracingCaps(ID3D12Device* pDevice, NVAPI_D3D12_RAYTRACING_CAPS_TYPE type, void* pData, size_t dataSize) {
        constexpr auto n = __func__;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(pDevice), type, log::fmt::ptr(pData), dataSize);

        if (!pDevice || !pData)
            return InvalidPointer(n);

        switch (type) {
            case NVAPI_D3D12_RAYTRACING_CAPS_TYPE_THREAD_REORDERING:
                if (dataSize != sizeof(NVAPI_D3D12_RAYTRACING_THREAD_REORDERING_CAPS))
                    return InvalidArgument(n);

                // let's hope that NvAPI_D3D12_IsNvShaderExtnOpCodeSupported returning false is enough to discourage games from attempting to use Shader Execution Reordering
                *static_cast<NVAPI_D3D12_RAYTRACING_THREAD_REORDERING_CAPS*>(pData) = NVAPI_D3D12_RAYTRACING_THREAD_REORDERING_CAP_NONE;
                break;

            case NVAPI_D3D12_RAYTRACING_CAPS_TYPE_OPACITY_MICROMAP:
                if (dataSize != sizeof(NVAPI_D3D12_RAYTRACING_OPACITY_MICROMAP_CAPS))
                    return InvalidArgument(n);

                *static_cast<NVAPI_D3D12_RAYTRACING_OPACITY_MICROMAP_CAPS*>(pData) = NVAPI_D3D12_RAYTRACING_OPACITY_MICROMAP_CAP_NONE;
                break;

            case NVAPI_D3D12_RAYTRACING_CAPS_TYPE_DISPLACEMENT_MICROMAP:
                if (dataSize != sizeof(NVAPI_D3D12_RAYTRACING_DISPLACEMENT_MICROMAP_CAPS))
                    return InvalidArgument(n);

                *static_cast<NVAPI_D3D12_RAYTRACING_DISPLACEMENT_MICROMAP_CAPS*>(pData) = NVAPI_D3D12_RAYTRACING_DISPLACEMENT_MICROMAP_CAP_NONE;
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
                        log::info("Triangles with OMM attachment passed to acceleration structure build when OMM is not supported");
                        return false;
                    case NVAPI_D3D12_RAYTRACING_GEOMETRY_TYPE_DMM_TRIANGLES_EX: // GetRaytracingCaps reports no DMM caps, we shouldn't reach this
                        log::info("Triangles with DMM attachment passed to acceleration structure build when DMM is not supported");
                        return false;
                    default:
                        log::info(str::format("Unknown NVAPI_D3D12_RAYTRACING_GEOMETRY_TYPE_EX: ", nvGeoDesc.type));
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
        thread_local bool alreadyLoggedOk = false;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(pDevice), log::fmt::ptr(pParams));

        if (!pDevice || !pParams)
            return InvalidArgument(n);

        if (pParams->version != NVAPI_GET_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO_EX_PARAMS_VER1)
            return IncompatibleStructVersion(n, pParams->version);

        if (!pParams->pDesc || !pParams->pInfo)
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
        thread_local bool alreadyLoggedOk = false;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(pCommandList), log::fmt::ptr(pParams));

        if (!pCommandList || !pParams)
            return InvalidArgument(n);

        if (pParams->version != NVAPI_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_EX_PARAMS_VER1)
            return IncompatibleStructVersion(n, pParams->version);

        if (!pParams->pDesc || (pParams->numPostbuildInfoDescs != 0 && !pParams->pPostbuildInfoDescs))
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
        thread_local bool alreadyLoggedNoImplementation = false;
        thread_local bool alreadyLoggedTypeIgnore = false;
        thread_local bool alreadyLoggedTypeRenderPresent = false;
        thread_local bool alreadyLoggedError = false;
        thread_local bool alreadyLoggedOk = false;

        if (log::tracing())
            log::trace(n, cqType);

        if (!nvapiAdapterRegistry)
            return ApiNotInitialized(n);

        if (!pCommandQueue)
            return InvalidPointer(n);

        auto lowLatencyDevice = NvapiD3dLowLatencyDevice::GetOrCreate(pCommandQueue);
        if (!lowLatencyDevice || !lowLatencyDevice->SupportsLowLatency())
            return NoImplementation(n);

        if (cqType == OUT_OF_BAND_IGNORE && !std::exchange(alreadyLoggedTypeIgnore, true))
            log::info("NvAPI_D3D12_NotifyOutOfBandCommandQueue is called with OUT_OF_BAND_IGNORE");

        if (cqType == OUT_OF_BAND_RENDER_PRESENT && !std::exchange(alreadyLoggedTypeRenderPresent, true))
            log::info("NvAPI_D3D12_NotifyOutOfBandCommandQueue is called with OUT_OF_BAND_RENDER_PRESENT");

        auto device = NvapiD3d12CommandQueue::GetOrCreate(pCommandQueue);
        if (!device)
            return NoImplementation(n, alreadyLoggedNoImplementation);

        switch (device->NotifyOutOfBandCommandQueue(static_cast<D3D12_OUT_OF_BAND_CQ_TYPE>(cqType))) {
            case S_OK:
                return Ok(n, alreadyLoggedOk);
            case E_NOTIMPL:
                return NoImplementation(n, alreadyLoggedNoImplementation);
            default:
                return Error(n, alreadyLoggedError);
        }
    }

    NvAPI_Status __cdecl NvAPI_D3D12_SetAsyncFrameMarker(ID3D12CommandQueue* pCommandQueue, NV_ASYNC_FRAME_MARKER_PARAMS* pSetAsyncFrameMarkerParams) {
        constexpr auto n = __func__;
        thread_local bool alreadyLoggedNoImplementation = false;
        thread_local bool alreadyLoggedError = false;
        thread_local bool alreadyLoggedOk = false;
        thread_local bool alreadyLoggedMarkerTypeNotSupported = false;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(pCommandQueue), log::fmt::nv_async_frame_marker_params(pSetAsyncFrameMarkerParams));

        if (!nvapiAdapterRegistry)
            return ApiNotInitialized(n);

        if (!pCommandQueue || !pSetAsyncFrameMarkerParams)
            return InvalidPointer(n);

        if (pSetAsyncFrameMarkerParams->version != NV_LATENCY_MARKER_PARAMS_VER1)
            return IncompatibleStructVersion(n, pSetAsyncFrameMarkerParams->version);

        auto lowLatencyDevice = NvapiD3dLowLatencyDevice::GetOrCreate(pCommandQueue);
        if (!lowLatencyDevice || !lowLatencyDevice->SupportsLowLatency())
            return NoImplementation(n, alreadyLoggedNoImplementation);

        auto markerType = NvapiD3dLowLatencyDevice::ToMarkerType(pSetAsyncFrameMarkerParams->markerType);
        if (!markerType.has_value()) {
            // Silently drop unsupported marker types
            if (!std::exchange(alreadyLoggedMarkerTypeNotSupported, true))
                log::info(str::format("Not supported NV_LATENCY_MARKER_TYPE: ", pSetAsyncFrameMarkerParams->markerType));

            return Ok(n, alreadyLoggedOk);
        }

        switch (lowLatencyDevice->SetLatencyMarker(pSetAsyncFrameMarkerParams->frameID, markerType.value())) {
            case S_OK:
                return Ok(n, alreadyLoggedOk);
            case E_NOTIMPL:
                return NoImplementation(n, alreadyLoggedNoImplementation);
            default:
                return Error(n, alreadyLoggedError);
        }
    }
}
