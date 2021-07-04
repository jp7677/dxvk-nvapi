#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"

#include "../inc/catch.hpp"
#include "../inc/catch2/trompeloeil.hpp"
#include "../src/sysinfo/resource_factory.h"
#include "../src/dxvk/dxvk_interfaces.h"
#include "../src/nvapi.cpp"
#include "nvapi_sysinfo_mocks.cpp"
#include "mock_factory.cpp"

using namespace trompeloeil;

void SetupResourceFactory(std::unique_ptr<DXGIFactoryMock> dxgiFactory, std::unique_ptr<Vulkan> vulkan, std::unique_ptr<Nvml> nvml) {
    resourceFactory = std::make_unique<MockFactory>(std::move(dxgiFactory), std::move(vulkan), std::move(nvml));
    nvapiAdapterRegistry.reset();
    initializationCount = 0ULL;
}

TEST_CASE("Initialize returns device-not-found when DXVK reports no adapters", "[sysinfo]") {
    auto dxgiFactory = std::make_unique<DXGIFactoryMock>();
    auto vulkan = std::make_unique<VulkanMock>();
    auto nvml = std::make_unique<NvmlMock>();

    ALLOW_CALL(*dxgiFactory, AddRef())
        .RETURN(1);
    ALLOW_CALL(*dxgiFactory, Release())
        .RETURN(0);
    ALLOW_CALL(*dxgiFactory, EnumAdapters(_, _))
        .RETURN(DXGI_ERROR_NOT_FOUND);

    ALLOW_CALL(*vulkan, IsAvailable())
        .RETURN(true);

    ALLOW_CALL(*nvml, IsAvailable())
        .RETURN(false);

    SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml));

    SECTION("Initialize and unloads") {
        REQUIRE(NvAPI_Initialize() == NVAPI_NVIDIA_DEVICE_NOT_FOUND);
        REQUIRE(NvAPI_Unload() == NVAPI_API_NOT_INITIALIZED);
    }
}

TEST_CASE("Initialize and unloads returns OK", "[sysinfo]") {
    auto dxgiFactory = std::make_unique<DXGIFactoryMock>();
    DXGIDxvkAdapterMock adapter;
    DXGIOutputMock output;
    auto vulkan = std::make_unique<VulkanMock>();
    auto nvml = std::make_unique<NvmlMock>();

    ALLOW_CALL(*dxgiFactory, AddRef())
        .RETURN(1);
    ALLOW_CALL(*dxgiFactory, Release())
        .RETURN(0);
    ALLOW_CALL(*dxgiFactory, EnumAdapters(0U, _))
        .LR_SIDE_EFFECT(*_2 = static_cast<IDXGIAdapter*>(&adapter))
        .RETURN(S_OK);
    ALLOW_CALL(*dxgiFactory, EnumAdapters(1U, _))
        .RETURN(DXGI_ERROR_NOT_FOUND);

    ALLOW_CALL(adapter, QueryInterface(IDXGIVkInteropAdapter::guid, _))
        .LR_SIDE_EFFECT(*_2 = static_cast<IDXGIVkInteropAdapter*>(&adapter))
        .RETURN(S_OK);
    ALLOW_CALL(adapter, Release())
        .RETURN(0);
    ALLOW_CALL(adapter, EnumOutputs(0U, _))
        .LR_SIDE_EFFECT(*_2 = static_cast<IDXGIOutput*>(&output))
        .RETURN(S_OK);
    ALLOW_CALL(adapter, EnumOutputs(1U, _))
        .RETURN(DXGI_ERROR_NOT_FOUND);
    ALLOW_CALL(adapter, GetVulkanHandles(_, _));

    ALLOW_CALL(output, Release())
        .RETURN(0);
    ALLOW_CALL(output, GetDesc(_))
        .SIDE_EFFECT(*_1 = DXGI_OUTPUT_DESC{L"Output1", {0,0,0,0}, 1, DXGI_MODE_ROTATION_UNSPECIFIED, nullptr})
        .RETURN(S_OK);

    ALLOW_CALL(*vulkan, IsAvailable())
        .RETURN(true);
    ALLOW_CALL(*vulkan, GetDeviceExtensions(_, _))
        .RETURN(std::set<std::string>{"ext"});
    ALLOW_CALL(*vulkan, GetPhysicalDeviceProperties2(_, _, _))
        .SIDE_EFFECT(strcpy(_3->properties.deviceName, "Device1"));
    ALLOW_CALL(*vulkan, GetPhysicalDeviceMemoryProperties2(_, _, _));

    ALLOW_CALL(*nvml, IsAvailable())
        .RETURN(false);

    SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml));

    SECTION("Initialize and unloads") {
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);
        REQUIRE(NvAPI_Unload() == NVAPI_OK);
    }
}
