#pragma once

#include "nvapi_tests_private.h"
#include "nvapi_d3d_mocks.h"
#include "nvapi_sysinfo_mocks.h"
#include "mock_factory.h"

void SetupResourceFactory(
    std::unique_ptr<DXGIFactory1Mock> dxgiFactory,
    std::unique_ptr<Vulkan> vulkan,
    std::unique_ptr<Nvml> nvml,
    std::unique_ptr<Lfx> lfx);

[[nodiscard]] std::array<std::unique_ptr<expectation>, 17> ConfigureDefaultTestEnvironment(
    DXGIFactory1Mock& dxgiFactory,
    VulkanMock& vulkan,
    NvmlMock& nvml,
    LfxMock& lfx,
    DXGIDxvkAdapterMock& adapter,
    DXGIOutputMock& output);

[[nodiscard]] std::array<std::unique_ptr<expectation>, 29> ConfigureExtendedTestEnvironment(
    DXGIFactory1Mock& dxgiFactory,
    VulkanMock& vulkan,
    NvmlMock& nvml,
    LfxMock& lfx,
    DXGIDxvkAdapterMock& adapter1,
    DXGIDxvkAdapterMock& adapter2,
    DXGIOutputMock& output1,
    DXGIOutputMock& output2,
    DXGIOutputMock& output3);

void ConfigureGetPhysicalDeviceProperties2(
    VkPhysicalDeviceProperties2* props,
    std::function<void(
        VkPhysicalDeviceProperties*,
        VkPhysicalDeviceIDProperties*,
        VkPhysicalDevicePCIBusInfoPropertiesEXT*,
        VkPhysicalDeviceDriverPropertiesKHR*,
        VkPhysicalDeviceFragmentShadingRatePropertiesKHR*)>
        configure);
