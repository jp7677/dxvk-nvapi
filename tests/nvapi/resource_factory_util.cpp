#include "resource_factory_util.h"
#include "mock_factory.h"

using namespace trompeloeil;
using namespace dxvk;

std::vector<std::unique_ptr<DXGIDxvkAdapterMock>> m_dxgiAdapterMocks;
std::vector<std::unique_ptr<DXGIOutput6Mock>> m_dxgiOutputMocks;

DXGIDxvkAdapterMock* CreateDXGIDxvkAdapterMock() {
    auto mock = std::make_unique<DXGIDxvkAdapterMock>();
    return m_dxgiAdapterMocks.emplace_back(std::move(mock)).get();
}

DXGIOutput6Mock* CreateDXGIOutput6Mock() {
    auto mock = std::make_unique<DXGIOutput6Mock>();
    return m_dxgiOutputMocks.emplace_back(std::move(mock)).get();
}

void SetupResourceFactory(
    std::unique_ptr<DXGIDxvkFactoryMock> dxgiFactory,
    std::unique_ptr<VkMock> vk,
    std::unique_ptr<NvmlMock> nvml) {
    resourceFactory = std::make_unique<MockFactory>(std::move(dxgiFactory), std::move(vk), std::move(nvml));
}

void ResetGlobals() {
    NvapiD3d11Device::Reset();
    NvapiD3d12Device::Reset();
    NvapiD3d12GraphicsCommandList::Reset();
    NvapiD3d12CommandQueue::Reset();
    NvapiD3dLowLatencyDevice::Reset();
    NvapiVulkanLowLatencyDevice::Reset();

    if (!resourceFactory)
        return;

    auto mockFactory = reinterpret_cast<MockFactory*>(resourceFactory.get());

    // Ensure that Com<*> mocks can be deleted by destructors
    std::vector<std::unique_ptr<expectation>> e;

    e.emplace_back(
        NAMED_ALLOW_CALL(*mockFactory->GetDXGIDxvkFactoryMock(), Release()).RETURN(0));

    std::for_each(m_dxgiAdapterMocks.begin(), m_dxgiAdapterMocks.end(),
        [&e](auto& mock) {
            e.emplace_back(
                NAMED_ALLOW_CALL(*mock, Release()).RETURN(0));
        });

    std::for_each(m_dxgiOutputMocks.begin(), m_dxgiOutputMocks.end(),
        [&e](auto& mock) {
            e.emplace_back(
                NAMED_ALLOW_CALL(*mock, Release()).RETURN(0));
        });

    nvapiAdapterRegistry.reset();
    initializationCount = 0ULL;
    resourceFactory.reset();

    m_dxgiAdapterMocks.clear();
    m_dxgiOutputMocks.clear();
}

[[nodiscard]] std::array<std::unique_ptr<expectation>, 22> ConfigureDefaultTestEnvironment(
    DXGIDxvkFactoryMock& dxgiFactory,
    VkMock& vk,
    NvmlMock& nvml,
    DXGIDxvkAdapterMock& adapter,
    DXGIOutput6Mock& output) {
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

        NAMED_ALLOW_CALL(dxgiFactory, QueryInterface(__uuidof(IDXGIVkInteropFactory1), _))
            .LR_SIDE_EFFECT(*_2 = static_cast<IDXGIVkInteropFactory1*>(&dxgiFactory))
            .RETURN(S_OK),
        NAMED_ALLOW_CALL(dxgiFactory, GetGlobalHDRState(_, _))
            .RETURN(S_OK),
        NAMED_ALLOW_CALL(dxgiFactory, SetGlobalHDRState(_, _))
            .RETURN(S_OK),

        NAMED_ALLOW_CALL(adapter, QueryInterface(__uuidof(IDXGIAdapter3), _))
            .LR_SIDE_EFFECT(*_2 = static_cast<IDXGIAdapter3*>(&adapter))
            .RETURN(S_OK),
        NAMED_ALLOW_CALL(adapter, QueryInterface(__uuidof(IDXGIVkInteropAdapter), _))
            .LR_SIDE_EFFECT(*_2 = static_cast<IDXGIVkInteropAdapter*>(&adapter))
            .RETURN(S_OK),
        NAMED_ALLOW_CALL(adapter, AddRef())
            .RETURN(1),
        NAMED_ALLOW_CALL(adapter, Release())
            .RETURN(0),
        NAMED_ALLOW_CALL(adapter, GetDesc1(_))
            .SIDE_EFFECT(_1->VendorId = 0x10de)
            .RETURN(S_OK),
        NAMED_ALLOW_CALL(adapter, EnumOutputs(0U, _))
            .LR_SIDE_EFFECT(*_2 = static_cast<IDXGIOutput*>(&output))
            .RETURN(S_OK),
        NAMED_ALLOW_CALL(adapter, EnumOutputs(1U, _))
            .RETURN(DXGI_ERROR_NOT_FOUND),
        NAMED_ALLOW_CALL(adapter, GetVulkanHandles(_, _)),

        NAMED_ALLOW_CALL(output, Release())
            .RETURN(0),
        NAMED_ALLOW_CALL(output, GetDesc(_))
            .SIDE_EFFECT(*_1 = DXGI_OUTPUT_DESC{L"Output1", {0, 0, 0, 0}, 1, DXGI_MODE_ROTATION_UNSPECIFIED, nullptr})
            .RETURN(S_OK),
        NAMED_ALLOW_CALL(output, QueryInterface(__uuidof(IDXGIOutput6), _))
            .RETURN(E_FAIL),

        NAMED_ALLOW_CALL(vk, IsAvailable())
            .RETURN(true),
        NAMED_ALLOW_CALL(vk, GetDeviceExtensions(_, _))
            .RETURN(std::set<std::string>{VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME}),
        NAMED_ALLOW_CALL(vk, GetPhysicalDeviceProperties2(_, _, _))
            .SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [](auto vkProps) {
                        strcpy(vkProps.props->deviceName, "Device1");
                        vkProps.props->deviceType = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
                        vkProps.driverProps->driverID = VK_DRIVER_ID_NVIDIA_PROPRIETARY;
                    })),

        NAMED_ALLOW_CALL(nvml, IsAvailable())
            .RETURN(false)};
}

