#include "../inc/nvapi_interface.h"
#include "nvapi.cpp"
#include "nvapi_sys.cpp"
#include "nvapi_disp.cpp"
#include "nvapi_mosaic.cpp"
#include "nvapi_gpu.cpp"
#include "nvapi_d3d.cpp"
#include "nvapi_d3d11.cpp"
#include "nvapi_d3d12.cpp"
#include "util/util_string.h"
#include "util/util_log.h"

#define INSERT_AND_RETURN_WHEN_EQUALS(method) \
    if (std::string(it->func) == #method) \
        return registry.insert({id, (void*) method}).first->second;

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
            log::write(str::format("NvAPI_QueryInterface 0x", std::hex, id, ": Unknown function ID"));
            return registry.insert({id, nullptr}).first->second;
        }

        // This block will be validated for completeness when running package-release.sh. Do not remove the comments.
        /* Start NVAPI methods */
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D11_SetDepthBoundsTest)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D11_BeginUAVOverlap)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D11_EndUAVOverlap)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D11_MultiDrawInstancedIndirect)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D11_MultiDrawIndexedInstancedIndirect)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D11_IsNvShaderExtnOpCodeSupported)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D12_IsNvShaderExtnOpCodeSupported)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D_GetObjectHandleForResource)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D_SetResourceHint)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_D3D_GetCurrentSLIState)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetGPUType)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetPCIIdentifiers)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetFullName)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetBusId)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetPhysicalFrameBufferSize)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetAdapterIdFromPhysicalGpu)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetArchInfo)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetDynamicPstatesInfoEx)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetThermalSettings)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetVbiosVersionString)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GPU_GetAllClockFrequencies)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_Disp_GetHdrCapabilities)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_DISP_GetDisplayIdByDisplayName)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_DISP_GetGDIPrimaryDisplayId)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_Mosaic_GetDisplayViewportsByResolution)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_SYS_GetPhysicalGpuFromDisplayId)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_SYS_GetDriverAndBranchVersion)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_EnumLogicalGPUs)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_EnumPhysicalGPUs)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GetDisplayDriverVersion)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GetPhysicalGPUsFromDisplay)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_EnumNvidiaDisplayHandle)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_EnumNvidiaUnAttachedDisplayHandle)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GetInterfaceVersionString)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GetInterfaceVersionStringEx)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_GetErrorMessage)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_Unload)
        INSERT_AND_RETURN_WHEN_EQUALS(NvAPI_Initialize)
        /* End */

        log::write(str::format("NvAPI_QueryInterface ", it->func, ": Not implemented method"));
        return registry.insert({id, nullptr}).first->second;
    }
}

#undef INSERT_AND_RETURN_WHEN_EQUALS
