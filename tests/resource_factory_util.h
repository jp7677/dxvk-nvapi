#pragma once

#include "nvapi_tests_private.h"
#include "nvapi_d3d_mocks.h"
#include "nvapi_sysinfo_mocks.h"
#include "mock_factory.h"

void SetupResourceFactory(
    std::unique_ptr<DXGIFactory1Mock> dxgiFactory,
    std::unique_ptr<dxvk::Vulkan> vulkan,
    std::unique_ptr<dxvk::Nvml> nvml,
    std::unique_ptr<dxvk::Lfx> lfx);

[[nodiscard]] std::array<std::unique_ptr<trompeloeil::expectation>, 18> ConfigureDefaultTestEnvironment(
    DXGIFactory1Mock& dxgiFactory,
    VulkanMock& vulkan,
    NvmlMock& nvml,
    LfxMock& lfx,
    DXGIDxvkAdapterMock& adapter,
    DXGIOutput6Mock& output);

[[nodiscard]] std::array<std::unique_ptr<trompeloeil::expectation>, 32> ConfigureExtendedTestEnvironment(
    DXGIFactory1Mock& dxgiFactory,
    VulkanMock& vulkan,
    NvmlMock& nvml,
    LfxMock& lfx,
    DXGIDxvkAdapterMock& adapter1,
    DXGIDxvkAdapterMock& adapter2,
    DXGIOutput6Mock& output1,
    DXGIOutput6Mock& output2,
    DXGIOutput6Mock& output3);

[[nodiscard]] std::array<std::unique_ptr<trompeloeil::expectation>, 25> ConfigureIntegratedAndDiscreteGpuTestEnvironment(
    DXGIFactory1Mock& dxgiFactory,
    VulkanMock& vulkan,
    NvmlMock& nvml,
    LfxMock& lfx,
    DXGIDxvkAdapterMock& adapter1,
    DXGIDxvkAdapterMock& adapter2,
    DXGIOutput6Mock& output1);

void ConfigureGetPhysicalDeviceProperties2(
    VkPhysicalDeviceProperties2* props,
    std::function<void(
        VkPhysicalDeviceProperties*,
        VkPhysicalDeviceIDProperties*,
        VkPhysicalDevicePCIBusInfoPropertiesEXT*,
        VkPhysicalDeviceDriverPropertiesKHR*,
        VkPhysicalDeviceFragmentShadingRatePropertiesKHR*)>
        configure);