[[nodiscard]] std::array<std::unique_ptr<expectation>, 39> ConfigureExtendedTestEnvironment(
    DXGIDxvkFactoryMock& dxgiFactory,
    VkMock& vk,
    NvmlMock& nvml,
    DXGIDxvkAdapterMock& adapter1,
    DXGIDxvkAdapterMock& adapter2,
    DXGIOutput6Mock& output1,
    DXGIOutput6Mock& output2,
    DXGIOutput6Mock& output3) {
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

        NAMED_ALLOW_CALL(dxgiFactory, QueryInterface(__uuidof(IDXGIVkInteropFactory1), _))
            .LR_SIDE_EFFECT(*_2 = static_cast<IDXGIVkInteropFactory1*>(&dxgiFactory))
            .RETURN(S_OK),
        NAMED_ALLOW_CALL(dxgiFactory, GetGlobalHDRState(_, _))
            .RETURN(S_OK),
        NAMED_ALLOW_CALL(dxgiFactory, SetGlobalHDRState(_, _))
            .RETURN(S_OK),

        NAMED_ALLOW_CALL(adapter1, QueryInterface(__uuidof(IDXGIAdapter3), _))
            .LR_SIDE_EFFECT(*_2 = static_cast<IDXGIAdapter3*>(&adapter1))
            .RETURN(S_OK),
        NAMED_ALLOW_CALL(adapter1, QueryInterface(__uuidof(IDXGIVkInteropAdapter), _))
            .LR_SIDE_EFFECT(*_2 = static_cast<IDXGIVkInteropAdapter*>(&adapter1))
            .RETURN(S_OK),
        NAMED_ALLOW_CALL(adapter1, AddRef())
            .RETURN(1),
        NAMED_ALLOW_CALL(adapter1, Release())
            .RETURN(0),
        NAMED_ALLOW_CALL(adapter1, GetDesc1(_))
            .SIDE_EFFECT(_1->VendorId = 0x10de)
            .RETURN(S_OK),
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

        NAMED_ALLOW_CALL(adapter2, QueryInterface(__uuidof(IDXGIAdapter3), _))
            .LR_SIDE_EFFECT(*_2 = static_cast<IDXGIAdapter3*>(&adapter2))
            .RETURN(S_OK),
        NAMED_ALLOW_CALL(adapter2, QueryInterface(__uuidof(IDXGIVkInteropAdapter), _))
            .LR_SIDE_EFFECT(*_2 = static_cast<IDXGIVkInteropAdapter*>(&adapter2))
            .RETURN(S_OK),
        NAMED_ALLOW_CALL(adapter2, AddRef())
            .RETURN(1),
        NAMED_ALLOW_CALL(adapter2, Release())
            .RETURN(0),
        NAMED_ALLOW_CALL(adapter2, GetDesc1(_))
            .SIDE_EFFECT(_1->VendorId = 0x10de)
            .RETURN(S_OK),
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
            .SIDE_EFFECT(*_1 = DXGI_OUTPUT_DESC{L"Output1", {0, 0, 0, 0}, 1, DXGI_MODE_ROTATION_UNSPECIFIED, nullptr})
            .RETURN(S_OK),
        NAMED_ALLOW_CALL(output1, QueryInterface(__uuidof(IDXGIOutput6), _))
            .RETURN(E_FAIL),

        NAMED_ALLOW_CALL(output2, Release())
            .RETURN(0),
        NAMED_ALLOW_CALL(output2, GetDesc(_))
            .SIDE_EFFECT(*_1 = DXGI_OUTPUT_DESC{L"Output2", {0, 0, 0, 0}, 1, DXGI_MODE_ROTATION_UNSPECIFIED, nullptr})
            .RETURN(S_OK),
        NAMED_ALLOW_CALL(output2, QueryInterface(__uuidof(IDXGIOutput6), _))
            .RETURN(E_FAIL),

        NAMED_ALLOW_CALL(output3, Release())
            .RETURN(0),
        NAMED_ALLOW_CALL(output3, GetDesc(_))
            .SIDE_EFFECT(*_1 = DXGI_OUTPUT_DESC{L"Output3", {0, 0, 0, 0}, 1, DXGI_MODE_ROTATION_UNSPECIFIED, nullptr})
            .RETURN(S_OK),
        NAMED_ALLOW_CALL(output3, QueryInterface(__uuidof(IDXGIOutput6), _))
            .RETURN(E_FAIL),

        NAMED_ALLOW_CALL(vk, IsAvailable())
            .RETURN(true),
        NAMED_ALLOW_CALL(vk, GetDeviceExtensions(_, _))
            .RETURN(std::set<std::string>{VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME}),
        NAMED_ALLOW_CALL(vk, GetPhysicalDeviceProperties2(_, reinterpret_cast<VkPhysicalDevice>(0x01), _))
            .SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [](auto vkProps) {
                        strcpy(vkProps.props->deviceName, "Device1");
                        vkProps.props->deviceType = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
                        vkProps.driverProps->driverID = VK_DRIVER_ID_NVIDIA_PROPRIETARY;
                    })),
        NAMED_ALLOW_CALL(vk, GetPhysicalDeviceProperties2(_, reinterpret_cast<VkPhysicalDevice>(0x02), _))
            .SIDE_EFFECT(
                ConfigureGetPhysicalDeviceProperties2(_3,
                    [](auto vkProps) {
                        strcpy(vkProps.props->deviceName, "Device2");
                        vkProps.props->deviceType = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
                        vkProps.driverProps->driverID = VK_DRIVER_ID_NVIDIA_PROPRIETARY;
                    })),

        NAMED_ALLOW_CALL(nvml, IsAvailable())
            .RETURN(false)};
}

