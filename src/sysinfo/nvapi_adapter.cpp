#include "nvapi_adapter.h"
#include "../dxvk/dxvk_interfaces.h"
#include "../util/util_string.h"
#include "../util/util_env.h"
#include "../util/util_log.h"

namespace dxvk {
    NvapiAdapter::NvapiAdapter(Nvml& nvml)
        : m_nvml(nvml) {}

    NvapiAdapter::~NvapiAdapter() = default;

    bool NvapiAdapter::Initialize(Com<IDXGIAdapter1>& dxgiAdapter, uint32_t index, Vulkan& vulkan, std::vector<NvapiOutput*>& outputs) {
        constexpr auto driverVersionEnvName = "DXVK_NVAPI_DRIVER_VERSION";
        constexpr auto allowOtherDriversEnvName = "DXVK_NVAPI_ALLOW_OTHER_DRIVERS";

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

        m_deviceExtensions = vulkan.GetDeviceExtensions(vkInstance, vkDevice);
        if (m_deviceExtensions.empty())
            return false;

        // Query Properties for this device. Per section 4.1.2. Extending Physical Device From Device Extensions of the Vulkan
        // 1.2.177 Specification, we must first query that a device extension is
        // supported before requesting information on its physical-device-level
        // functionality (ie: Properties).
        VkPhysicalDeviceProperties2 deviceProperties2{};
        deviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        deviceProperties2.pNext = nullptr;

        if (IsVkDeviceExtensionSupported(VK_EXT_PCI_BUS_INFO_EXTENSION_NAME)) {
            m_devicePciBusProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PCI_BUS_INFO_PROPERTIES_EXT;
            m_devicePciBusProperties.pNext = deviceProperties2.pNext;
            deviceProperties2.pNext = &m_devicePciBusProperties;
        }

        if (IsVkDeviceExtensionSupported(VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME)) {
            m_deviceDriverProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES_KHR;
            m_deviceDriverProperties.pNext = deviceProperties2.pNext;
            deviceProperties2.pNext = &m_deviceDriverProperties;
        }

        if (IsVkDeviceExtensionSupported(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME)) {
            m_deviceFragmentShadingRateProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_PROPERTIES_KHR;
            m_deviceFragmentShadingRateProperties.pNext = deviceProperties2.pNext;
            deviceProperties2.pNext = &m_deviceFragmentShadingRateProperties;
        }

        m_deviceIdProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES;
        m_deviceIdProperties.pNext = deviceProperties2.pNext;
        deviceProperties2.pNext = &m_deviceIdProperties;

        vulkan.GetPhysicalDeviceProperties2(vkInstance, vkDevice, &deviceProperties2);
        m_deviceProperties = deviceProperties2.properties;

        auto allowOtherDrivers = env::getEnvVariable(allowOtherDriversEnvName);
        if (!allowOtherDrivers.empty())
            log::write(str::format(allowOtherDriversEnvName, " is set, reporting also GPUs with non-NVIDIA proprietary driver"));

        if (!HasNvProprietaryDriver() && allowOtherDrivers.empty())
            return false;

        if (HasNvProprietaryDriver())
            // Handle NVIDIA version notation
            m_vkDriverVersion = VK_MAKE_VERSION(
                VK_VERSION_MAJOR(m_deviceProperties.driverVersion),
                VK_VERSION_MINOR(m_deviceProperties.driverVersion >> 0) >> 2,
                VK_VERSION_PATCH(m_deviceProperties.driverVersion >> 2) >> 4);
        else
            m_vkDriverVersion = m_deviceProperties.driverVersion;

        log::write(str::format("NvAPI Device: ", m_deviceProperties.deviceName, " (",
            VK_VERSION_MAJOR(m_vkDriverVersion), ".",
            VK_VERSION_MINOR(m_vkDriverVersion), ".",
            VK_VERSION_PATCH(m_vkDriverVersion), ")"));

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
                m_devicePciBusProperties.pciDomain,
                m_devicePciBusProperties.pciBus,
                m_devicePciBusProperties.pciDevice);

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
        return {m_deviceProperties.deviceName};
    }

    uint32_t NvapiAdapter::GetDriverVersion() const {
        // Windows releases can only ever have a two digit minor version
        // and does not have a patch number
        return m_driverVersionOverride > 0
            ? m_driverVersionOverride
            : VK_VERSION_MAJOR(m_vkDriverVersion) * 100 + std::min(VK_VERSION_MINOR(m_vkDriverVersion), 99U);
    }

    bool NvapiAdapter::HasNvProprietaryDriver() const {
        return m_deviceDriverProperties.driverID == VK_DRIVER_ID_NVIDIA_PROPRIETARY;
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
        return Vulkan::ToNvGpuType(m_deviceProperties.deviceType);
    }

    uint32_t NvapiAdapter::GetPciBusId() const {
        return m_devicePciBusProperties.pciBus;
    }

    uint32_t NvapiAdapter::GetPciDeviceId() const {
        return m_devicePciBusProperties.pciDevice;
    }

    uint32_t NvapiAdapter::GetBoardId() const {
        // There is also https://docs.nvidia.com/deploy/nvml-api/group__nvmlDeviceQueries.html#group__nvmlDeviceQueries_1gbf4a80f14b6093ce98e4c2dd511275c5
        // But since we don't have NVML everywhere, we will create a board ID derived from PCI Domain/BUS/Device ID
        return m_devicePciBusProperties.pciDomain << 16
            | m_devicePciBusProperties.pciBus << 8
            | m_devicePciBusProperties.pciDevice;
    }

    uint32_t NvapiAdapter::GetVRamSize() const {
        // Report VRAM size from DXVK to honor memory overrides
        return m_dxgiDesc.DedicatedVideoMemory / 1024;
    }

    std::optional<LUID> NvapiAdapter::GetLuid() const {
        if (!m_deviceIdProperties.deviceLUIDValid)
            return {};

        LUID luid{};
        memcpy(&luid, &m_deviceIdProperties.deviceLUID, sizeof(luid));
        return std::make_optional(luid);
    }

    NV_GPU_ARCHITECTURE_ID NvapiAdapter::GetArchitectureId() const {
        // In lieu of a more idiomatic Vulkan-based solution, check the PCI
        // DeviceID to determine if an Ada card is present
        if (m_deviceProperties.deviceID >= 0x2600)
            return NV_GPU_ARCHITECTURE_AD100;

        // KHR_fragment_shading_rate's
        // primitiveFragmentShadingRateWithMultipleViewports is supported on
        // Ampere and newer
        if (IsVkDeviceExtensionSupported(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME)
            && m_deviceFragmentShadingRateProperties.primitiveFragmentShadingRateWithMultipleViewports)
            return NV_GPU_ARCHITECTURE_GA100;

        // Variable rate shading is supported on Turing and newer
        if (IsVkDeviceExtensionSupported(VK_NV_SHADING_RATE_IMAGE_EXTENSION_NAME))
            return NV_GPU_ARCHITECTURE_TU100;

        // VK_NVX_image_view_handle is supported on Volta and newer
        if (IsVkDeviceExtensionSupported(VK_NVX_IMAGE_VIEW_HANDLE_EXTENSION_NAME))
            return NV_GPU_ARCHITECTURE_GV100;

        // VK_NV_clip_space_w_scaling is supported on Pascal and newer
        if (IsVkDeviceExtensionSupported(VK_NV_CLIP_SPACE_W_SCALING_EXTENSION_NAME))
            return NV_GPU_ARCHITECTURE_GP100;

        // VK_NV_viewport_array2 is supported on Maxwell and newer
        if (IsVkDeviceExtensionSupported(VK_NV_VIEWPORT_ARRAY2_EXTENSION_NAME))
            return NV_GPU_ARCHITECTURE_GM200;

        // Fall back to Kepler
        return NV_GPU_ARCHITECTURE_GK100;
    }

    bool NvapiAdapter::IsVkDeviceExtensionSupported(std::string name) const { // NOLINT(performance-unnecessary-value-param)
        return m_deviceExtensions.find(name) != m_deviceExtensions.end();
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
