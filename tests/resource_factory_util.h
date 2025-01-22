#pragma once

#include "nvapi_tests_private.h"
#include "nvapi_d3d_mocks.h"
#include "nvapi_sysinfo_mocks.h"
#include "nvapi_vulkan_mocks.h"

DXGIDxvkAdapterMock* CreateDXGIDxvkAdapterMock();
DXGIOutput6Mock* CreateDXGIOutput6Mock();

void SetupResourceFactory(
    std::unique_ptr<DXGIDxvkFactoryMock> dxgiFactory,
    std::unique_ptr<VkMock> vk,
    std::unique_ptr<NvmlMock> nvml,
    std::unique_ptr<LfxMock> lfx);

void ResetGlobals();

[[nodiscard]] std::array<std::unique_ptr<trompeloeil::expectation>, 23> ConfigureDefaultTestEnvironment(
    DXGIDxvkFactoryMock& dxgiFactory,
    VkMock& vk,
    NvmlMock& nvml,
    LfxMock& lfx,
    DXGIDxvkAdapterMock& adapter,
    DXGIOutput6Mock& output);

[[nodiscard]] std::array<std::unique_ptr<trompeloeil::expectation>, 40> ConfigureExtendedTestEnvironment(
    DXGIDxvkFactoryMock& dxgiFactory,
    VkMock& vk,
    NvmlMock& nvml,
    LfxMock& lfx,
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
    VkPhysicalDeviceCudaKernelLaunchPropertiesNV* cudaKernelLaunchProperties;
};

void ConfigureGetPhysicalDeviceProperties2(
    VkPhysicalDeviceProperties2* props,
    std::function<void(ConfigureProps)> configure);
