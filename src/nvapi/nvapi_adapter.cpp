#include "nvapi_adapter.h"
#include "../interfaces/dxvk_interfaces.h"
#include "../util/util_string.h"
#include "../util/util_env.h"
#include "../util/util_log.h"
#include "../util/util_version.h"

namespace dxvk {
    NvapiAdapter::NvapiAdapter(Vk& vk, Nvml& nvml, Com<IDXGIAdapter3> dxgiAdapter)
        : m_vk(vk), m_nvml(nvml), m_dxgiAdapter(std::move(dxgiAdapter)) {}

    NvapiAdapter::~NvapiAdapter() = default;

    bool NvapiAdapter::Initialize(uint32_t index, std::vector<NvapiOutput*>& outputs) {
        DXGI_ADAPTER_DESC1 dxgiDesc{};
        if (FAILED(m_dxgiAdapter->GetDesc1(&dxgiDesc)))
            return false; // Should never happen since we already know that we are dealing with a recent DXVK version

        // Report vendor / device IDs from DXVK to honor ID overrides
        m_dxgiVendorId = dxgiDesc.VendorId;
        m_dxgiDeviceId = dxgiDesc.DeviceId;

        // Report VRAM size from DXVK to honor memory overrides
        m_memoryInfo.DedicatedSystemMemory = dxgiDesc.DedicatedSystemMemory;
        m_memoryInfo.DedicatedVideoMemory = dxgiDesc.DedicatedVideoMemory;
        m_memoryInfo.SharedSystemMemory = dxgiDesc.SharedSystemMemory;

        // Get the Vulkan handle from the DXGI adapter to get access to Vulkan device properties which has some information we want.
        Com<IDXGIVkInteropAdapter> dxgiVkInteropAdapter;
        if (FAILED(m_dxgiAdapter->QueryInterface(IID_PPV_ARGS(&dxgiVkInteropAdapter)))) {
            log::info("Querying Vulkan handle from DXGI adapter failed, please ensure that DXVK's dxgi.dll is present");
            return false;
        }

        VkInstance vkInstance = VK_NULL_HANDLE;
        VkPhysicalDevice vkDevice = VK_NULL_HANDLE;
        dxgiVkInteropAdapter->GetVulkanHandles(&vkInstance, &vkDevice);

        m_vkExtensions = m_vk.GetDeviceExtensions(vkInstance, vkDevice);
        if (m_vkExtensions.empty())
            return false;

        // Query Properties for this device. Per section 4.1.2. Extending Physical Device From Device Extensions of the Vulkan
        // 1.2.177 Specification, we must first query that a device extension is
        // supported before requesting information on its physical-device-level
        // functionality (ie: Properties).
        VkPhysicalDeviceProperties2 deviceProperties2{};
        deviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        deviceProperties2.pNext = nullptr;

        if (IsVkDeviceExtensionSupported(VK_EXT_PCI_BUS_INFO_EXTENSION_NAME)) {
            m_vkPciBusProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PCI_BUS_INFO_PROPERTIES_EXT;
            m_vkPciBusProperties.pNext = deviceProperties2.pNext;
            deviceProperties2.pNext = &m_vkPciBusProperties;
        }

        if (IsVkDeviceExtensionSupported(VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME)) {
            m_vkDriverProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES_KHR;
            m_vkDriverProperties.pNext = deviceProperties2.pNext;
            deviceProperties2.pNext = &m_vkDriverProperties;
        }

        if (IsVkDeviceExtensionSupported(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME)) {
            m_vkFragmentShadingRateProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_PROPERTIES_KHR;
            m_vkFragmentShadingRateProperties.pNext = deviceProperties2.pNext;
            deviceProperties2.pNext = &m_vkFragmentShadingRateProperties;
        }

        if (IsVkDeviceExtensionSupported(VK_KHR_COMPUTE_SHADER_DERIVATIVES_EXTENSION_NAME)) {
            m_vkComputeShaderDerivativesProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COMPUTE_SHADER_DERIVATIVES_PROPERTIES_KHR;
            m_vkComputeShaderDerivativesProperties.pNext = deviceProperties2.pNext;
            deviceProperties2.pNext = &m_vkComputeShaderDerivativesProperties;
        }

        m_vkIdProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES;
        m_vkIdProperties.pNext = deviceProperties2.pNext;
        deviceProperties2.pNext = &m_vkIdProperties;

        m_vk.GetPhysicalDeviceProperties2(vkInstance, vkDevice, &deviceProperties2);
        m_vkProperties = deviceProperties2.properties;

        auto allowOtherDrivers = env::getEnvVariable(allowOtherDriversEnvName);
        if (allowOtherDrivers == "1")
            log::info(str::format(allowOtherDriversEnvName, " is set, reporting also GPUs with non-NVIDIA proprietary driver"));

        if (!HasNvProprietaryDriver() && !HasNvkDriver() && allowOtherDrivers != "1")
            return false;

        if ((HasNvProprietaryDriver() || HasNvkDriver())
            && dxgiDesc.VendorId != NvidiaPciVendorId
            && env::getEnvVariable("DXVK_ENABLE_NVAPI") != "1")
            return false; // DXVK NVAPI-hack is enabled, skip this adapter

        if (HasNvProprietaryDriver())
            m_vkDriverVersion = m_vkProperties.driverVersion;
        else
            // Reporting e.g. Mesa driver versions turned out to be not very useful
            // since those will usually always fail driver version checks,
            // so just report a number that should be "useful" until the end of time
            m_vkDriverVersion = nvMakeVersion(999, 99, 0);

        log::info(str::format("NvAPI Device: ", m_vkProperties.deviceName, " (",
            nvVersionMajor(m_vkDriverVersion), ".",
            nvVersionMinor(m_vkDriverVersion), ".",
            nvVersionPatch(m_vkDriverVersion), ")"));

        // Query all outputs from DXVK
        // Mosaic setup is not supported, thus one display output refers to one GPU
        Com<IDXGIOutput> dxgiOutput;
        for (auto i = 0U; m_dxgiAdapter->EnumOutputs(i, &dxgiOutput) != DXGI_ERROR_NOT_FOUND; i++) {
            auto nvapiOutput = new NvapiOutput(this, index, i);
            nvapiOutput->Initialize(dxgiOutput);
            outputs.push_back(nvapiOutput);
        }

        if (m_nvml.IsAvailable()) {
            char pciId[NVML_DEVICE_PCI_BUS_ID_BUFFER_SIZE];

            snprintf(pciId, NVML_DEVICE_PCI_BUS_ID_BUFFER_SIZE, NVML_DEVICE_PCI_BUS_ID_FMT,
                m_vkPciBusProperties.pciDomain,
                m_vkPciBusProperties.pciBus,
                m_vkPciBusProperties.pciDevice);

            nvmlDevice_t nvmlDevice{};
            auto result = m_nvml.DeviceGetHandleByPciBusId_v2(pciId, &nvmlDevice);
            if (result == NVML_SUCCESS) {
                m_nvmlDevice = nvmlDevice;

                nvmlMemory_v2_t memory{};
                memory.version = nvmlMemory_v2;
                if (m_nvml.DeviceGetMemoryInfo_v2(m_nvmlDevice, &memory) == NVML_SUCCESS) {
                    m_memoryInfo.ReservedVideoMemory = memory.reserved;
                }
            } else
                log::info(str::format("NVML failed to find device with PCI BusId [", pciId, "]: ", m_nvml.ErrorString(result)));
        }

        auto driverVersion = env::getEnvVariable(driverVersionEnvName);
        if (!driverVersion.empty()) {
            char* end{};
            auto driverVersionOverride = std::strtol(driverVersion.c_str(), &end, 10);
            if (std::string(end).empty() && driverVersionOverride >= 100 && driverVersionOverride <= 99999) {
                std::stringstream stream;
                stream << (driverVersionOverride / 100) << "." << std::setfill('0') << std::setw(2) << (driverVersionOverride % 100);
                log::info(str::format(driverVersionEnvName, " is set to '", driverVersion, "', reporting driver version ", stream.str()));
                m_driverVersionOverride = driverVersionOverride;
            } else
                log::info(str::format(driverVersionEnvName, " is set to '", driverVersion, "', but this value is invalid, please set a number between 100 and 99999"));
        }

        return true;
    }

