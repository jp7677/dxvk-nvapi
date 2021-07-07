#include "mock_factory.cpp"

void ResetResourceFactory() {
    resourceFactory.reset();
    nvapiAdapterRegistry.reset();
    initializationCount = 0ULL;
}

void SetupResourceFactory(std::unique_ptr<DXGIFactoryMock> dxgiFactory, std::unique_ptr<Vulkan> vulkan, std::unique_ptr<Nvml> nvml) {
    resourceFactory = std::make_unique<MockFactory>(std::move(dxgiFactory), std::move(vulkan), std::move(nvml));
    nvapiAdapterRegistry.reset();
    initializationCount = 0ULL;
}

void ConfigureGetPhysicalDeviceProperties2(
        VkPhysicalDeviceProperties2* props,
        std::function<void(
                VkPhysicalDeviceProperties*,
                VkPhysicalDeviceIDProperties*,
                VkPhysicalDevicePCIBusInfoPropertiesEXT*,
                VkPhysicalDeviceDriverPropertiesKHR*,
                VkPhysicalDeviceFragmentShadingRatePropertiesKHR*)> configure) { // NOLINT(performance-unnecessary-value-param)
    VkPhysicalDeviceIDProperties* idProps = nullptr;
    VkPhysicalDevicePCIBusInfoPropertiesEXT* pciBusInfoProps = nullptr;
    VkPhysicalDeviceDriverPropertiesKHR* driverProps = nullptr;
    VkPhysicalDeviceFragmentShadingRatePropertiesKHR* fragmentShadingRateProps = nullptr;

    struct VkStructure {VkStructureType sType;void* pNext;};
    auto next = reinterpret_cast<VkStructure*>(props);
    while (next != nullptr) {
        switch (next->sType) {
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES: {
                idProps = reinterpret_cast<VkPhysicalDeviceIDProperties*>(next);
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PCI_BUS_INFO_PROPERTIES_EXT: {
                pciBusInfoProps = reinterpret_cast<VkPhysicalDevicePCIBusInfoPropertiesEXT*>(next);
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES_KHR: {
                driverProps = reinterpret_cast<VkPhysicalDeviceDriverPropertiesKHR*>(next);
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_PROPERTIES_KHR: {
                fragmentShadingRateProps = reinterpret_cast<VkPhysicalDeviceFragmentShadingRatePropertiesKHR*>(next);
                break;
            }
            default:
                break;
        }

        next = reinterpret_cast<VkStructure*>(next->pNext);
    }

    configure(&props->properties, idProps, pciBusInfoProps, driverProps, fragmentShadingRateProps);
}