void ConfigureGetPhysicalDeviceProperties2(
    VkPhysicalDeviceProperties2* props,
    std::function<void(ConfigureProps)> configure) { // NOLINT(performance-unnecessary-value-param)
    auto vkProps = ConfigureProps{
        .props = &props->properties,
        .idProps = nullptr,
        .pciBusInfoProps = nullptr,
        .driverProps = nullptr,
        .fragmentShadingRateProps = nullptr,
        .computeShaderDerivativesProps = nullptr};

    auto next = reinterpret_cast<VkBaseOutStructure*>(props);
    while (next != nullptr) {
        switch (next->sType) {
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES: {
                vkProps.idProps = reinterpret_cast<VkPhysicalDeviceIDProperties*>(next);
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PCI_BUS_INFO_PROPERTIES_EXT: {
                vkProps.pciBusInfoProps = reinterpret_cast<VkPhysicalDevicePCIBusInfoPropertiesEXT*>(next);
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES_KHR: {
                vkProps.driverProps = reinterpret_cast<VkPhysicalDeviceDriverPropertiesKHR*>(next);
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_PROPERTIES_KHR: {
                vkProps.fragmentShadingRateProps = reinterpret_cast<VkPhysicalDeviceFragmentShadingRatePropertiesKHR*>(next);
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COMPUTE_SHADER_DERIVATIVES_PROPERTIES_KHR: {
                vkProps.computeShaderDerivativesProps = reinterpret_cast<VkPhysicalDeviceComputeShaderDerivativesPropertiesKHR*>(next);
                break;
            }
            default:
                break;
        }

        next = next->pNext;
    }

    configure(vkProps);
}