    std::string NvapiAdapter::GetDeviceName() const {
        return {m_vkProperties.deviceName};
    }

    uint32_t NvapiAdapter::GetDriverVersion() const {
        // Windows releases can only ever have a two digit minor version
        // and does not have a patch number
        return m_driverVersionOverride > 0
            ? m_driverVersionOverride
            : nvVersionMajor(m_vkDriverVersion) * 100 + std::min((nvVersionMinor(m_vkDriverVersion)), 99U);
    }

    bool NvapiAdapter::HasNvProprietaryDriver() const {
        return m_vkDriverProperties.driverID == VK_DRIVER_ID_NVIDIA_PROPRIETARY;
    }

    bool NvapiAdapter::HasNvkDriver() const {
        return m_vkDriverProperties.driverID == VK_DRIVER_ID_MESA_NVK;
    }

    uint32_t NvapiAdapter::GetDeviceId() const {
        return (m_dxgiDeviceId << 16) | m_dxgiVendorId;
    }

    uint32_t NvapiAdapter::GetExternalDeviceId() const {
        return m_dxgiDeviceId;
    }

    uint32_t NvapiAdapter::GetSubSystemId() const {
        if (!this->GetNvml())
            return 0;

        nvmlPciInfo_t pciInfo{};
        auto result = this->m_nvml.DeviceGetPciInfo_v3(this->m_nvmlDevice, &pciInfo);
        return result == NVML_SUCCESS ? pciInfo.pciSubSystemId : 0;
    }

