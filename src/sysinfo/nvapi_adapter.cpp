#include "nvapi_adapter.h"
#include "../dxvk/dxvk_interfaces.h"
#include "../util/util_string.h"
#include "../util/util_env.h"
#include "../util/util_log.h"
#include "../util/util_version.h"

namespace dxvk {
    NvapiAdapter::NvapiAdapter(Vulkan& vulkan, Nvml& nvml)
        : m_vulkan(vulkan), m_nvml(nvml) {}

    NvapiAdapter::~NvapiAdapter() = default;

    bool NvapiAdapter::Initialize(Com<IDXGIAdapter1>& dxgiAdapter, uint32_t index, std::vector<NvapiOutput*>& outputs) {
        if (FAILED(dxgiAdapter->GetDesc1(&m_dxgiDesc)))
            return false; // Should never happen since we already know that we are dealing with a recent DXVK version

        // Get the Vulkan handle from the DXGI adapter to get access to Vulkan device properties which has some information we want.
        Com<IDXGIVkInteropAdapter> dxgiVkInteropAdapter;
        if (FAILED(dxgiAdapter->QueryInterface(IID_PPV_ARGS(&dxgiVkInteropAdapter)))) {
            log::write("Querying Vulkan handle from DXGI adapter failed, please ensure that DXVK's dxgi.dll is present");
            return false;
        }

        VkInstance vkInstance = VK_NULL_HANDLE;
        VkPhysicalDevice vkDevice = VK_NULL_HANDLE;
        dxgiVkInteropAdapter->GetVulkanHandles(&vkInstance, &vkDevice);

        m_vkExtensions = m_vulkan.GetDeviceExtensions(vkInstance, vkDevice);
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

        m_vkIdProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES;
        m_vkIdProperties.pNext = deviceProperties2.pNext;
        deviceProperties2.pNext = &m_vkIdProperties;

        m_vulkan.GetPhysicalDeviceProperties2(vkInstance, vkDevice, &deviceProperties2);
        m_vkProperties = deviceProperties2.properties;

        VkPhysicalDeviceFeatures2 deviceFeatures2{};
        deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        deviceFeatures2.pNext = nullptr;

        if (IsVkDeviceExtensionSupported(VK_EXT_DEPTH_CLIP_CONTROL_EXTENSION_NAME)) {
            m_vkDepthClipControlFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLIP_CONTROL_FEATURES_EXT;
            m_vkDepthClipControlFeatures.pNext = deviceFeatures2.pNext;
            deviceFeatures2.pNext = &m_vkDepthClipControlFeatures;
        }

        m_vulkan.GetPhysicalDeviceFeatures2(vkInstance, vkDevice, &deviceFeatures2);
        m_vkFeatures = deviceFeatures2.features;

        auto allowOtherDrivers = env::getEnvVariable(allowOtherDriversEnvName);
        if (allowOtherDrivers == "1")
            log::write(str::format(allowOtherDriversEnvName, " is set, reporting also GPUs with non-NVIDIA proprietary driver"));

        if (!HasNvProprietaryDriver() && !HasNvkDriver() && allowOtherDrivers != "1")
            return false;

        if ((HasNvProprietaryDriver() || HasNvkDriver())
            && m_dxgiDesc.VendorId != NvidiaPciVendorId
            && env::getEnvVariable("DXVK_ENABLE_NVAPI") != "1")
            return false; // DXVK NVAPI-hack is enabled, skip this adapter

        if (HasNvProprietaryDriver())
            m_vkDriverVersion = m_vkProperties.driverVersion;
        else
            // Reporting e.g. Mesa driver versions turned out to be not very useful
            // since those will usually always fail driver version checks,
            // so just report a number that should be "useful" until the end of time
            m_vkDriverVersion = nvMakeVersion(999, 99, 0);

        log::write(str::format("NvAPI Device: ", m_vkProperties.deviceName, " (",
            nvVersionMajor(m_vkDriverVersion), ".",
            nvVersionMinor(m_vkDriverVersion), ".",
            nvVersionPatch(m_vkDriverVersion), ")"));

        // Query all outputs from DXVK
        // Mosaic setup is not supported, thus one display output refers to one GPU
        Com<IDXGIOutput> dxgiOutput;
        for (auto i = 0U; dxgiAdapter->EnumOutputs(i, &dxgiOutput) != DXGI_ERROR_NOT_FOUND; i++) {
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
            if (result == NVML_SUCCESS)
                m_nvmlDevice = nvmlDevice;
            else
                log::write(str::format("NVML failed to find device with PCI BusId [", pciId, "]: ", m_nvml.ErrorString(result)));
        }

        auto driverVersion = env::getEnvVariable(driverVersionEnvName);
        if (!driverVersion.empty()) {
            char* end{};
            auto driverVersionOverride = std::strtol(driverVersion.c_str(), &end, 10);
            if (std::string(end).empty() && driverVersionOverride >= 100 && driverVersionOverride <= 99999) {
                std::stringstream stream;
                stream << (driverVersionOverride / 100) << "." << std::setfill('0') << std::setw(2) << (driverVersionOverride % 100);
                log::write(str::format(driverVersionEnvName, " is set to '", driverVersion, "', reporting driver version ", stream.str()));
                m_driverVersionOverride = driverVersionOverride;
            } else
                log::write(str::format(driverVersionEnvName, " is set to '", driverVersion, "', but this value is invalid, please set a number between 100 and 99999"));
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
        // Report vendor / device IDs from DXVK to honor ID overrides
        return (m_dxgiDesc.DeviceId << 16) | m_dxgiDesc.VendorId;
    }

    uint32_t NvapiAdapter::GetExternalDeviceId() const {
        // Report device ID from DXVK to honor ID overrides
        return m_dxgiDesc.DeviceId;
    }

    uint32_t NvapiAdapter::GetSubSystemId() const {
        if (!this->HasNvmlDevice())
            return 0;

        nvmlPciInfo_t pciInfo{};
        auto result = this->m_nvml.DeviceGetPciInfo_v3(this->m_nvmlDevice, &pciInfo);
        return result == NVML_SUCCESS ? pciInfo.pciSubSystemId : 0;
    }

    NV_GPU_TYPE NvapiAdapter::GetGpuType() const {
        return Vulkan::ToNvGpuType(m_vkProperties.deviceType);
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

    uint32_t NvapiAdapter::GetVRamSize() const {
        // Report VRAM size from DXVK to honor memory overrides
        return m_dxgiDesc.DedicatedVideoMemory / 1024;
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
            log::write(str::format(allowOtherDriversEnvName, " is set, spoofing Pascal for GPU with non-NVIDIA proprietary driver"));
            return NV_GPU_ARCHITECTURE_GP100;
        }

        // In lieu of a more idiomatic Vulkan-based solution, check the PCI
        // DeviceID to determine if an Ada card is present
        if (m_vkProperties.deviceID >= 0x2600)
            return NV_GPU_ARCHITECTURE_AD100;

        // See https://gitlab.freedesktop.org/mesa/mesa/-/blob/main/src/nouveau/vulkan/nvk_physical_device.c
        // for NVK properties and features

        // KHR_fragment_shading_rate's
        // primitiveFragmentShadingRateWithMultipleViewports is supported on Ampere and newer
        // TODO: We haven't found a way yet to identify Ampere on NVK
        if (HasNvProprietaryDriver()
            && IsVkDeviceExtensionSupported(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME)
            && m_vkFragmentShadingRateProperties.primitiveFragmentShadingRateWithMultipleViewports)
            return NV_GPU_ARCHITECTURE_GA100;

        // VK_KHR_fragment_shader_barycentric is supported on Turing and newer
        if (IsVkDeviceExtensionSupported(VK_KHR_FRAGMENT_SHADER_BARYCENTRIC_EXTENSION_NAME))
            return NV_GPU_ARCHITECTURE_TU100;

        // VK_NVX_image_view_handle is supported on Volta and newer on the NVIDIA proprietary driver
        // VK_EXT_depth_clip_control's depthClipControl is supported on Volta and newer on NVK
        if ((HasNvProprietaryDriver() && IsVkDeviceExtensionSupported(VK_NVX_IMAGE_VIEW_HANDLE_EXTENSION_NAME))
            || (HasNvkDriver()
                && IsVkDeviceExtensionSupported(VK_EXT_DEPTH_CLIP_CONTROL_EXTENSION_NAME)
                && m_vkDepthClipControlFeatures.depthClipControl))
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

        // Fall back to Kepler
        return NV_GPU_ARCHITECTURE_GK100;
    }

    bool NvapiAdapter::IsVkDeviceExtensionSupported(std::string name) const { // NOLINT(performance-unnecessary-value-param)
        return m_vkExtensions.find(name) != m_vkExtensions.end();
    }

    bool NvapiAdapter::HasNvml() const {
        return m_nvml.IsAvailable();
    }

    bool NvapiAdapter::HasNvmlDevice() const {
        return m_nvml.IsAvailable() && m_nvmlDevice != nullptr;
    }

    std::string NvapiAdapter::GetNvmlErrorString(nvmlReturn_t result) const {
        return {m_nvml.ErrorString(result)};
    }

    nvmlReturn_t NvapiAdapter::GetNvmlDeviceClockInfo(nvmlClockType_t type, unsigned int* clock) const {
        return m_nvml.DeviceGetClockInfo(m_nvmlDevice, type, clock);
    }

    nvmlReturn_t NvapiAdapter::GetNvmlDeviceTemperature(nvmlTemperatureSensors_t sensorType, unsigned int* temp) const {
        return m_nvml.DeviceGetTemperature(m_nvmlDevice, sensorType, temp);
    }

    nvmlReturn_t NvapiAdapter::GetNvmlDeviceThermalSettings(unsigned int sensorIndex, nvmlGpuThermalSettings_t* pThermalSettings) const {
        return m_nvml.DeviceGetThermalSettings(m_nvmlDevice, sensorIndex, pThermalSettings);
    }

    nvmlReturn_t NvapiAdapter::GetNvmlDevicePerformanceState(nvmlPstates_t* pState) const {
        return m_nvml.DeviceGetPerformanceState(m_nvmlDevice, pState);
    }

    nvmlReturn_t NvapiAdapter::GetNvmlDeviceUtilizationRates(nvmlUtilization_t* utilization) const {
        return m_nvml.DeviceGetUtilizationRates(m_nvmlDevice, utilization);
    }

    nvmlReturn_t NvapiAdapter::GetNvmlDeviceVbiosVersion(char* version, unsigned int length) const {
        return m_nvml.DeviceGetVbiosVersion(m_nvmlDevice, version, length);
    }

    nvmlReturn_t NvapiAdapter::GetNvmlDeviceGetCurrPcieLinkWidth(unsigned int* width) const {
        return m_nvml.DeviceGetCurrPcieLinkWidth(m_nvmlDevice, width);
    }

    nvmlReturn_t NvapiAdapter::GetNvmlDeviceGetIrqNum(unsigned int* irq) const {
        return m_nvml.DeviceGetIrqNum(m_nvmlDevice, irq);
    }

    nvmlReturn_t NvapiAdapter::GetNvmlDeviceNumGpuCores(unsigned int* numCores) const {
        return m_nvml.DeviceGetNumGpuCores(m_nvmlDevice, numCores);
    }

    nvmlReturn_t NvapiAdapter::GetNvmlDeviceBusType(nvmlBusType_t* type) const {
        return m_nvml.DeviceGetBusType(m_nvmlDevice, type);
    }

    nvmlReturn_t NvapiAdapter::GetNvmlDeviceDynamicPstatesInfo(nvmlGpuDynamicPstatesInfo_t* pDynamicPstatesInfo) const {
        return m_nvml.DeviceGetDynamicPstatesInfo(m_nvmlDevice, pDynamicPstatesInfo);
    }
}
