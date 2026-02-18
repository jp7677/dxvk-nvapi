#include "extended_test_environment.h"

using namespace trompeloeil;
using namespace dxvk;

ExtendedTestEnvironment::ExtendedTestEnvironment() {
    resourceFactory = std::make_unique<MockFactory>();
    mockFactory = reinterpret_cast<MockFactory*>(resourceFactory.get());

    adapter1 = mockFactory->CreateDXGIDxvkAdapterMock();
    adapter2 = mockFactory->CreateDXGIDxvkAdapterMock();
    output1 = mockFactory->CreateDXGIOutput6Mock();
    output2 = mockFactory->CreateDXGIOutput6Mock();
    output3 = mockFactory->CreateDXGIOutput6Mock();
}

[[nodiscard]] std::array<std::unique_ptr<expectation>, 39> ExtendedTestEnvironment::ConfigureExpectations() {
    auto dxgiFactory = mockFactory->GetDXGIFactoryMock();
    auto vk = mockFactory->GetVkMock();
    auto nvml = mockFactory->GetNvmlMock();
    return {
        NAMED_ALLOW_CALL(*dxgiFactory, AddRef())
            .RETURN(1),
        NAMED_ALLOW_CALL(*dxgiFactory, Release())
            .RETURN(0),
        NAMED_ALLOW_CALL(*dxgiFactory, EnumAdapters1(0U, _))
            .LR_SIDE_EFFECT(*_2 = static_cast<IDXGIAdapter1*>(adapter1))
            .RETURN(S_OK),
        NAMED_ALLOW_CALL(*dxgiFactory, EnumAdapters1(1U, _))
            .LR_SIDE_EFFECT(*_2 = static_cast<IDXGIAdapter1*>(adapter2))
            .RETURN(S_OK),
        NAMED_ALLOW_CALL(*dxgiFactory, EnumAdapters1(2U, _))
            .RETURN(DXGI_ERROR_NOT_FOUND),

        NAMED_ALLOW_CALL(*dxgiFactory, QueryInterface(__uuidof(IDXGIVkInteropFactory1), _))
            .RETURN(E_NOINTERFACE),

        NAMED_ALLOW_CALL(*adapter1, QueryInterface(__uuidof(IDXGIAdapter3), _))
            .LR_SIDE_EFFECT(*_2 = static_cast<IDXGIAdapter3*>(adapter1))
            .RETURN(S_OK),
        NAMED_ALLOW_CALL(*adapter1, QueryInterface(__uuidof(IDXGIVkInteropAdapter), _))
            .LR_SIDE_EFFECT(*_2 = static_cast<IDXGIVkInteropAdapter*>(adapter1))
            .RETURN(S_OK),
        NAMED_ALLOW_CALL(*adapter1, AddRef())
            .RETURN(1),
        NAMED_ALLOW_CALL(*adapter1, Release())
            .RETURN(0),
        NAMED_ALLOW_CALL(*adapter1, GetDesc1(_))
            .SIDE_EFFECT(_1->VendorId = 0x10de)
            .RETURN(S_OK),
        NAMED_ALLOW_CALL(*adapter1, EnumOutputs(0U, _))
            .LR_SIDE_EFFECT(*_2 = static_cast<IDXGIOutput*>(output1))
            .RETURN(S_OK),
        NAMED_ALLOW_CALL(*adapter1, EnumOutputs(1U, _))
            .LR_SIDE_EFFECT(*_2 = static_cast<IDXGIOutput*>(output2))
            .RETURN(S_OK),
        NAMED_ALLOW_CALL(*adapter1, EnumOutputs(2U, _))
            .RETURN(DXGI_ERROR_NOT_FOUND),
        NAMED_ALLOW_CALL(*adapter1, GetVulkanHandles(_, _))
            .LR_SIDE_EFFECT(*_2 = reinterpret_cast<VkPhysicalDevice>(0x01)),

        NAMED_ALLOW_CALL(*adapter2, QueryInterface(__uuidof(IDXGIAdapter3), _))
            .LR_SIDE_EFFECT(*_2 = static_cast<IDXGIAdapter3*>(adapter2))
            .RETURN(S_OK),
        NAMED_ALLOW_CALL(*adapter2, QueryInterface(__uuidof(IDXGIVkInteropAdapter), _))
            .LR_SIDE_EFFECT(*_2 = static_cast<IDXGIVkInteropAdapter*>(adapter2))
            .RETURN(S_OK),
        NAMED_ALLOW_CALL(*adapter2, AddRef())
            .RETURN(1),
        NAMED_ALLOW_CALL(*adapter2, Release())
            .RETURN(0),
        NAMED_ALLOW_CALL(*adapter2, GetDesc1(_))
            .SIDE_EFFECT(_1->VendorId = 0x10de)
            .RETURN(S_OK),
        NAMED_ALLOW_CALL(*adapter2, EnumOutputs(0U, _))
            .LR_SIDE_EFFECT(*_2 = static_cast<IDXGIOutput*>(output3))
            .RETURN(S_OK),
        NAMED_ALLOW_CALL(*adapter2, EnumOutputs(1U, _))
            .RETURN(DXGI_ERROR_NOT_FOUND),
        NAMED_ALLOW_CALL(*adapter2, GetVulkanHandles(_, _))
            .LR_SIDE_EFFECT(*_2 = reinterpret_cast<VkPhysicalDevice>(0x02)),

        NAMED_ALLOW_CALL(*output1, Release())
            .RETURN(0),
        NAMED_ALLOW_CALL(*output1, GetDesc(_))
            .SIDE_EFFECT(*_1 = DXGI_OUTPUT_DESC{L"Output1", {0, 0, 0, 0}, 1, DXGI_MODE_ROTATION_UNSPECIFIED, nullptr})
            .RETURN(S_OK),
        NAMED_ALLOW_CALL(*output1, QueryInterface(__uuidof(IDXGIOutput6), _))
            .RETURN(E_FAIL),

        NAMED_ALLOW_CALL(*output2, Release())
            .RETURN(0),
        NAMED_ALLOW_CALL(*output2, GetDesc(_))
            .SIDE_EFFECT(*_1 = DXGI_OUTPUT_DESC{L"Output2", {0, 0, 0, 0}, 1, DXGI_MODE_ROTATION_UNSPECIFIED, nullptr})
            .RETURN(S_OK),
        NAMED_ALLOW_CALL(*output2, QueryInterface(__uuidof(IDXGIOutput6), _))
            .RETURN(E_FAIL),

        NAMED_ALLOW_CALL(*output3, Release())
            .RETURN(0),
        NAMED_ALLOW_CALL(*output3, GetDesc(_))
            .SIDE_EFFECT(*_1 = DXGI_OUTPUT_DESC{L"Output3", {0, 0, 0, 0}, 1, DXGI_MODE_ROTATION_UNSPECIFIED, nullptr})
            .RETURN(S_OK),
        NAMED_ALLOW_CALL(*output3, QueryInterface(__uuidof(IDXGIOutput6), _))
            .RETURN(E_FAIL),

        NAMED_ALLOW_CALL(*vk, IsAvailable())
            .RETURN(true),
        NAMED_ALLOW_CALL(*vk, GetDeviceExtensions(_, _))
            .RETURN(std::set<std::string>{VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME}),
        NAMED_ALLOW_CALL(*vk, GetPhysicalDeviceProperties2(_, reinterpret_cast<VkPhysicalDevice>(0x01), _))
            .SIDE_EFFECT(
                VkMock::ConfigureGetPhysicalDeviceProperties2(_3,
                    [](auto vkProps) {
                        strcpy(vkProps.props->deviceName, "Device1");
                        vkProps.props->deviceType = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
                        vkProps.driverProps->driverID = VK_DRIVER_ID_NVIDIA_PROPRIETARY;
                    })),
        NAMED_ALLOW_CALL(*vk, GetPhysicalDeviceProperties2(_, reinterpret_cast<VkPhysicalDevice>(0x02), _))
            .SIDE_EFFECT(
                VkMock::ConfigureGetPhysicalDeviceProperties2(_3,
                    [](auto vkProps) {
                        strcpy(vkProps.props->deviceName, "Device2");
                        vkProps.props->deviceType = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
                        vkProps.driverProps->driverID = VK_DRIVER_ID_NVIDIA_PROPRIETARY;
                    })),

        NAMED_ALLOW_CALL(*nvml, IsAvailable())
            .RETURN(false)};
}