    NV_GPU_TYPE NvapiAdapter::GetGpuType() const {
        return Vk::ToNvGpuType(m_vkProperties.deviceType);
    }

    uint32_t NvapiAdapter::GetPciBusId() const {
        return m_vkPciBusProperties.pciBus;
    }

    uint32_t NvapiAdapter::GetPciDeviceId() const {
        return m_vkPciBusProperties.pciDevice;
    }

    uint32_t NvapiAdapter::GetBoardId() const {
        // There is also https://docs.nvidia.com/deploy/nvml-api/group__nvmlDeviceQueries.html#group__nvmlDeviceQueries_1gbf4a80f14b6093ce98e4c2dd511275c5
        // But since we don't have NVML everywhere, we will create a board ID derived from PCI Domain/BUS/Device ID
        return m_vkPciBusProperties.pciDomain << 16
            | m_vkPciBusProperties.pciBus << 8
            | m_vkPciBusProperties.pciDevice;
    }

    const NvapiAdapter::MemoryInfo& NvapiAdapter::GetMemoryInfo() const {
        return m_memoryInfo;
    }

    NvapiAdapter::MemoryBudgetInfo NvapiAdapter::GetCurrentMemoryBudgetInfo() const {
        DXGI_QUERY_VIDEO_MEMORY_INFO dxgiMemInfo{};
        if (FAILED(m_dxgiAdapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &dxgiMemInfo)))
            return MemoryBudgetInfo{};

