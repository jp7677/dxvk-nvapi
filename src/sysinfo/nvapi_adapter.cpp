#include "nvapi_adapter.h"
#include "../dxvk/dxvk_interfaces.h"
#include "../util/util_string.h"
#include "../util/util_log.h"

namespace dxvk {
    NvapiAdapter::NvapiAdapter() = default;

    NvapiAdapter::~NvapiAdapter() = default;

    bool NvapiAdapter::Initialize(Com<IDXGIAdapter>& dxgiAdapter, std::vector<NvapiOutput*>& outputs) {
        // Get the Vulkan handle  from the DXGI adapter to get access to Vulkan device properties which has some information we want.
        Com<IDXGIVkInteropAdapter> dxgiVkInteropAdapter;
        if (FAILED(dxgiAdapter->QueryInterface(IID_PPV_ARGS(&dxgiVkInteropAdapter)))) {
            log::write("Querying Vulkan handle from DXGI adapter failed. Please ensure that DXVK's dxgi.dll is loaded.");
            return false;
        }

        const auto vkModuleName = "vulkan-1.dll";
        auto vkModule = LoadLibraryA(vkModuleName);
        if (vkModule == nullptr) {
            log::write(str::format("Loading ", vkModuleName, " failed with error code ", GetLastError()));
            return false;
        }

        auto vkGetInstanceProcAddr =
            reinterpret_cast<PFN_vkGetInstanceProcAddr>(
                reinterpret_cast<void*>(
                    GetProcAddress(vkModule, "vkGetInstanceProcAddr")));

        VkInstance vkInstance = VK_NULL_HANDLE;
        VkPhysicalDevice vkDevice = VK_NULL_HANDLE;
        dxgiVkInteropAdapter->GetVulkanHandles(&vkInstance, &vkDevice);

        m_devicePciBusProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PCI_BUS_INFO_PROPERTIES_EXT;
        m_devicePciBusProperties.pNext = nullptr;

        VkPhysicalDeviceProperties2 deviceProperties2;
        deviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        deviceProperties2.pNext = &m_devicePciBusProperties;

        auto vkGetPhysicalDeviceProperties2 =
            reinterpret_cast<PFN_vkGetPhysicalDeviceProperties2>(
                vkGetInstanceProcAddr(vkInstance, "vkGetPhysicalDeviceProperties2"));

        vkGetPhysicalDeviceProperties2(vkDevice, &deviceProperties2);
        m_deviceProperties = deviceProperties2.properties;

        VkPhysicalDeviceMemoryProperties2 memoryProperties2;
        memoryProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
        memoryProperties2.pNext = nullptr;

        auto vkGetPhysicalDeviceMemoryProperties2 =
            reinterpret_cast<PFN_vkGetPhysicalDeviceMemoryProperties2>(
                vkGetInstanceProcAddr(vkInstance, "vkGetPhysicalDeviceMemoryProperties2"));

        vkGetPhysicalDeviceMemoryProperties2(vkDevice, &memoryProperties2);
        m_memoryProperties = memoryProperties2.memoryProperties;

        if (m_deviceProperties.vendorID == 0x10de)
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

        // Query all outputs from DXVK (just one, unless the DXVK dxgi-multi-monitor branch is used)
        // Mosaic setup is not supported, thus one display output refers to one GPU
        Com<IDXGIOutput> dxgiOutput;
        for (auto i = 0U; dxgiAdapter->EnumOutputs(i, &dxgiOutput) != DXGI_ERROR_NOT_FOUND; i++) {
            auto nvapiOutput = new NvapiOutput((uintptr_t) this);
            nvapiOutput->Initialize(dxgiOutput);
            outputs.push_back(nvapiOutput);
        }

        FreeLibrary(vkModule);
        return true;
    }

    std::string NvapiAdapter::GetDeviceName() const {
        return std::string(m_deviceProperties.deviceName);
    }

    u_int NvapiAdapter::GetDriverVersion() const {
        // Windows releases can only ever have a two digit minor version
        // and does not have a patch number
        return VK_VERSION_MAJOR(m_vkDriverVersion) * 100 +
            std::min(VK_VERSION_MINOR(m_vkDriverVersion), (u_int) 99);
    }

    u_int NvapiAdapter::GetDeviceId() const {
        return (m_deviceProperties.deviceID << 16) + m_deviceProperties.vendorID;
    }

    u_int NvapiAdapter::GetGpuType() const {
        // The enum values for discrete, integrated and unknown GPU are the same for Vulkan and NvAPI
        auto vkDeviceType = m_deviceProperties.deviceType;
        if (vkDeviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU || vkDeviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
            return vkDeviceType;

        return VK_PHYSICAL_DEVICE_TYPE_OTHER;
    }

    u_int NvapiAdapter::GetBusId() const {
        return m_devicePciBusProperties.pciBus;
    }

    u_int NvapiAdapter::GetVRamSize() const {
        // Not sure if it is completely correct to just look at the first DEVICE_LOCAL heap,
        // but it seems to give the correct result.
        for (auto i = 0U; i < m_memoryProperties.memoryHeapCount; i++) {
            auto heap = m_memoryProperties.memoryHeaps[i];
            if (heap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
                return heap.size / 1024;
        }

        return 0;
    }
}
