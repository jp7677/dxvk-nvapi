#pragma once

#include "nvapi_tests_private.h"
#include "../mocks/vulkan_mocks.h"
#include "../mocks/dxgi_mocks.h"
#include "../mocks/nvml_mocks.h"

DXGIDxvkAdapterMock* CreateDXGIDxvkAdapterMock();
DXGIOutput6Mock* CreateDXGIOutput6Mock();

void SetupResourceFactory(
    std::unique_ptr<DXGIDxvkFactoryMock> dxgiFactory,
    std::unique_ptr<VkMock> vk,
    std::unique_ptr<NvmlMock> nvml);

void ResetGlobals();

[[nodiscard]] std::array<std::unique_ptr<trompeloeil::expectation>, 22> ConfigureDefaultTestEnvironment(
    DXGIDxvkFactoryMock& dxgiFactory,
    VkMock& vk,
    NvmlMock& nvml,
    DXGIDxvkAdapterMock& adapter,
    DXGIOutput6Mock& output);

[[nodiscard]] std::array<std::unique_ptr<trompeloeil::expectation>, 39> ConfigureExtendedTestEnvironment(
    DXGIDxvkFactoryMock& dxgiFactory,
    VkMock& vk,
    NvmlMock& nvml,
    DXGIDxvkAdapterMock& adapter1,
    DXGIDxvkAdapterMock& adapter2,
    DXGIOutput6Mock& output1,
    DXGIOutput6Mock& output2,
    DXGIOutput6Mock& output3);

struct ConfigureProps {
    VkPhysicalDeviceProperties* props;
    VkPhysicalDeviceIDProperties* idProps;
    VkPhysicalDevicePCIBusInfoPropertiesEXT* pciBusInfoProps;
    VkPhysicalDeviceDriverPropertiesKHR* driverProps;
    VkPhysicalDeviceFragmentShadingRatePropertiesKHR* fragmentShadingRateProps;
    VkPhysicalDeviceComputeShaderDerivativesPropertiesKHR* computeShaderDerivativesProps;
};

void ConfigureGetPhysicalDeviceProperties2(
    VkPhysicalDeviceProperties2* props,
    std::function<void(ConfigureProps)> configure);
