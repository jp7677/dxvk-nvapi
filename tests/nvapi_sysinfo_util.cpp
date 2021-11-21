#include "mock_factory.cpp"

void ResetResourceFactory() {
    resourceFactory.reset();
    nvapiAdapterRegistry.reset();
    initializationCount = 0ULL;
}

void SetupResourceFactory(
        std::unique_ptr<DXGIFactory1Mock> dxgiFactory,
        std::unique_ptr<Vulkan> vulkan,
        std::unique_ptr<Nvml> nvml) {
    resourceFactory = std::make_unique<MockFactory>(std::move(dxgiFactory), std::move(vulkan), std::move(nvml));
    nvapiAdapterRegistry.reset();
    initializationCount = 0ULL;
}

[[nodiscard]] std::array<std::unique_ptr<expectation>, 16> ConfigureDefaultTestEnvironment(
        DXGIFactory1Mock& dxgiFactory,
        VulkanMock& vulkan,
        NvmlMock& nvml,
        DXGIDxvkAdapterMock& adapter,
        DXGIOutputMock& output) {
    return {
        NAMED_ALLOW_CALL(dxgiFactory, AddRef())
            .RETURN(1),
        NAMED_ALLOW_CALL(dxgiFactory, Release())
            .RETURN(0),
        NAMED_ALLOW_CALL(dxgiFactory, EnumAdapters1(0U, _))
            .LR_SIDE_EFFECT(*_2 = static_cast<IDXGIAdapter1*>(&adapter))
            .RETURN(S_OK),
        NAMED_ALLOW_CALL(dxgiFactory, EnumAdapters1(1U, _))
            .RETURN(DXGI_ERROR_NOT_FOUND),

        NAMED_ALLOW_CALL(adapter, QueryInterface(IDXGIVkInteropAdapter::guid, _))
            .LR_SIDE_EFFECT(*_2 = static_cast<IDXGIVkInteropAdapter*>(&adapter))
            .RETURN(S_OK),
        NAMED_ALLOW_CALL(adapter, Release())
            .RETURN(0),
        NAMED_ALLOW_CALL(adapter, EnumOutputs(0U, _))
            .LR_SIDE_EFFECT(*_2 = static_cast<IDXGIOutput*>(&output))
            .RETURN(S_OK),
        NAMED_ALLOW_CALL(adapter, EnumOutputs(1U, _))
            .RETURN(DXGI_ERROR_NOT_FOUND),
        NAMED_ALLOW_CALL(adapter, GetVulkanHandles(_, _)),

        NAMED_ALLOW_CALL(output, Release())
            .RETURN(0),
        NAMED_ALLOW_CALL(output, GetDesc(_))
            .SIDE_EFFECT(*_1 = DXGI_OUTPUT_DESC{L"Output1", {0,0,0,0}, 1, DXGI_MODE_ROTATION_UNSPECIFIED, nullptr})
            .RETURN(S_OK),

        NAMED_ALLOW_CALL(vulkan, IsAvailable())
            .RETURN(true),
        NAMED_ALLOW_CALL(vulkan, GetDeviceExtensions(_, _))
            .RETURN(std::set<std::string>{"ext"}),
        NAMED_ALLOW_CALL(vulkan, GetPhysicalDeviceProperties2(_, _, _))
            .SIDE_EFFECT(strcpy(_3->properties.deviceName, "Device1")),
        NAMED_ALLOW_CALL(vulkan, GetPhysicalDeviceMemoryProperties2(_, _, _)),

        NAMED_ALLOW_CALL(nvml, IsAvailable())
            .RETURN(false)
    };
}

