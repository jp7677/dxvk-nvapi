#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"

#include "../inc/catch.hpp"
#include "../inc/catch2/trompeloeil.hpp"
#include "../src/sysinfo/resource_factory.h"
#include "../src/dxvk/dxvk_interfaces.h"
#include "../src/nvapi.cpp"

using namespace trompeloeil;

class DXGIFactoryMock : public mock_interface<IDXGIFactory> {
    MAKE_MOCK2(QueryInterface, HRESULT(REFIID, void**), override);
    MAKE_MOCK0(AddRef, ULONG(), override);
    MAKE_MOCK0(Release, ULONG(), override);
    IMPLEMENT_MOCK3(GetPrivateData);
    IMPLEMENT_MOCK3(SetPrivateData);
    IMPLEMENT_MOCK2(SetPrivateDataInterface);
    IMPLEMENT_MOCK2(GetParent);
    IMPLEMENT_MOCK2(EnumAdapters);
    IMPLEMENT_MOCK2(MakeWindowAssociation);
    IMPLEMENT_MOCK1(GetWindowAssociation);
    IMPLEMENT_MOCK3(CreateSwapChain);
    IMPLEMENT_MOCK2(CreateSoftwareAdapter);
};

class IDXGIDxvkAdapter : public IDXGIAdapter, public IDXGIVkInteropAdapter {};

class DXGIDxvkAdapterMock : public mock_interface<IDXGIDxvkAdapter> {
    MAKE_MOCK2(QueryInterface, HRESULT(REFIID, void**), override);
    MAKE_MOCK0(AddRef, ULONG(), override);
    MAKE_MOCK0(Release, ULONG(), override);
    IMPLEMENT_MOCK3(GetPrivateData);
    IMPLEMENT_MOCK3(SetPrivateData);
    IMPLEMENT_MOCK2(SetPrivateDataInterface);
    IMPLEMENT_MOCK2(GetParent);
    IMPLEMENT_MOCK2(EnumOutputs);
    IMPLEMENT_MOCK1(GetDesc);
    IMPLEMENT_MOCK2(CheckInterfaceSupport);
    IMPLEMENT_MOCK2(GetVulkanHandles);
};

class DXGIOutputMock : public mock_interface<IDXGIOutput> {
    MAKE_MOCK2(QueryInterface, HRESULT(REFIID, void**), override);
    MAKE_MOCK0(AddRef, ULONG(), override);
    MAKE_MOCK0(Release, ULONG(), override);
    IMPLEMENT_MOCK3(GetPrivateData);
    IMPLEMENT_MOCK3(SetPrivateData);
    IMPLEMENT_MOCK2(SetPrivateDataInterface);
    IMPLEMENT_MOCK2(GetParent);
    IMPLEMENT_MOCK1(GetDesc);
    IMPLEMENT_MOCK4(GetDisplayModeList);
    IMPLEMENT_MOCK3(FindClosestMatchingMode);
    IMPLEMENT_MOCK0(WaitForVBlank);
    IMPLEMENT_MOCK2(TakeOwnership);
    IMPLEMENT_MOCK0(ReleaseOwnership);
    IMPLEMENT_MOCK1(GetGammaControlCapabilities);
    IMPLEMENT_MOCK1(SetGammaControl);
    IMPLEMENT_MOCK1(GetGammaControl);
    IMPLEMENT_MOCK1(SetDisplaySurface);
    IMPLEMENT_MOCK1(GetDisplaySurfaceData);
    IMPLEMENT_MOCK1(GetFrameStatistics);
};

class VulkanMock : public mock_interface<Vulkan> {
    IMPLEMENT_CONST_MOCK0(IsAvailable);
    IMPLEMENT_CONST_MOCK2(GetDeviceExtensions);
    IMPLEMENT_CONST_MOCK3(GetPhysicalDeviceProperties2);
    IMPLEMENT_CONST_MOCK3(GetPhysicalDeviceMemoryProperties2);
};

class NvmlMock : public mock_interface<Nvml> {
    IMPLEMENT_CONST_MOCK0(IsAvailable);
    IMPLEMENT_CONST_MOCK1(ErrorString);
    IMPLEMENT_CONST_MOCK2(DeviceGetHandleByPciBusId_v2);
    IMPLEMENT_CONST_MOCK3(DeviceGetTemperature);
    IMPLEMENT_CONST_MOCK2(DeviceGetUtilizationRates);
    IMPLEMENT_CONST_MOCK3(DeviceGetVbiosVersion);
    IMPLEMENT_CONST_MOCK3(DeviceGetClockInfo);
};

