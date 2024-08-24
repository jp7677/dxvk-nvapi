#include "nvapi_private.h"
#include "nvapi_globals.h"
#include "d3d11/nvapi_d3d11_device.h"
#include "util/util_statuscode.h"
#include "util/util_op_code.h"
#include "util/util_string.h"

extern "C" {
    using namespace dxvk;

    NvAPI_Status __cdecl NvAPI_D3D11_CreateDevice(IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, CONST D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, ID3D11Device** ppDevice, D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext, NVAPI_DEVICE_FEATURE_LEVEL* pSupportedLevel) {
        constexpr auto n = __func__;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(pAdapter), DriverType, Software, log::fmt::flags(Flags), log::fmt::ptr(pFeatureLevels), FeatureLevels, SDKVersion, log::fmt::ptr(ppDevice), log::fmt::ptr(pFeatureLevel), log::fmt::ptr(ppImmediateContext), log::fmt::ptr(pSupportedLevel));

        if (pSupportedLevel == nullptr)
            return InvalidArgument(n);

        if (FAILED(::D3D11CreateDevice(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, ppDevice, pFeatureLevel, ppImmediateContext)))
            return Error(n);

        *pSupportedLevel = NVAPI_DEVICE_FEATURE_LEVEL_11_0; // We are safe to always return 11_0 since our minimum required DXVK version needs Maxwell or newer

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_D3D11_CreateDeviceAndSwapChain(IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, CONST D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, CONST DXGI_SWAP_CHAIN_DESC* pSwapChainDesc, IDXGISwapChain** ppSwapChain, ID3D11Device** ppDevice, D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext, NVAPI_DEVICE_FEATURE_LEVEL* pSupportedLevel) {
        constexpr auto n = __func__;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(pAdapter), DriverType, Software, log::fmt::flags(Flags), log::fmt::ptr(pFeatureLevels), FeatureLevels, SDKVersion, log::fmt::ptr(pSwapChainDesc), log::fmt::ptr(ppSwapChain), log::fmt::ptr(ppDevice), log::fmt::ptr(pFeatureLevel), log::fmt::ptr(ppImmediateContext), log::fmt::ptr(pSupportedLevel));

        if (pSupportedLevel == nullptr)
            return InvalidArgument(n);

        if (FAILED(::D3D11CreateDeviceAndSwapChain(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, pSwapChainDesc, ppSwapChain, ppDevice, pFeatureLevel, ppImmediateContext)))
            return Error(n);

        *pSupportedLevel = NVAPI_DEVICE_FEATURE_LEVEL_11_0; // We are safe to always return 11_0 since our minimum required DXVK version needs Maxwell or newer

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_D3D11_IsNvShaderExtnOpCodeSupported(IUnknown* pDeviceOrContext, NvU32 code, bool* supported) {
        constexpr auto n = __func__;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(pDeviceOrContext), code, log::fmt::ptr(supported));

        if (pDeviceOrContext == nullptr || supported == nullptr)
            return InvalidArgument(n);

        // DXVK does not know any NVIDIA intrinsics backdoors
        *supported = false;

        return Ok(str::format(n, " (", code, "/", fromCode(code), ")"));
    }

    NvAPI_Status __cdecl NvAPI_D3D11_MultiGPU_GetCaps(PNV_MULTIGPU_CAPS pMultiGPUCaps) {
        constexpr auto n = __func__;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(pMultiGPUCaps));

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (pMultiGPUCaps == nullptr)
            return InvalidArgument(n);

        switch (pMultiGPUCaps->version) {
            case 0: // NV_MULTIGPU_CAPS_V1 has no version field
            case NV_MULTIGPU_CAPS_VER2:
                [[fallthrough]];
            case NV_MULTIGPU_CAPS_VER1:
                *pMultiGPUCaps = {};
                // Report that SLI is not available
                pMultiGPUCaps->nTotalGPUs = nvapiAdapterRegistry->GetAdapterCount();
                pMultiGPUCaps->nSLIGPUs = 0;
                break;
            default:
                return IncompatibleStructVersion(n, pMultiGPUCaps->version);
        }

        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_D3D11_MultiGPU_Init(bool bEnable) {
        constexpr auto n = __func__;

        if (log::tracing())
            log::trace(n, bEnable);

        // Just acknowledge the request since there is nothing to do here
        return Ok(n);
    }

    NvAPI_Status __cdecl NvAPI_D3D11_SetDepthBoundsTest(IUnknown* pDeviceOrContext, NvU32 bEnable, float fMinDepth, float fMaxDepth) {
        constexpr auto n = __func__;
        thread_local bool alreadyLoggedError = false;
        thread_local bool alreadyLoggedOk = false;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(pDeviceOrContext), bEnable, log::fmt::flt(fMinDepth), log::fmt::flt(fMaxDepth));

        if (pDeviceOrContext == nullptr)
            return InvalidArgument(n);

        if (!NvapiD3d11Device::SetDepthBoundsTest(pDeviceOrContext, bEnable, fMinDepth, fMaxDepth))
            return Error(n, alreadyLoggedError);

        return Ok(n, alreadyLoggedOk);
    }

    NvAPI_Status __cdecl NvAPI_D3D11_BeginUAVOverlap(IUnknown* pDeviceOrContext) {
        constexpr auto n = __func__;
        thread_local bool alreadyLoggedError = false;
        thread_local bool alreadyLoggedOk = false;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(pDeviceOrContext));

        if (pDeviceOrContext == nullptr)
            return InvalidArgument(n);

        if (!NvapiD3d11Device::BeginUAVOverlap(pDeviceOrContext))
            return Error(n, alreadyLoggedError);

        return Ok(n, alreadyLoggedOk);
    }

    NvAPI_Status __cdecl NvAPI_D3D11_EndUAVOverlap(IUnknown* pDeviceOrContext) {
        constexpr auto n = __func__;
        thread_local bool alreadyLoggedError = false;
        thread_local bool alreadyLoggedOk = false;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(pDeviceOrContext));

        if (pDeviceOrContext == nullptr)
            return InvalidArgument(n);

        if (!NvapiD3d11Device::EndUAVOverlap(pDeviceOrContext))
            return Error(n, alreadyLoggedError);

        return Ok(n, alreadyLoggedOk);
    }

    NvAPI_Status __cdecl NvAPI_D3D11_MultiDrawInstancedIndirect(ID3D11DeviceContext* pDevContext11, NvU32 drawCount, ID3D11Buffer* pBuffer, NvU32 alignedByteOffsetForArgs, NvU32 alignedByteStrideForArgs) {
        constexpr auto n = __func__;
        thread_local bool alreadyLoggedError = false;
        thread_local bool alreadyLoggedOk = false;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(pDevContext11), drawCount, log::fmt::ptr(pBuffer), alignedByteOffsetForArgs, alignedByteStrideForArgs);

        if (pDevContext11 == nullptr || pBuffer == nullptr)
            return InvalidArgument(n);

        if (!NvapiD3d11Device::MultiDrawInstancedIndirect(pDevContext11, drawCount, pBuffer, alignedByteOffsetForArgs, alignedByteStrideForArgs))
            return Error(n, alreadyLoggedError);

        return Ok(n, alreadyLoggedOk);
    }

    NvAPI_Status __cdecl NvAPI_D3D11_MultiDrawIndexedInstancedIndirect(ID3D11DeviceContext* pDevContext11, NvU32 drawCount, ID3D11Buffer* pBuffer, NvU32 alignedByteOffsetForArgs, NvU32 alignedByteStrideForArgs) {
        constexpr auto n = __func__;
        thread_local bool alreadyLoggedError = false;
        thread_local bool alreadyLoggedOk = false;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(pDevContext11), drawCount, log::fmt::ptr(pBuffer), alignedByteOffsetForArgs, alignedByteStrideForArgs);

        if (pDevContext11 == nullptr || pBuffer == nullptr)
            return InvalidArgument(n);

        if (!NvapiD3d11Device::MultiDrawIndexedInstancedIndirect(pDevContext11, drawCount, pBuffer, alignedByteOffsetForArgs, alignedByteStrideForArgs))
            return Error(n, alreadyLoggedError);

        return Ok(n, alreadyLoggedOk);
    }

    NvAPI_Status __cdecl NvAPI_D3D11_CreateCubinComputeShader(ID3D11Device* pDevice, const void* pCubin, NvU32 size, NvU32 blockX, NvU32 blockY, NvU32 blockZ, NVDX_ObjectHandle* phShader) {
        constexpr auto n = __func__;
        thread_local bool alreadyLoggedError = false;
        thread_local bool alreadyLoggedOk = false;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(pDevice), log::fmt::ptr(pCubin), size, blockX, blockY, blockZ, log::fmt::ptr(phShader));

        if (pDevice == nullptr || pCubin == nullptr || phShader == nullptr)
            return InvalidArgument(n);

        if (!NvapiD3d11Device::CreateCubinComputeShaderWithName(pDevice, pCubin, size, blockX, blockY, blockZ, "", phShader))
            return Error(n, alreadyLoggedError);

        return Ok(n, alreadyLoggedOk);
    }

    NvAPI_Status __cdecl NvAPI_D3D11_CreateCubinComputeShaderWithName(ID3D11Device* pDevice, const void* pCubin, NvU32 size, NvU32 blockX, NvU32 blockY, NvU32 blockZ, const char* pShaderName, NVDX_ObjectHandle* phShader) {
        constexpr auto n = __func__;
        thread_local bool alreadyLoggedError = false;
        thread_local bool alreadyLoggedOk = false;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(pDevice), log::fmt::ptr(pCubin), size, blockX, blockY, blockZ, log::fmt::ptr(pShaderName), log::fmt::ptr(phShader));

        if (pDevice == nullptr || pCubin == nullptr || pShaderName == nullptr || phShader == nullptr)
            return InvalidArgument(n);

        if (!NvapiD3d11Device::CreateCubinComputeShaderWithName(pDevice, pCubin, size, blockX, blockY, blockZ, pShaderName, phShader))
            return Error(n, alreadyLoggedError);

        return Ok(n, alreadyLoggedOk);
    }

    NvAPI_Status __cdecl NvAPI_D3D11_LaunchCubinShader(ID3D11DeviceContext* pDeviceContext, NVDX_ObjectHandle hShader, NvU32 gridX, NvU32 gridY, NvU32 gridZ, const void* pParams, NvU32 paramSize, const NVDX_ObjectHandle* pReadResources, NvU32 numReadResources, const NVDX_ObjectHandle* pWriteResources, NvU32 numWriteResources) {
        constexpr auto n = __func__;
        thread_local bool alreadyLoggedError = false;
        thread_local bool alreadyLoggedOk = false;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(pDeviceContext), log::fmt::hnd(hShader), gridX, gridY, gridZ, log::fmt::ptr(pParams), paramSize, log::fmt::ptr(pReadResources), numReadResources, log::fmt::ptr(pWriteResources), numWriteResources);

        if (pDeviceContext == nullptr)
            return InvalidArgument(n);

        if (!NvapiD3d11Device::LaunchCubinShader(pDeviceContext, hShader, gridX, gridY, gridZ, pParams, paramSize, pReadResources, numReadResources, pWriteResources, numWriteResources))
            return Error(n, alreadyLoggedError);

        return Ok(n, alreadyLoggedOk);
    }

    NvAPI_Status __cdecl NvAPI_D3D11_DestroyCubinComputeShader(ID3D11Device* pDevice, NVDX_ObjectHandle hShader) {
        constexpr auto n = __func__;
        thread_local bool alreadyLoggedError = false;
        thread_local bool alreadyLoggedOk = false;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(pDevice), log::fmt::hnd(hShader));

        if (pDevice == nullptr)
            return InvalidArgument(n);

        if (!NvapiD3d11Device::DestroyCubinShader(hShader))
            return Error(n, alreadyLoggedError);

        return Ok(n, alreadyLoggedOk);
    }

    NvAPI_Status __cdecl NvAPI_D3D11_IsFatbinPTXSupported(ID3D11Device* pDevice, bool* pSupported) {
        constexpr auto n = __func__;
        thread_local bool alreadyLoggedOk = false;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(pDevice), log::fmt::ptr(pSupported));

        if (pDevice == nullptr || pSupported == nullptr)
            return InvalidArgument(n);

        *pSupported = NvapiD3d11Device::IsFatbinPTXSupported(pDevice);

        return Ok(str::format(n, "(", *pSupported ? "Supported" : "Unsupported", ")"), alreadyLoggedOk);
    }

    NvAPI_Status __cdecl NvAPI_D3D11_CreateUnorderedAccessView(ID3D11Device* pDevice, ID3D11Resource* pResource, const D3D11_UNORDERED_ACCESS_VIEW_DESC* pDesc, ID3D11UnorderedAccessView** ppUAV, NvU32* pDriverHandle) {
        constexpr auto n = __func__;
        thread_local bool alreadyLoggedError = false;
        thread_local bool alreadyLoggedOk = false;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(pDevice), log::fmt::ptr(pResource), log::fmt::ptr(pDesc), log::fmt::ptr(ppUAV), log::fmt::ptr(pDriverHandle));

        if (pDevice == nullptr || pResource == nullptr || pDesc == nullptr || ppUAV == nullptr || pDriverHandle == nullptr)
            return InvalidArgument(n);

        if (!NvapiD3d11Device::CreateUnorderedAccessViewAndGetDriverHandle(pDevice, pResource, pDesc, ppUAV, pDriverHandle))
            return Error(n, alreadyLoggedError);

        return Ok(n, alreadyLoggedOk);
    }

    NvAPI_Status __cdecl NvAPI_D3D11_CreateShaderResourceView(ID3D11Device* pDevice, ID3D11Resource* pResource, const D3D11_SHADER_RESOURCE_VIEW_DESC* pDesc, ID3D11ShaderResourceView** ppSRV, NvU32* pDriverHandle) {
        constexpr auto n = __func__;
        thread_local bool alreadyLoggedError = false;
        thread_local bool alreadyLoggedOk = false;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(pDevice), log::fmt::ptr(pResource), log::fmt::ptr(pDesc), log::fmt::ptr(ppSRV), log::fmt::ptr(pDriverHandle));

        if (pDevice == nullptr || pResource == nullptr || pDesc == nullptr || ppSRV == nullptr || pDriverHandle == nullptr)
            return InvalidArgument(n);

        if (!NvapiD3d11Device::CreateShaderResourceViewAndGetDriverHandle(pDevice, pResource, pDesc, ppSRV, pDriverHandle))
            return Error(n, alreadyLoggedError);

        return Ok(n, alreadyLoggedOk);
    }

    NvAPI_Status __cdecl NvAPI_D3D11_GetResourceHandle(ID3D11Device* pDevice, ID3D11Resource* pResource, NVDX_ObjectHandle* phObject) {
        constexpr auto n = __func__;
        thread_local bool alreadyLoggedOk = false;
        thread_local bool alreadyLoggedError = false;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(pDevice), log::fmt::ptr(pResource), log::fmt::ptr(phObject));

        if (pDevice == nullptr || pResource == nullptr || phObject == nullptr)
            return InvalidArgument(n);

        if (!NvapiD3d11Device::GetResourceDriverHandle(pResource, phObject))
            return Error(n, alreadyLoggedError);

        return Ok(n, alreadyLoggedOk);
    }

    NvAPI_Status __cdecl NvAPI_D3D11_GetResourceGPUVirtualAddress(ID3D11Device* pDevice, const NVDX_ObjectHandle hResource, NvU64* pGpuVA) {
        constexpr auto n = __func__;
        thread_local bool alreadyLoggedOk = false;
        thread_local bool alreadyLoggedError = false;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(pDevice), log::fmt::hnd(hResource), log::fmt::ptr(pGpuVA));

        if (pDevice == nullptr || hResource == NVDX_OBJECT_NONE || pGpuVA == nullptr)
            return InvalidArgument(n);

        NvU64 dummy;
        if (!NvapiD3d11Device::GetResourceHandleGPUVirtualAddressAndSize(pDevice, hResource, pGpuVA, &dummy))
            return Error(n, alreadyLoggedError);

        return Ok(n, alreadyLoggedOk);
    }

    NvAPI_Status __cdecl NvAPI_D3D11_GetResourceGPUVirtualAddressEx(ID3D11Device* pDevice, NV_GET_GPU_VIRTUAL_ADDRESS* pParams) {
        constexpr auto n = __func__;
        thread_local bool alreadyLoggedOk = false;
        thread_local bool alreadyLoggedError = false;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(pDevice), log::fmt::ptr(pParams));

        if (pDevice == nullptr || pParams == nullptr)
            return InvalidArgument(n);

        if (pParams->version != NV_GET_GPU_VIRTUAL_ADDRESS_VER1)
            return IncompatibleStructVersion(n, pParams->version);

        if (pParams->hResource == NVDX_OBJECT_NONE)
            return InvalidArgument(n);

        if (!NvapiD3d11Device::GetResourceHandleGPUVirtualAddressAndSize(pDevice, pParams->hResource, &pParams->gpuVAStart, &pParams->gpuVASize))
            return Error(n, alreadyLoggedError);

        return Ok(n, alreadyLoggedOk);
    }

    NvAPI_Status __cdecl NvAPI_D3D11_CreateSamplerState(ID3D11Device* pDevice, const D3D11_SAMPLER_DESC* pSamplerDesc, ID3D11SamplerState** ppSamplerState, NvU32* pDriverHandle) {
        constexpr auto n = __func__;
        thread_local bool alreadyLoggedError = false;
        thread_local bool alreadyLoggedOk = false;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(pDevice), log::fmt::ptr(pSamplerDesc), log::fmt::ptr(ppSamplerState), log::fmt::ptr(pDriverHandle));

        if (pDevice == nullptr || pSamplerDesc == nullptr || ppSamplerState == nullptr || pDriverHandle == nullptr)
            return InvalidArgument(n);

        if (!NvapiD3d11Device::CreateSamplerStateAndGetDriverHandle(pDevice, pSamplerDesc, ppSamplerState, reinterpret_cast<uint32_t*>(pDriverHandle)))
            return Error(n, alreadyLoggedError);

        return Ok(n, alreadyLoggedOk);
    }

    NvAPI_Status __cdecl NvAPI_D3D11_GetCudaTextureObject(ID3D11Device* pDevice, NvU32 srvDriverHandle, NvU32 samplerDriverHandle, NvU32* pCudaTextureHandle) {
        constexpr auto n = __func__;
        thread_local bool alreadyLoggedError = false;
        thread_local bool alreadyLoggedOk = false;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(pDevice), srvDriverHandle, samplerDriverHandle, log::fmt::ptr(pCudaTextureHandle));

        if (pDevice == nullptr || pCudaTextureHandle == nullptr)
            return InvalidArgument(n);

        if (!NvapiD3d11Device::GetCudaTextureObject(pDevice, srvDriverHandle, samplerDriverHandle, reinterpret_cast<uint32_t*>(pCudaTextureHandle)))
            return Error(n, alreadyLoggedError);

        return Ok(n, alreadyLoggedOk);
    }
}
