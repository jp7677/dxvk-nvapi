#pragma once

#include "nvapi_tests_private.h"

class NvapiVulkanLowLatencyDeviceMock final : public dxvk::NvapiVulkanLowLatencyDevice {
  public:
    MAKE_CONST_MOCK0(GetSemaphore, auto()->VkSemaphore, override);
    MAKE_CONST_MOCK0(GetLowLatencyMode, auto()->bool, override);
    MAKE_MOCK1(SetLatencySleepMode, auto(std::nullptr_t)->VkResult, override);
    MAKE_MOCK3(SetLatencySleepMode, auto(bool, bool, uint32_t)->VkResult, override);
    MAKE_MOCK1(LatencySleep, auto(uint64_t)->VkResult, override);
    MAKE_MOCK1(GetLatencyTimings, auto(std::array<VkLatencyTimingsFrameReportNV, 64>&)->bool, override);
    MAKE_MOCK2(SetLatencyMarker, auto(uint64_t, VkLatencyMarkerNV)->void, override);
    MAKE_MOCK2(QueueNotifyOutOfBand, auto(VkQueue, VkOutOfBandQueueTypeNV)->void, override);

    NvapiVulkanLowLatencyDeviceMock(VkDevice device)
        : dxvk::NvapiVulkanLowLatencyDevice(
              device,
              VK_NULL_HANDLE,
              [](VkDevice, VkSemaphore semaphore, const VkAllocationCallbacks*) {},
              nullptr,
              nullptr,
              nullptr,
              nullptr,
              nullptr) {}
};
