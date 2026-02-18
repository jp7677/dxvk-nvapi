#include "mock_factory.h"

using namespace trompeloeil;

MockFactory::MockFactory() {
    m_dxgiFactoryMock = std::make_unique<DXGIDxvkFactoryMock>();
    m_d3d12DeviceMock = std::make_unique<D3D12Vkd3dDeviceMock>();
    m_vkMock = std::make_unique<VkMock>();
    m_nvmlMock = std::make_unique<NvmlMock>();
}

dxvk::Com<IDXGIFactory1> MockFactory::CreateDXGIFactory1() {
    return m_dxgiFactoryMock.get();
}

dxvk::Com<ID3D12Device> MockFactory::CreateD3D12Device(dxvk::Com<IDXGIAdapter3>& dxgiAdapter, D3D_FEATURE_LEVEL feature_level) {
    return m_d3d12DeviceMock.get();
}

std::unique_ptr<dxvk::Vk> MockFactory::CreateVulkan(dxvk::Com<IDXGIFactory1>& dxgiFactory) {
    return std::move(m_vkMock);
}

std::unique_ptr<dxvk::Vk> MockFactory::CreateVulkan(const char* moduleName) {
    return std::move(m_vkMock);
}

std::unique_ptr<dxvk::Nvml> MockFactory::CreateNvml() {
    return std::move(m_nvmlMock);
}

DXGIDxvkFactoryMock* MockFactory::GetDXGIFactoryMock() const {
    return m_dxgiFactoryMock.get();
}

D3D12Vkd3dDeviceMock* MockFactory::GetD3D12DeviceMock() const {
    return m_d3d12DeviceMock.get();
}

VkMock* MockFactory::GetVkMock() const {
    REQUIRE(m_vkMock); // will be moved after NvAPI_Initialize()
    return m_vkMock.get();
}

NvmlMock* MockFactory::GetNvmlMock() const {
    REQUIRE(m_nvmlMock); // will be moved after NvAPI_Initialize()
    return m_nvmlMock.get();
}

DXGIDxvkAdapterMock* MockFactory::CreateDXGIDxvkAdapterMock() {
    auto mock = std::make_unique<DXGIDxvkAdapterMock>();
    return m_dxgiAdapterMocks.emplace_back(std::move(mock)).get();
}

DXGIOutput6Mock* MockFactory::CreateDXGIOutput6Mock() {
    auto mock = std::make_unique<DXGIOutput6Mock>();
    return m_dxgiOutputMocks.emplace_back(std::move(mock)).get();
}

std::vector<std::unique_ptr<expectation>> MockFactory::ConfigureRelease() {
    std::vector<std::unique_ptr<expectation>> e;

    e.emplace_back(
        NAMED_ALLOW_CALL(*m_dxgiFactoryMock, Release()).RETURN(0));

    e.emplace_back(
        NAMED_ALLOW_CALL(*m_d3d12DeviceMock, Release()).RETURN(0));

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

    return e;
}