class MockFactory : public ResourceFactory {

    public:
    MockFactory(std::unique_ptr<IDXGIFactory> dxgiFactoryMock, std::unique_ptr<Vulkan> vulkanMock, std::unique_ptr<Nvml> nvmlMock)
        : m_dxgiFactoryMock(std::move(dxgiFactoryMock)), m_vulkanMock(std::move(vulkanMock)), m_nvmlMock(std::move(nvmlMock)) {};

    Com<IDXGIFactory> CreateDXGIFactory() override {
        Com<IDXGIFactory> dxgiFactory = m_dxgiFactoryMock.get();
        return dxgiFactory;
    };

    std::unique_ptr<Vulkan> CreateVulkan() override {
        return std::move(m_vulkanMock);
    }

    std::unique_ptr<Nvml> CreateNvml() override {
        return std::move(m_nvmlMock);
    }

    private:
    std::unique_ptr<IDXGIFactory> m_dxgiFactoryMock;
    std::unique_ptr<Vulkan> m_vulkanMock;
    std::unique_ptr<Nvml> m_nvmlMock;
};

TEST_CASE("Initialize returns device-not-found when DXVK reports no adapters") {
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

    auto factory = std::make_unique<MockFactory>(std::move(dxgiFactory), std::move(vulkan), std::move(nvml));
    SetResourceFactory(std::move(factory));
    REQUIRE(NvAPI_Initialize() == NVAPI_NVIDIA_DEVICE_NOT_FOUND);
    REQUIRE(NvAPI_Unload() == NVAPI_API_NOT_INITIALIZED);
}

TEST_CASE("Initialize and unloads returns OK") {
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

    ALLOW_CALL(adapter, Release())
        .RETURN(0);
    ALLOW_CALL(adapter, EnumOutputs(0U, _))
        .LR_SIDE_EFFECT(*_2 = static_cast<IDXGIOutput*>(&output))
        .RETURN(S_OK);
    ALLOW_CALL(adapter, EnumOutputs(1U, _))
        .RETURN(DXGI_ERROR_NOT_FOUND);
    ALLOW_CALL(adapter, QueryInterface(_, _))
        .LR_SIDE_EFFECT({
            if (_1 == IDXGIVkInteropAdapter::guid)
                *_2 = static_cast<IDXGIVkInteropAdapter*>(&adapter);
        })
        .RETURN(S_OK);
    ALLOW_CALL(adapter, GetVulkanHandles(_, _));

    ALLOW_CALL(output, Release())
        .RETURN(0);
    ALLOW_CALL(output, GetDesc(_))
        .SIDE_EFFECT({
            DXGI_OUTPUT_DESC desc = {L"Output1", {0,0,0,0}, 1, DXGI_MODE_ROTATION_UNSPECIFIED, nullptr};
            *_1 = desc;
        })
        .RETURN(S_OK);

    ALLOW_CALL(*vulkan, IsAvailable())
        .RETURN(true);
    ALLOW_CALL(*vulkan, GetDeviceExtensions(_, _))
        .RETURN(std::set<std::string>{"ext"});
    ALLOW_CALL(*vulkan, GetPhysicalDeviceProperties2(_, _, _))
        .SIDE_EFFECT({
            VkPhysicalDeviceProperties2 props2 {};
            strcpy(props2.properties.deviceName, "Device1");
            *_3 = props2;
        });
    ALLOW_CALL(*vulkan, GetPhysicalDeviceMemoryProperties2(_, _, _))
        .SIDE_EFFECT(*_3 = VkPhysicalDeviceMemoryProperties2{});

    ALLOW_CALL(*nvml, IsAvailable())
        .RETURN(false);

    auto factory = std::make_unique<MockFactory>(std::move(dxgiFactory), std::move(vulkan), std::move(nvml));
    SetResourceFactory(std::move(factory));
    REQUIRE(NvAPI_Initialize() == NVAPI_OK);
    REQUIRE(NvAPI_Unload() == NVAPI_OK);
}
