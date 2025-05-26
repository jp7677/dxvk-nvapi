#include "nvapi_private.h"
#include "nvapi_interface.h"
#include "util/util_string.h"
#include "util/util_log.h"

extern "C" {
    using namespace dxvk;

    static const auto disabledEnvName = "DXVK_NVAPI_DISABLE_ENTRYPOINTS";
    static const auto disabledString = str::fromnullable(std::getenv(disabledEnvName));
    static const auto disabled = str::split<std::set<std::string_view, str::CaseInsensitiveCompare<std::string_view>>>(disabledString, std::regex(","));

    static std::once_flag logDisabledOnceFlag;

    static void logDisabled() {
        std::set<std::string_view, str::CaseInsensitiveCompare<std::string_view>> known;
        std::vector<std::string_view> recognized, unrecognized;

        std::transform(
            std::begin(nvapi_interface_table),
            std::end(nvapi_interface_table),
            std::inserter(known, known.begin()),
            [](const auto& iface) { return std::string_view(iface.func); });

        for (const auto& name : disabled) {
            if (name.empty())
                continue;

            if (known.find(name) != known.end())
                recognized.push_back(name);
            else
                unrecognized.push_back(name);
        }

        if (!recognized.empty())
            log::info(str::format("NvAPI_QueryInterface: Disabling entrypoints from ", disabledEnvName, ": ", str::implode(", ", recognized)));

        if (!unrecognized.empty())
            log::info(str::format("NvAPI_QueryInterface: Ignoring unrecognized entrypoints from ", disabledEnvName, ": ", str::implode(", ", unrecognized)));
    }

    void* __cdecl nvapi_QueryInterface(NvU32 id) {
        constexpr auto n = __func__;

        static std::unordered_map<NvU32, void*> registry;
        static std::mutex registryMutex;
        std::scoped_lock lock(registryMutex);

        if (log::tracing())
            log::trace(n, log::fmt::hex(id));

        auto entry = registry.find(id);
        if (entry != registry.end())
            return entry->second;

        std::call_once(logDisabledOnceFlag, logDisabled);

        auto it = std::find_if(
            std::begin(nvapi_interface_table),
            std::end(nvapi_interface_table),
            [id](const auto& item) { return item.id == id; });

        if (it == std::end(nvapi_interface_table)) {
            log::info(str::format(n, " (0x", std::hex, id, "): Unknown function ID"));
            return registry.insert({id, nullptr}).first->second;
        }

        auto name = std::string_view(it->func);

        if (disabled.find(name) != disabled.end()) {
            log::info(str::format(n, " (", name, "): Disabled"));
            return registry.insert({id, nullptr}).first->second;
        }

#define INSERT_AND_RETURN_WHEN_EQUALS(method) \
    if (name == #method)                      \
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
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D11_CreateDevice)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D11_CreateDeviceAndSwapChain)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D1x_GetGraphicsCapabilities)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D11_MultiGPU_Init)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D11_MultiGPU_GetCaps)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D12_IsNvShaderExtnOpCodeSupported)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D12_EnumerateMetaCommands)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D12_CreateGraphicsPipelineState)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D12_SetDepthBoundsTestValues)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D12_CreateCubinComputeShaderWithName)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D12_CreateCubinComputeShaderEx)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D12_CreateCubinComputeShaderExV2)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D12_CreateCubinComputeShader)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D12_DestroyCubinComputeShader)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D12_GetCudaTextureObject)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D12_GetCudaSurfaceObject)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D12_GetCudaMergedTextureSamplerObject)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D12_GetCudaIndependentDescriptorObject)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D12_LaunchCubinShader)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D12_CaptureUAVInfo)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D12_GetGraphicsCapabilities)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D12_IsFatbinPTXSupported)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D12_SetNvShaderExtnSlotSpace)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D12_SetNvShaderExtnSlotSpaceLocalThread)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D12_GetRaytracingCaps)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D12_GetRaytracingAccelerationStructurePrebuildInfoEx)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D12_BuildRaytracingAccelerationStructureEx)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D12_NotifyOutOfBandCommandQueue)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D12_SetAsyncFrameMarker)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D_RegisterDevice)
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
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_Vulkan_InitLowLatencyDevice)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_Vulkan_DestroyLowLatencyDevice)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_Vulkan_GetSleepStatus)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_Vulkan_SetSleepMode)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_Vulkan_Sleep)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_Vulkan_GetLatency)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_Vulkan_SetLatencyMarker)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_Vulkan_NotifyOutOfBandVkQueue)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetConnectedDisplayIds)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetCurrentPCIEDownstreamWidth)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetIRQ)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetGpuCoreCount)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetGPUType)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetSystemType)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetPCIIdentifiers)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetFullName)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetBusId)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetBusSlotId)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetBusType)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetPhysicalFrameBufferSize)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetVirtualFrameBufferSize)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetMemoryInfo)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetMemoryInfoEx)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetAdapterIdFromPhysicalGpu)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetLogicalGpuInfo)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetArchInfo)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_CudaEnumComputeCapableGpus)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetGPUInfo)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetDynamicPstatesInfoEx)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetTachReading)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetThermalSettings)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetVbiosVersionString)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetCurrentPstate)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetPstates20)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetAllClockFrequencies)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_DRS_FindApplicationByName)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_DRS_FindProfileByName)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_DRS_GetSetting)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_DRS_GetBaseProfile)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_DRS_GetCurrentGlobalProfile)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_DRS_CreateProfile)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_DRS_LoadSettings)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_DRS_DestroySession)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_DRS_CreateSession)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_Disp_GetHdrCapabilities)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_Disp_HdrColorControl)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_DISP_GetDisplayIdByDisplayName)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_DISP_GetGDIPrimaryDisplayId)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_Mosaic_GetDisplayViewportsByResolution)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_NGX_GetNGXOverrideState)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_NGX_SetNGXOverrideState)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_SYS_GetPhysicalGpuFromDisplayId)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_SYS_GetDriverAndBranchVersion)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_SYS_GetDisplayDriverInfo)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_SYS_GetPhysicalGPUs)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_SYS_GetLogicalGPUs)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_EnumLogicalGPUs)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_EnumPhysicalGPUs)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_EnumTCCPhysicalGPUs)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GetPhysicalGPUFromGPUID)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GetGPUIDfromPhysicalGPU)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GetDisplayDriverVersion)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GetPhysicalGPUsFromDisplay)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GetLogicalGPUFromPhysicalGPU)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GetLogicalGPUFromDisplay)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GetPhysicalGPUsFromLogicalGPU)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_EnumNvidiaDisplayHandle)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_EnumNvidiaUnAttachedDisplayHandle)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GetAssociatedNvidiaDisplayName)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GetAssociatedNvidiaDisplayHandle)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GetInterfaceVersionString)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GetErrorMessage)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_Unload)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_Initialize)
        /* End */

#undef INSERT_AND_RETURN_WHEN_EQUALS

        log::info(str::format(n, " (", name, "): Not implemented method"));
        return registry.insert({id, nullptr}).first->second;
    }
}
