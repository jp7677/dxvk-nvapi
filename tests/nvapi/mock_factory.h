#pragma once

#include "nvapi_tests_private.h"
#include "../../src/nvapi/nvapi_resource_factory.h"
#include "../mocks/vulkan_mocks.h"
#include "../mocks/dxgi_mocks.h"
#include "../mocks/nvml_mocks.h"
#include "../mocks/d3d12_mocks.h"

using namespace trompeloeil;

class MockFactory final : public dxvk::NvapiResourceFactory {

  public:
    MockFactory();

    dxvk::Com<IDXGIFactory1> CreateDXGIFactory1() override;
    dxvk::Com<ID3D12Device> CreateD3D12Device(dxvk::Com<IDXGIAdapter3>& dxgiAdapter, D3D_FEATURE_LEVEL feature_level) override;
    std::unique_ptr<dxvk::Vk> CreateVulkan(dxvk::Com<IDXGIFactory1>& dxgiFactory) override;
    std::unique_ptr<dxvk::Vk> CreateVulkan(const char* moduleName) override;
    std::unique_ptr<dxvk::Nvml> CreateNvml() override;

    [[nodiscard]] DXGIDxvkFactoryMock* GetDXGIFactoryMock() const;
    [[nodiscard]] D3D12Vkd3dDeviceMock* GetD3D12DeviceMock() const;
    [[nodiscard]] VkMock* GetVkMock() const;
    [[nodiscard]] NvmlMock* GetNvmlMock() const;

    [[nodiscard]] DXGIDxvkAdapterMock* CreateDXGIDxvkAdapterMock();
    [[nodiscard]] DXGIOutput6Mock* CreateDXGIOutput6Mock();
    [[nodiscard]] std::vector<std::unique_ptr<expectation>> ConfigureRelease();

  private:
    std::unique_ptr<DXGIDxvkFactoryMock> m_dxgiFactoryMock;
    std::unique_ptr<D3D12Vkd3dDeviceMock> m_d3d12DeviceMock;
    std::unique_ptr<VkMock> m_vkMock;
    std::unique_ptr<NvmlMock> m_nvmlMock;

    std::vector<std::unique_ptr<DXGIDxvkAdapterMock>> m_dxgiAdapterMocks;
    std::vector<std::unique_ptr<DXGIOutput6Mock>> m_dxgiOutputMocks;
};
