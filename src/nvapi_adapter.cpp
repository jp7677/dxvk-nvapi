#include "nvapi_adapter.h"

namespace dxvk {
    NvapiAdapter::NvapiAdapter() {}
    NvapiAdapter::~NvapiAdapter() {}

    bool NvapiAdapter::Initialize() {
        Com<IDXGIFactory> dxgiFactory; 
        if(FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&dxgiFactory)))
            return false;

        // Query all D3D11 adapter from DXVK to honor any DXVK device filtering and
        // use the first NVIDIA card. Get the Vulkan handle  from that (DXVK) adapter
        // to get access to Vulkan device properties which has some information we want.
        Com<IDXGIAdapter> dxgiAdapter;
        for (u_int i = 0; dxgiFactory->EnumAdapters(i, &dxgiAdapter) != DXGI_ERROR_NOT_FOUND; ++i) {
            Com<￼IDXGIVkInteropAdapter> dxgiVkInteropAdapter;
            if (FAILED(dxgiAdapter->QueryInterface(IID_PPV_ARGS(&dxgiVkInteropAdapter))))
                return false;

            VkInstance vkInstance = VK_NULL_HANDLE;
            dxgiVkInteropAdapter->GetVulkanHandles(&vkInstance, &m_vkDevice);

            vkGetPhysicalDeviceProperties(m_vkDevice, &m_deviceProperties);
            if (m_deviceProperties.vendorID != 0x10de)
                continue; // No Nvidia card

            m_vkDriverVersion = VK_MAKE_VERSION(
                VK_VERSION_MAJOR(m_deviceProperties.driverVersion),
                VK_VERSION_MINOR(m_deviceProperties.driverVersion >> 0) >> 2,
                VK_VERSION_PATCH(m_deviceProperties.driverVersion >> 2) >> 4);

            std::cerr << "NvAPI Device: " << m_deviceProperties.deviceName << " ("<< std::dec << VK_VERSION_MAJOR(m_vkDriverVersion) << "." << VK_VERSION_MINOR(m_vkDriverVersion) << "." << VK_VERSION_PATCH(m_vkDriverVersion) << ")" << std::endl;

            return true;
        }

        // TODO: Support other vendors. Currently we depend on a NVIDIA GPU, though we don't do any NVIDIA specific stuff.
        std::cerr << "NvAPI Device: No NVIDIA GPU has been found" << std::endl;
        return false;
    }

    std::string NvapiAdapter::GetDeviceName() {
        return std::string(m_deviceProperties.deviceName);
    }

    u_int NvapiAdapter::GetDriverVersion() {
        // Windows releases can only ever have a two digit minor version
        // and does not have a patch number
        return VK_VERSION_MAJOR(m_vkDriverVersion) * 100 +
            std::min(VK_VERSION_MINOR(m_vkDriverVersion), (u_int)99); 
    }

    u_int NvapiAdapter::GetDeviceId() {
        return m_deviceProperties.deviceID;
    }

    u_int NvapiAdapter::GetGpuType() {
        // The enum values for discrete and unknown GPU are the same for Vulkan and NvAPI.
        VkPhysicalDeviceType vkDeviceType = m_deviceProperties.deviceType;
        if (vkDeviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU || vkDeviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
            return vkDeviceType;
        
        return VK_PHYSICAL_DEVICE_TYPE_OTHER;
    }
}
