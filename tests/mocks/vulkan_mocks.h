#pragma once

#include "../../src/nvapi_private.h"
#include "../../src/shared/vk.h"

using namespace trompeloeil;

struct ConfigureProps {
    VkPhysicalDeviceProperties* props;
    VkPhysicalDeviceIDProperties* idProps;
    VkPhysicalDevicePCIBusInfoPropertiesEXT* pciBusInfoProps;
    VkPhysicalDeviceDriverPropertiesKHR* driverProps;
    VkPhysicalDeviceFragmentShadingRatePropertiesKHR* fragmentShadingRateProps;
    VkPhysicalDeviceComputeShaderDerivativesPropertiesKHR* computeShaderDerivativesProps;
};

class VkDeviceMock {
    MAKE_MOCK4(vkCreateSemaphore, VkResult(VkDevice, const VkSemaphoreCreateInfo*, const VkAllocationCallbacks*, VkSemaphore*));
    MAKE_MOCK3(vkDestroySemaphore, void(VkDevice, VkSemaphore, const VkAllocationCallbacks*));
    MAKE_MOCK2(vkSignalSemaphore, VkResult(VkDevice, const VkSemaphoreSignalInfo*));
    MAKE_MOCK3(vkSetLatencySleepModeNV, VkResult(VkDevice, VkSwapchainKHR, const VkLatencySleepModeInfoNV*));
    MAKE_MOCK3(vkLatencySleepNV, VkResult(VkDevice, VkSwapchainKHR, const VkLatencySleepInfoNV*));
    MAKE_MOCK3(vkGetLatencyTimingsNV, void(VkDevice, VkSwapchainKHR, VkGetLatencyMarkerInfoNV*));
    MAKE_MOCK3(vkSetLatencyMarkerNV, void(VkDevice, VkSwapchainKHR, const VkSetLatencyMarkerInfoNV*));

    static VkResult CreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore) {
        return reinterpret_cast<VkDeviceMock*>(device)->vkCreateSemaphore(device, pCreateInfo, pAllocator, pSemaphore);
    }
    static void DestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator) {
        reinterpret_cast<VkDeviceMock*>(device)->vkDestroySemaphore(device, semaphore, pAllocator);
    }
    static VkResult SignalSemaphore(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo) {
        return reinterpret_cast<VkDeviceMock*>(device)->vkSignalSemaphore(device, pSignalInfo);
    }
    static VkResult SetLatencySleepModeNV(VkDevice device, VkSwapchainKHR swapchain, const VkLatencySleepModeInfoNV* pSleepModeInfo) {
        return reinterpret_cast<VkDeviceMock*>(device)->vkSetLatencySleepModeNV(device, swapchain, pSleepModeInfo);
    }
    static VkResult LatencySleepNV(VkDevice device, VkSwapchainKHR swapchain, const VkLatencySleepInfoNV* pSleepInfo) {
        return reinterpret_cast<VkDeviceMock*>(device)->vkLatencySleepNV(device, swapchain, pSleepInfo);
    }
    static void GetLatencyTimingsNV(VkDevice device, VkSwapchainKHR swapchain, VkGetLatencyMarkerInfoNV* pLatencyMarkerInfo) {
        reinterpret_cast<VkDeviceMock*>(device)->vkGetLatencyTimingsNV(device, swapchain, pLatencyMarkerInfo);
    }
    static void SetLatencyMarkerNV(VkDevice device, VkSwapchainKHR swapchain, const VkSetLatencyMarkerInfoNV* pLatencyMarkerInfo) {
        reinterpret_cast<VkDeviceMock*>(device)->vkSetLatencyMarkerNV(device, swapchain, pLatencyMarkerInfo);
    }
};

class VkPhysicalDeviceMock {
    MAKE_MOCK3(vkGetPhysicalDeviceQueueFamilyProperties, void(VkPhysicalDevice, uint32_t*, VkQueueFamilyProperties*));

    static void GetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties* pQueueFamilyProperties) {
        reinterpret_cast<VkPhysicalDeviceMock*>(physicalDevice)->vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
    }
};

class VkQueueMock {
    MAKE_MOCK2(vkQueueNotifyOutOfBandNV, void(VkQueue, const VkOutOfBandQueueTypeInfoNV*));

    static void QueueNotifyOutOfBandNV(VkQueue queue, const VkOutOfBandQueueTypeInfoNV* pQueueTypeInfo) {
        reinterpret_cast<VkQueueMock*>(queue)->vkQueueNotifyOutOfBandNV(queue, pQueueTypeInfo);
    }
};

class VkMock final : public mock_interface<dxvk::Vk> {
    IMPLEMENT_CONST_MOCK0(IsAvailable);
    IMPLEMENT_CONST_MOCK2(GetInstanceProcAddr);
    IMPLEMENT_CONST_MOCK2(GetDeviceProcAddr);
    IMPLEMENT_CONST_MOCK2(GetDeviceExtensions);
    IMPLEMENT_CONST_MOCK3(GetPhysicalDeviceProperties2);

