#include "nvapi_private.h"
#include "d3d12/nvapi_d3d12_device.h"
#include "util/util_statuscode.h"
#include "util/util_op_code.h"
#include "util/util_string.h"

extern "C" {
    using namespace dxvk;

    NvAPI_Status __cdecl NvAPI_D3D12_IsNvShaderExtnOpCodeSupported(ID3D12Device *pDevice, NvU32 opCode, bool *pSupported) {
        constexpr auto n = __func__;

        if (pDevice == nullptr || pSupported == nullptr)
                return InvalidArgument(n);

        // VKD3D does not know any NVIDIA intrinsics
        *pSupported = false;

        return Ok(str::format(n, " ", opCode, " (", fromCode(opCode), ")"));
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
        return NvAPI_D3D12_CreateCubinComputeShaderWithName(pDevice, cubinData, cubinSize, blockX, blockY, blockZ, "", pShader);
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
        static bool alreadyLogged = false;

        if (pDevice == nullptr || structVersion != NV_D3D12_GRAPHICS_CAPS_VER1)
            return InvalidArgument(n);

        // Currently we do not require these fields, we can implement them later.
        // pGraphicsCaps->bExclusiveScissorRectsSupported      = ;
        // pGraphicsCaps->bVariablePixelRateShadingSupported   = ;
        // pGraphicsCaps->majorSMVersion                       = ;
        // pGraphicsCaps->minorSMVersion                       = ;

        // all Vulkan drivers are expected to support ZBC clear without padding
        pGraphicsCaps->bFastUAVClearSupported               = true;

        return Ok(n, alreadyLogged);
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
}