[[nodiscard]] std::array<std::unique_ptr<expectation>, 28> ConfigureExtendedTestEnvironment(
        DXGIFactory1Mock& dxgiFactory,
        VulkanMock& vulkan,
        NvmlMock& nvml,
        DXGIDxvkAdapterMock& adapter1,
        DXGIDxvkAdapterMock& adapter2,
        DXGIOutputMock& output1,
        DXGIOutputMock& output2,
        DXGIOutputMock& output3) {
    return {
        NAMED_ALLOW_CALL(dxgiFactory, AddRef())
            .RETURN(1),
        NAMED_ALLOW_CALL(dxgiFactory, Release())
            .RETURN(0),
        NAMED_ALLOW_CALL(dxgiFactory, EnumAdapters1(0U, _))
            .LR_SIDE_EFFECT(*_2 = static_cast<IDXGIAdapter1*>(&adapter1))
            .RETURN(S_OK),
        NAMED_ALLOW_CALL(dxgiFactory, EnumAdapters1(1U, _))
            .LR_SIDE_EFFECT(*_2 = static_cast<IDXGIAdapter1*>(&adapter2))
            .RETURN(S_OK),
        NAMED_ALLOW_CALL(dxgiFactory, EnumAdapters1(2U, _))
            .RETURN(DXGI_ERROR_NOT_FOUND),

        NAMED_ALLOW_CALL(adapter1, QueryInterface(IDXGIVkInteropAdapter::guid, _))
            .LR_SIDE_EFFECT(*_2 = static_cast<IDXGIVkInteropAdapter*>(&adapter1))
            .RETURN(S_OK),
        NAMED_ALLOW_CALL(adapter1, Release())
            .RETURN(0),
        NAMED_ALLOW_CALL(adapter1, EnumOutputs(0U, _))
            .LR_SIDE_EFFECT(*_2 = static_cast<IDXGIOutput*>(&output1))
            .RETURN(S_OK),
        NAMED_ALLOW_CALL(adapter1, EnumOutputs(1U, _))
            .LR_SIDE_EFFECT(*_2 = static_cast<IDXGIOutput*>(&output2))
            .RETURN(S_OK),
        NAMED_ALLOW_CALL(adapter1, EnumOutputs(2U, _))
            .RETURN(DXGI_ERROR_NOT_FOUND),
        NAMED_ALLOW_CALL(adapter1, GetVulkanHandles(_, _))
            .LR_SIDE_EFFECT(*_2 = reinterpret_cast<VkPhysicalDevice>(0x01)),

        NAMED_ALLOW_CALL(adapter2, QueryInterface(IDXGIVkInteropAdapter::guid, _))
            .LR_SIDE_EFFECT(*_2 = static_cast<IDXGIVkInteropAdapter*>(&adapter2))
            .RETURN(S_OK),
        NAMED_ALLOW_CALL(adapter2, Release())
            .RETURN(0),
        NAMED_ALLOW_CALL(adapter2, EnumOutputs(0U, _))
            .LR_SIDE_EFFECT(*_2 = static_cast<IDXGIOutput*>(&output3))
            .RETURN(S_OK),
        NAMED_ALLOW_CALL(adapter2, EnumOutputs(1U, _))
            .RETURN(DXGI_ERROR_NOT_FOUND),
        NAMED_ALLOW_CALL(adapter2, GetVulkanHandles(_, _))
            .LR_SIDE_EFFECT(*_2 = reinterpret_cast<VkPhysicalDevice>(0x02)),

        NAMED_ALLOW_CALL(output1, Release())
            .RETURN(0),
        NAMED_ALLOW_CALL(output1, GetDesc(_))
            .SIDE_EFFECT(*_1 = DXGI_OUTPUT_DESC{L"Output1", {0,0,0,0}, 1, DXGI_MODE_ROTATION_UNSPECIFIED, nullptr})
            .RETURN(S_OK),

        NAMED_ALLOW_CALL(output2, Release())
            .RETURN(0),
        NAMED_ALLOW_CALL(output2, GetDesc(_))
            .SIDE_EFFECT(*_1 = DXGI_OUTPUT_DESC{L"Output2", {0,0,0,0}, 1, DXGI_MODE_ROTATION_UNSPECIFIED, nullptr})
            .RETURN(S_OK),

        NAMED_ALLOW_CALL(output3, Release())
            .RETURN(0),
        NAMED_ALLOW_CALL(output3, GetDesc(_))
            .SIDE_EFFECT(*_1 = DXGI_OUTPUT_DESC{L"Output3", {0,0,0,0}, 1, DXGI_MODE_ROTATION_UNSPECIFIED, nullptr})
            .RETURN(S_OK),

        NAMED_ALLOW_CALL(vulkan, IsAvailable())
            .RETURN(true),
        NAMED_ALLOW_CALL(vulkan, GetDeviceExtensions(_, _))
            .RETURN(std::set<std::string>{"ext"}),
        NAMED_ALLOW_CALL(vulkan, GetPhysicalDeviceProperties2(_, reinterpret_cast<VkPhysicalDevice>(0x01), _))
            .SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [](auto props, auto idProps, auto pciBusInfoProps, auto driverProps, auto fragmentShadingRateProps) {
                        strcpy(props->deviceName, "Device1");
                    })
            ),
        NAMED_ALLOW_CALL(vulkan, GetPhysicalDeviceProperties2(_, reinterpret_cast<VkPhysicalDevice>(0x02), _))
            .SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [](auto props, auto idProps, auto pciBusInfoProps, auto driverProps, auto fragmentShadingRateProps) {
                        strcpy(props->deviceName, "Device2");
                    })
            ),
        NAMED_ALLOW_CALL(vulkan, GetPhysicalDeviceMemoryProperties2(_, _, _)),

        NAMED_ALLOW_CALL(nvml, IsAvailable())
            .RETURN(false)
    };
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