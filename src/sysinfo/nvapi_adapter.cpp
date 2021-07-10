#include "nvapi_adapter.h"
#include "../dxvk/dxvk_interfaces.h"
#include "../util/util_string.h"
#include "../util/util_env.h"
#include "../util/util_log.h"

namespace dxvk {
    NvapiAdapter::NvapiAdapter(Vulkan& vulkan, Nvml& nvml)
        : m_vulkan(vulkan), m_nvml(nvml) {}

    NvapiAdapter::~NvapiAdapter() = default;

    bool NvapiAdapter::Initialize(Com<IDXGIAdapter>& dxgiAdapter, std::vector<NvapiOutput*>& outputs) {
        constexpr auto driverVersionEnvName = "DXVK_NVAPI_DRIVER_VERSION";

        // Query all outputs from DXVK
        // Mosaic setup is not supported, thus one display output refers to one GPU
        Com<IDXGIOutput> dxgiOutput;
        for (auto i = 0U; dxgiAdapter->EnumOutputs(i, &dxgiOutput) != DXGI_ERROR_NOT_FOUND; i++) {
            auto nvapiOutput = new NvapiOutput((uintptr_t) this);
            nvapiOutput->Initialize(dxgiOutput);
            outputs.push_back(nvapiOutput);
        }

        // Get the Vulkan handle from the DXGI adapter to get access to Vulkan device properties which has some information we want.
        Com<IDXGIVkInteropAdapter> dxgiVkInteropAdapter;
        if (FAILED(dxgiAdapter->QueryInterface(IID_PPV_ARGS(&dxgiVkInteropAdapter)))) {
            log::write("Querying Vulkan handle from DXGI adapter failed, please ensure that DXVK's dxgi.dll is loaded");
            return false;
        }

        VkInstance vkInstance = VK_NULL_HANDLE;
        VkPhysicalDevice vkDevice = VK_NULL_HANDLE;
        dxgiVkInteropAdapter->GetVulkanHandles(&vkInstance, &vkDevice);

        m_deviceExtensions = m_vulkan.GetDeviceExtensions(vkInstance, vkDevice);
        if (m_deviceExtensions.empty())
            return false;

        // Query Properties for this device. Per section 4.1.2. Extending Physical Device From Device Extensions of the Vulkan
        // 1.2.177 Specification, we must first query that a device extension is
        // supported before requesting information on its physical-device-level
        // functionality (ie: Properties).
        VkPhysicalDeviceProperties2 deviceProperties2;
        deviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        deviceProperties2.pNext = nullptr;

        if (isVkDeviceExtensionSupported(VK_EXT_PCI_BUS_INFO_EXTENSION_NAME)) {
            m_devicePciBusProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PCI_BUS_INFO_PROPERTIES_EXT;
            m_devicePciBusProperties.pNext = deviceProperties2.pNext;
            deviceProperties2.pNext = &m_devicePciBusProperties;
        }

        if (isVkDeviceExtensionSupported(VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME)) {
            m_deviceDriverProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES_KHR;
            m_deviceDriverProperties.pNext = deviceProperties2.pNext;
            deviceProperties2.pNext = &m_deviceDriverProperties;
        }

        if (isVkDeviceExtensionSupported(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME)) {
            m_deviceFragmentShadingRateProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_PROPERTIES_KHR;
            m_deviceFragmentShadingRateProperties.pNext = deviceProperties2.pNext;
            deviceProperties2.pNext = &m_deviceFragmentShadingRateProperties;
        }

        m_deviceIdProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES;
        m_deviceIdProperties.pNext = deviceProperties2.pNext;
        deviceProperties2.pNext = &m_deviceIdProperties;

        m_vulkan.GetPhysicalDeviceProperties2(vkInstance, vkDevice, &deviceProperties2);
        m_deviceProperties = deviceProperties2.properties;

        VkPhysicalDeviceMemoryProperties2 memoryProperties2;
        memoryProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
        memoryProperties2.pNext = nullptr;

        m_vulkan.GetPhysicalDeviceMemoryProperties2(vkInstance, vkDevice, &memoryProperties2);
        m_memoryProperties = memoryProperties2.memoryProperties;

        if (GetDriverId() == VK_DRIVER_ID_NVIDIA_PROPRIETARY)
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

        if (m_nvml.IsAvailable()) {
            char pciId[NVML_DEVICE_PCI_BUS_ID_BUFFER_SIZE];

            snprintf(pciId, NVML_DEVICE_PCI_BUS_ID_BUFFER_SIZE, NVML_DEVICE_PCI_BUS_ID_FMT,
                m_devicePciBusProperties.pciDomain,
                m_devicePciBusProperties.pciBus,
                m_devicePciBusProperties.pciDevice);

            nvmlDevice_t nvmlDevice;
            auto result = m_nvml.DeviceGetHandleByPciBusId_v2(pciId, &nvmlDevice);
            if (result == NVML_SUCCESS)
                m_nvmlDevice = nvmlDevice;
            else
                log::write(str::format("NVML failed to find device with PCI BusId [", pciId, "]: ", m_nvml.ErrorString(result)));
        }

        auto driverVersion = env::getEnvVariable(driverVersionEnvName);
        if (!driverVersion.empty()) {
            char* end;
            auto driverVersionOverride = std::strtol(driverVersion.c_str(), &end, 10);
            if (std::string(end).empty() && driverVersionOverride >= 100 && driverVersionOverride <= 99999) {
                std::stringstream stream;
                stream << (driverVersionOverride / 100) << "." << std::setfill('0') << std::setw(2) << (driverVersionOverride % 100);
                log::write(str::format(driverVersionEnvName, " is set to '", driverVersion, "', reporting driver version ", stream.str()));
                m_driverVersionOverride = driverVersionOverride;
            }
            else
                log::write(str::format(driverVersionEnvName, " is set to '", driverVersion, "', but this value is invalid. Please set a number between 100 and 99999."));
        }

        return true;
    }