    [[nodiscard]] static std::array<std::unique_ptr<expectation>, 2> ConfigureDefaultPFN(VkMock& mock) {
        return {
            NAMED_ALLOW_CALL(mock, GetDeviceProcAddr(_, eq(std::string_view("vkCreateSemaphore"))))
                .RETURN(reinterpret_cast<PFN_vkVoidFunction>(VkDeviceMock::CreateSemaphore)),
            NAMED_ALLOW_CALL(mock, GetDeviceProcAddr(_, eq(std::string_view("vkDestroySemaphore"))))
                .RETURN(reinterpret_cast<PFN_vkVoidFunction>(VkDeviceMock::DestroySemaphore))};
    }

    [[nodiscard]] static std::array<std::unique_ptr<expectation>, 2> ConfigureSignalSemaphorePFN(VkMock& mock, bool khr) {
        return {
            NAMED_ALLOW_CALL(mock, GetDeviceProcAddr(_, eq(std::string_view("vkSignalSemaphore"))))
                .RETURN(reinterpret_cast<PFN_vkVoidFunction>(khr ? nullptr : VkDeviceMock::SignalSemaphore)),
            NAMED_ALLOW_CALL(mock, GetDeviceProcAddr(_, eq(std::string_view("vkSignalSemaphoreKHR"))))
                .RETURN(reinterpret_cast<PFN_vkVoidFunction>(khr ? VkDeviceMock::SignalSemaphore : nullptr))};
    }

    [[nodiscard]] static std::array<std::unique_ptr<expectation>, 6> ConfigureLL2PFN(VkMock& mock) {
        return {
            NAMED_ALLOW_CALL(mock, GetDeviceProcAddr(_, eq(std::string_view("vkSignalSemaphore"))))
                .RETURN(reinterpret_cast<PFN_vkVoidFunction>(VkDeviceMock::SignalSemaphore)),
            NAMED_ALLOW_CALL(mock, GetDeviceProcAddr(_, eq(std::string_view("vkSetLatencySleepModeNV"))))
                .RETURN(reinterpret_cast<PFN_vkVoidFunction>(VkDeviceMock::SetLatencySleepModeNV)),
            NAMED_ALLOW_CALL(mock, GetDeviceProcAddr(_, eq(std::string_view("vkLatencySleepNV"))))
                .RETURN(reinterpret_cast<PFN_vkVoidFunction>(VkDeviceMock::LatencySleepNV)),
            NAMED_ALLOW_CALL(mock, GetDeviceProcAddr(_, eq(std::string_view("vkGetLatencyTimingsNV"))))
                .RETURN(reinterpret_cast<PFN_vkVoidFunction>(VkDeviceMock::GetLatencyTimingsNV)),
            NAMED_ALLOW_CALL(mock, GetDeviceProcAddr(_, eq(std::string_view("vkSetLatencyMarkerNV"))))
                .RETURN(reinterpret_cast<PFN_vkVoidFunction>(VkDeviceMock::SetLatencyMarkerNV)),
            NAMED_ALLOW_CALL(mock, GetDeviceProcAddr(_, eq(std::string_view("vkQueueNotifyOutOfBandNV"))))
                .RETURN(reinterpret_cast<PFN_vkVoidFunction>(VkQueueMock::QueueNotifyOutOfBandNV))};
    }

    static void ConfigureGetPhysicalDeviceProperties2(
        VkPhysicalDeviceProperties2* props,
        std::function<void(ConfigureProps)> configure) { // NOLINT(performance-unnecessary-value-param)
        auto vkProps = ConfigureProps{
            .props = &props->properties,
            .idProps = nullptr,
            .pciBusInfoProps = nullptr,
            .driverProps = nullptr,
            .fragmentShadingRateProps = nullptr,
            .computeShaderDerivativesProps = nullptr};

        auto next = reinterpret_cast<VkBaseOutStructure*>(props);
        while (next != nullptr) {
            switch (next->sType) {
                case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES: {
                    vkProps.idProps = reinterpret_cast<VkPhysicalDeviceIDProperties*>(next);
                    break;
                }
                case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PCI_BUS_INFO_PROPERTIES_EXT: {
                    vkProps.pciBusInfoProps = reinterpret_cast<VkPhysicalDevicePCIBusInfoPropertiesEXT*>(next);
                    break;
                }
                case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES_KHR: {
                    vkProps.driverProps = reinterpret_cast<VkPhysicalDeviceDriverPropertiesKHR*>(next);
                    break;
                }
                case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_PROPERTIES_KHR: {
                    vkProps.fragmentShadingRateProps = reinterpret_cast<VkPhysicalDeviceFragmentShadingRatePropertiesKHR*>(next);
                    break;
                }
                case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COMPUTE_SHADER_DERIVATIVES_PROPERTIES_KHR: {
                    vkProps.computeShaderDerivativesProps = reinterpret_cast<VkPhysicalDeviceComputeShaderDerivativesPropertiesKHR*>(next);
                    break;
                }
                default:
                    break;
            }

            next = next->pNext;
        }

        configure(vkProps);
    }
};
