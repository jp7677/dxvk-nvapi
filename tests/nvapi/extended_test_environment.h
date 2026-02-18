#pragma once

#include "nvapi_tests_private.h"
#include "../mocks/vulkan_mocks.h"
#include "../mocks/dxgi_mocks.h"
#include "../mocks/nvml_mocks.h"
#include "mock_factory.h"

using namespace trompeloeil;
using namespace dxvk;

class ExtendedTestEnvironment {
  public:
    ExtendedTestEnvironment();

    [[nodiscard]] std::array<std::unique_ptr<expectation>, 39> ConfigureExpectations();
    [[nodiscard]] DXGIDxvkFactoryMock* DXGIFactory() const { return mockFactory->GetDXGIFactoryMock(); }
    [[nodiscard]] VkMock* Vk() const { return mockFactory->GetVkMock(); }
    [[nodiscard]] NvmlMock* Nvml() const { return mockFactory->GetNvmlMock(); }
    [[nodiscard]] DXGIDxvkAdapterMock* DXGIAdapter1() const { return adapter1; }
    [[nodiscard]] DXGIDxvkAdapterMock* DXGIAdapter2() const { return adapter2; }
    [[nodiscard]] DXGIOutput6Mock* DXGIOutput1() const { return output1; }
    [[nodiscard]] DXGIOutput6Mock* DXGIOutput2() const { return output2; }
    [[nodiscard]] DXGIOutput6Mock* DXGIOutput3() const { return output3; }

  private:
    MockFactory* mockFactory;
    DXGIDxvkAdapterMock* adapter1;
    DXGIDxvkAdapterMock* adapter2;
    DXGIOutput6Mock* output1;
    DXGIOutput6Mock* output2;
    DXGIOutput6Mock* output3;
};