        return MemoryBudgetInfo{
            .Budget = dxgiMemInfo.Budget,
            .CurrentUsage = dxgiMemInfo.CurrentUsage};
    }

    std::optional<LUID> NvapiAdapter::GetLuid() const {
        if (!m_vkIdProperties.deviceLUIDValid)
            return {};

        LUID luid{};
        memcpy(&luid, &m_vkIdProperties.deviceLUID, sizeof(luid));
        return std::make_optional(luid);
    }

    NV_GPU_ARCHITECTURE_ID NvapiAdapter::GetArchitectureId() const {
        if (!this->HasNvProprietaryDriver() && !this->HasNvkDriver()) {
            // DXVK_NVAPI_ALLOW_OTHER_DRIVERS must be set, otherwise this would be unreachable
            log::info(str::format(allowOtherDriversEnvName, " is set, using Pascal architecture for non-NVIDIA GPUs by default"));
            return NV_GPU_ARCHITECTURE_GP100;
        }

        // GB20x supports mesh+task derivatives
        if (IsVkDeviceExtensionSupported(VK_KHR_COMPUTE_SHADER_DERIVATIVES_EXTENSION_NAME)
            && m_vkComputeShaderDerivativesProperties.meshAndTaskShaderDerivatives)
            return NV_GPU_ARCHITECTURE_GB200;

        // In lieu of a more idiomatic Vulkan-based solution, check the PCI
        // DeviceID to determine if an Ada card is present
        if (m_vkProperties.deviceID >= 0x2600)
            return NV_GPU_ARCHITECTURE_AD100;

        // See https://gitlab.freedesktop.org/mesa/mesa/-/blob/main/src/nouveau/vulkan/nvk_physical_device.c
        // for NVK properties and features

        // KHR_fragment_shading_rate's
        // primitiveFragmentShadingRateWithMultipleViewports is supported on Ampere and newer
        if (IsVkDeviceExtensionSupported(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME)
            && m_vkFragmentShadingRateProperties.primitiveFragmentShadingRateWithMultipleViewports)
            return NV_GPU_ARCHITECTURE_GA100;

        // VK_KHR_fragment_shader_barycentric is supported on Turing and newer
        if (IsVkDeviceExtensionSupported(VK_KHR_FRAGMENT_SHADER_BARYCENTRIC_EXTENSION_NAME))
            return NV_GPU_ARCHITECTURE_TU100;

        // VK_NVX_image_view_handle is supported on Volta and newer on the NVIDIA proprietary driver
        // VK_EXT_depth_range_unrestricted is supported on Volta and newer on NVK
        if ((HasNvProprietaryDriver() && IsVkDeviceExtensionSupported(VK_NVX_IMAGE_VIEW_HANDLE_EXTENSION_NAME))
            || (HasNvkDriver() && IsVkDeviceExtensionSupported(VK_EXT_DEPTH_RANGE_UNRESTRICTED_EXTENSION_NAME)))
            return NV_GPU_ARCHITECTURE_GV100;

        // VK_NV_clip_space_w_scaling is supported on Pascal and newer on the NVIDIA proprietary driver
        // Use device limits to identify Pascal on NVK
        if ((HasNvProprietaryDriver() && IsVkDeviceExtensionSupported(VK_NV_CLIP_SPACE_W_SCALING_EXTENSION_NAME))
            || (HasNvkDriver() && m_vkProperties.limits.maxFramebufferHeight >= 0x8000))
            return NV_GPU_ARCHITECTURE_GP100;

        // VK_NV_viewport_array2 is supported on Maxwell and newer on the NVIDIA proprietary driver
        // VK_EXT_shader_viewport_index_layer is supported on Maxwell and newer on NVK
        if ((HasNvProprietaryDriver() && IsVkDeviceExtensionSupported(VK_NV_VIEWPORT_ARRAY2_EXTENSION_NAME))
            || (HasNvkDriver() && IsVkDeviceExtensionSupported(VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME)))
            return NV_GPU_ARCHITECTURE_GM200;

        // VK_EXT_shader_image_atomic_int64 is supported on Maxwell 1 (GM10x) and newer
        if (IsVkDeviceExtensionSupported(VK_EXT_SHADER_IMAGE_ATOMIC_INT64_EXTENSION_NAME))
            return NV_GPU_ARCHITECTURE_GM000;

        // Fall back to Kepler
        return NV_GPU_ARCHITECTURE_GK100;
    }

    bool NvapiAdapter::IsVkDeviceExtensionSupported(const std::string& name) const {
        return m_vkExtensions.find(name) != m_vkExtensions.end();
    }

    Nvml* NvapiAdapter::GetNvml() const {
        if (!this->m_nvml.IsAvailable())
            return nullptr;

        return &m_nvml;
    }

    nvmlDevice_t NvapiAdapter::GetNvmlDevice() const {
        if (!this->m_nvml.IsAvailable())
            return nullptr;

        return m_nvmlDevice;
    }
}