    std::string NvapiAdapter::GetDeviceName() const {
        return std::string(m_deviceProperties.deviceName);
    }

    uint32_t NvapiAdapter::GetDriverVersion() const {
        // Windows releases can only ever have a two digit minor version
        // and does not have a patch number
        return m_driverVersionOverride > 0
            ? m_driverVersionOverride
            : VK_VERSION_MAJOR(m_vkDriverVersion) * 100 +
                std::min(VK_VERSION_MINOR(m_vkDriverVersion), 99U);
    }

    VkDriverIdKHR NvapiAdapter::GetDriverId() const {
        return m_deviceDriverProperties.driverID;
    }

    uint32_t NvapiAdapter::GetDeviceId() const {
        return (m_deviceProperties.deviceID << 16) | m_deviceProperties.vendorID;
    }

    uint32_t NvapiAdapter::GetGpuType() const {
        // The enum values for discrete, integrated and unknown GPU are the same for Vulkan and NvAPI
        auto vkDeviceType = m_deviceProperties.deviceType;
        return vkDeviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU || vkDeviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU
            ? vkDeviceType
            : VK_PHYSICAL_DEVICE_TYPE_OTHER;
    }

    uint32_t NvapiAdapter::GetBusId() const {
        return m_devicePciBusProperties.pciBus;
    }

    uint32_t NvapiAdapter::GetVRamSize() const {
        // The total size of all device-local heaps sometimes do not match what other tools are reporting,
        // though this is best we have.
        auto size = 0U;
        for (auto i = 0U; i < m_memoryProperties.memoryHeapCount; i++) {
            auto heap = m_memoryProperties.memoryHeaps[i];
            if (heap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
                size += heap.size / 1024;
        }

        return size;
    }

    bool NvapiAdapter::GetLUID(LUID* luid) const {
        if (!m_deviceIdProperties.deviceLUIDValid)
            return false;

        memcpy(luid, &m_deviceIdProperties.deviceLUID, sizeof(*luid));
        return true;
    }

    NV_GPU_ARCHITECTURE_ID NvapiAdapter::GetArchitectureId() const {
        // KHR_fragment_shading_rate's
        // primitiveFragmentShadingRateWithMultipleViewports is supported on
        // Ampere and newer
        if (isVkDeviceExtensionSupported(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME)
            && m_deviceFragmentShadingRateProperties.primitiveFragmentShadingRateWithMultipleViewports)
            return NV_GPU_ARCHITECTURE_GA100;

        // Variable rate shading is supported on Turing and newer
        if (isVkDeviceExtensionSupported(VK_NV_SHADING_RATE_IMAGE_EXTENSION_NAME))
            return NV_GPU_ARCHITECTURE_TU100;

        // VK_NVX_image_view_handle is supported on Volta and newer
        if (isVkDeviceExtensionSupported(VK_NVX_IMAGE_VIEW_HANDLE_EXTENSION_NAME))
            return NV_GPU_ARCHITECTURE_GV100;

        // VK_NV_clip_space_w_scaling is supported on Pascal and newer
        if (isVkDeviceExtensionSupported(VK_NV_CLIP_SPACE_W_SCALING_EXTENSION_NAME))
            return NV_GPU_ARCHITECTURE_GP100;

        // VK_NV_viewport_array2 is supported on Maxwell and newer
        if (isVkDeviceExtensionSupported(VK_NV_VIEWPORT_ARRAY2_EXTENSION_NAME))
            return NV_GPU_ARCHITECTURE_GM200;

        // Fall back to Kepler
        return NV_GPU_ARCHITECTURE_GK100;
    }

    bool NvapiAdapter::isVkDeviceExtensionSupported(const std::string name) const { // NOLINT(performance-unnecessary-value-param)
        return m_deviceExtensions.find(name) != m_deviceExtensions.end();
    }

    bool NvapiAdapter::HasNvml() const {
        return m_nvml.IsAvailable();
    }

    bool NvapiAdapter::HasNvmlDevice() const {
        return m_nvml.IsAvailable() && m_nvmlDevice != nullptr;
    }

    std::string NvapiAdapter::NvmlErrorString(nvmlReturn_t result) const {
        return std::string(m_nvml.ErrorString(result));
    }

    nvmlReturn_t NvapiAdapter::NvmlDeviceGetTemperature(nvmlTemperatureSensors_t sensorType, unsigned int *temp) const {
        return m_nvml.DeviceGetTemperature(m_nvmlDevice, sensorType, temp);
    }

    nvmlReturn_t NvapiAdapter::NvmlDeviceGetUtilizationRates(nvmlUtilization_t *utilization) const {
        return m_nvml.DeviceGetUtilizationRates(m_nvmlDevice, utilization);
    }

    nvmlReturn_t NvapiAdapter::NvmlDeviceGetVbiosVersion(char* version, unsigned int length) const {
        return m_nvml.DeviceGetVbiosVersion(m_nvmlDevice, version, length);
    }

    nvmlReturn_t NvapiAdapter::NvmlDeviceGetClockInfo(nvmlClockType_t type, unsigned int* clock) const {
        return m_nvml.DeviceGetClockInfo(m_nvmlDevice, type, clock);
    }
}
