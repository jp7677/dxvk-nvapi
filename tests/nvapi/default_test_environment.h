#pragma once

#include "nvapi_tests_private.h"
#include "../mocks/vulkan_mocks.h"
#include "../mocks/dxgi_mocks.h"
#include "../mocks/nvml_mocks.h"
#include "mock_factory.h"

using namespace trompeloeil;
using namespace dxvk;

class DefaultTestEnvironment {
  public:
    DefaultTestEnvironment();

    [[nodiscard]] std::array<std::unique_ptr<expectation>, 22> ConfigureExpectations();
    [[nodiscard]] DXGIDxvkFactoryMock* DXGIFactory() const { return mockFactory->GetDXGIFactoryMock(); }
    [[nodiscard]] D3D12Vkd3dDeviceMock* D3D12Device() const { return mockFactory->GetD3D12DeviceMock(); }
    [[nodiscard]] VkMock* Vk() const { return mockFactory->GetVkMock(); }
    [[nodiscard]] NvmlMock* Nvml() const { return mockFactory->GetNvmlMock(); }
    [[nodiscard]] DXGIDxvkAdapterMock* DXGIAdapter() const { return adapter; }
    [[nodiscard]] DXGIOutput6Mock* DXGIOutput() const { return output; }

  private:
    MockFactory* mockFactory;
    DXGIDxvkAdapterMock* adapter;
    DXGIOutput6Mock* output;
};