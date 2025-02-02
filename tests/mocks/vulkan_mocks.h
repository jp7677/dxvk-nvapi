#pragma once

#include "../../src/nvapi_private.h"
#include "../../src/shared/vk.h"

using namespace trompeloeil;

class VkDeviceMock {
    MAKE_MOCK4(vkCreateSemaphore, VkResult(VkDevice, const VkSemaphoreCreateInfo*, const VkAllocationCallbacks*, VkSemaphore*));
    MAKE_MOCK3(vkDestroySemaphore, void(VkDevice, VkSemaphore, const VkAllocationCallbacks*));
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

    [[nodiscard]] static std::array<std::unique_ptr<expectation>, 5> ConfigureLL2PFN(VkMock& mock) {
        return {
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
};
