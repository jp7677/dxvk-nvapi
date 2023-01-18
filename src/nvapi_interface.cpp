#include "nvapi_private.h"
#include "../inc/nvapi_interface.h"
#include "util/util_string.h"
#include "util/util_log.h"

extern "C" {
    using namespace dxvk;

    static std::unordered_map<NvU32, void*> registry;

    void* nvapi_QueryInterface(NvU32 id) {
        auto entry = registry.find(id);
        if (entry != registry.end())
            return entry->second;

        auto it = std::find_if(
            std::begin(nvapi_interface_table),
            std::end(nvapi_interface_table),
            [id](const auto& item) { return item.id == id; });

        if (it == std::end(nvapi_interface_table)) {
            log::write(str::format("NvAPI_QueryInterface (0x", std::hex, id, "): Unknown function ID"));
            return registry.insert({id, nullptr}).first->second;
        }

#define INSERT_AND_RETURN_WHEN_EQUALS(method) \
    if (std::string(it->func) == #method)     \
        return registry.insert({id, (void*)method}).first->second;

        // This block will be validated for completeness when running package-release.sh. Do not remove the comments.
        /* Start NVAPI methods */
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D11_SetDepthBoundsTest)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D11_BeginUAVOverlap)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D11_EndUAVOverlap)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D11_MultiDrawInstancedIndirect)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D11_MultiDrawIndexedInstancedIndirect)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D11_IsNvShaderExtnOpCodeSupported)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D11_CreateCubinComputeShader)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D11_CreateCubinComputeShaderWithName)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D11_LaunchCubinShader)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D11_DestroyCubinComputeShader)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D11_IsFatbinPTXSupported)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D11_CreateUnorderedAccessView)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D11_CreateSamplerState)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D11_GetCudaTextureObject)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D11_CreateShaderResourceView)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D11_GetResourceHandle)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D11_GetResourceGPUVirtualAddress)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D11_GetResourceGPUVirtualAddressEx)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D1x_GetGraphicsCapabilities)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D12_IsNvShaderExtnOpCodeSupported)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D12_EnumerateMetaCommands)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D12_CreateGraphicsPipelineState)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D12_SetDepthBoundsTestValues)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D12_CreateCubinComputeShaderWithName)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D12_CreateCubinComputeShader)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D12_DestroyCubinComputeShader)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D12_GetCudaTextureObject)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D12_GetCudaSurfaceObject)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D12_LaunchCubinShader)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D12_CaptureUAVInfo)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D12_GetGraphicsCapabilities)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D12_IsFatbinPTXSupported)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D_GetObjectHandleForResource)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D_SetResourceHint)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D_GetCurrentSLIState)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D_ImplicitSLIControl)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D_BeginResourceRendering)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D_EndResourceRendering)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D_SetSleepMode)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D_GetSleepStatus)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D_Sleep)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D_GetLatency)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D_SetLatencyMarker)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetConnectedDisplayIds)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetCurrentPCIEDownstreamWidth)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetIRQ)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetGpuCoreCount)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetGPUType)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetPCIIdentifiers)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetFullName)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetBusId)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetBusSlotId)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetBusType)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetPhysicalFrameBufferSize)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetAdapterIdFromPhysicalGpu)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetLogicalGpuInfo)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetArchInfo)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_CudaEnumComputeCapableGpus)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetDynamicPstatesInfoEx)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetThermalSettings)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetVbiosVersionString)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetCurrentPstate)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetPstates20)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetAllClockFrequencies)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_DRS_FindApplicationByName)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_DRS_FindProfileByName)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_DRS_GetSetting)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_DRS_GetBaseProfile)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_DRS_LoadSettings)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_DRS_DestroySession)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_DRS_CreateSession)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_Disp_GetHdrCapabilities)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_DISP_GetDisplayIdByDisplayName)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_DISP_GetGDIPrimaryDisplayId)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_Mosaic_GetDisplayViewportsByResolution)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_SYS_GetPhysicalGpuFromDisplayId)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_SYS_GetDriverAndBranchVersion)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_EnumLogicalGPUs)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_EnumPhysicalGPUs)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_EnumTCCPhysicalGPUs)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GetPhysicalGPUFromGPUID)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GetGPUIDfromPhysicalGPU)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GetDisplayDriverVersion)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GetPhysicalGPUsFromDisplay)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GetPhysicalGPUsFromLogicalGPU)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_EnumNvidiaDisplayHandle)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_EnumNvidiaUnAttachedDisplayHandle)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GetAssociatedNvidiaDisplayName)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GetInterfaceVersionString)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GetErrorMessage)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_Unload)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_Initialize)
        /* End */

#undef INSERT_AND_RETURN_WHEN_EQUALS

        log::write(str::format("NvAPI_QueryInterface ", it->func, ": Not implemented method"));
        return registry.insert({id, nullptr}).first->second;
    }
}
