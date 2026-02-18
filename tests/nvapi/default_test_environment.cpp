#include "default_test_environment.h"

using namespace trompeloeil;
using namespace dxvk;

DefaultTestEnvironment::DefaultTestEnvironment() {
    resourceFactory = std::make_unique<MockFactory>();
    mockFactory = reinterpret_cast<MockFactory*>(resourceFactory.get());

    adapter = mockFactory->CreateDXGIDxvkAdapterMock();
    output = mockFactory->CreateDXGIOutput6Mock();
}

[[nodiscard]] std::array<std::unique_ptr<expectation>, 22> DefaultTestEnvironment::ConfigureExpectations() {
    auto dxgiFactory = mockFactory->GetDXGIFactoryMock();
    auto vk = mockFactory->GetVkMock();
    auto nvml = mockFactory->GetNvmlMock();
    return {
        NAMED_ALLOW_CALL(*dxgiFactory, AddRef())
            .RETURN(1),
        NAMED_ALLOW_CALL(*dxgiFactory, Release())
            .RETURN(0),
        NAMED_ALLOW_CALL(*dxgiFactory, EnumAdapters1(0U, _))
            .LR_SIDE_EFFECT(*_2 = static_cast<IDXGIAdapter1*>(adapter))
            .RETURN(S_OK),
        NAMED_ALLOW_CALL(*dxgiFactory, EnumAdapters1(1U, _))
            .RETURN(DXGI_ERROR_NOT_FOUND),

        NAMED_ALLOW_CALL(*dxgiFactory, QueryInterface(__uuidof(IDXGIVkInteropFactory1), _))
            .RETURN(E_NOINTERFACE),

        NAMED_ALLOW_CALL(*adapter, QueryInterface(__uuidof(IDXGIAdapter3), _))
            .LR_SIDE_EFFECT(*_2 = static_cast<IDXGIAdapter3*>(adapter))
            .RETURN(S_OK),
        NAMED_ALLOW_CALL(*adapter, QueryInterface(__uuidof(IDXGIVkInteropAdapter), _))
            .LR_SIDE_EFFECT(*_2 = static_cast<IDXGIVkInteropAdapter*>(adapter))
            .RETURN(S_OK),
        NAMED_ALLOW_CALL(*adapter, AddRef())
            .RETURN(1),
        NAMED_ALLOW_CALL(*adapter, Release())
            .RETURN(0),
        NAMED_ALLOW_CALL(*adapter, GetDesc1(_))
            .SIDE_EFFECT(_1->VendorId = 0x10de)
            .RETURN(S_OK),
        NAMED_ALLOW_CALL(*adapter, EnumOutputs(0U, _))
            .LR_SIDE_EFFECT(*_2 = static_cast<IDXGIOutput*>(output))
            .RETURN(S_OK),
        NAMED_ALLOW_CALL(*adapter, EnumOutputs(1U, _))
            .RETURN(DXGI_ERROR_NOT_FOUND),
        NAMED_ALLOW_CALL(*adapter, GetVulkanHandles(_, _)),

        NAMED_ALLOW_CALL(*output, Release())
            .RETURN(0),
        NAMED_ALLOW_CALL(*output, GetDesc(_))
            .SIDE_EFFECT(*_1 = DXGI_OUTPUT_DESC{L"Output1", {0, 0, 0, 0}, 1, DXGI_MODE_ROTATION_UNSPECIFIED, nullptr})
            .RETURN(S_OK),
        NAMED_ALLOW_CALL(*output, QueryInterface(__uuidof(IDXGIOutput6), _))
            .RETURN(E_FAIL),

        NAMED_ALLOW_CALL(*vk, IsAvailable())
            .RETURN(true),
        NAMED_ALLOW_CALL(*vk, GetDeviceExtensions(_, _))
            .RETURN(std::set<std::string>{VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME}),
        NAMED_ALLOW_CALL(*vk, GetPhysicalDeviceProperties2(_, _, _))
            .SIDE_EFFECT(
                VkMock::ConfigureGetPhysicalDeviceProperties2(_3,
                    [](auto vkProps) {
                        strcpy(vkProps.props->deviceName, "Device1");
                        vkProps.props->deviceType = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
                        vkProps.driverProps->driverID = VK_DRIVER_ID_NVIDIA_PROPRIETARY;
                    })),

        NAMED_ALLOW_CALL(*nvml, IsAvailable())
            .RETURN(false)};
}
