#pragma once

#include "../nvapi_private.h"
#include "./resource_factory.h"

namespace dxvk {
    class NvapiVulkanLowLatencyDevice {
      public:
        static bool Initialize(ResourceFactory& resourceFactory);
        static std::pair<NvapiVulkanLowLatencyDevice*, bool> Insert(std::unique_ptr<NvapiVulkanLowLatencyDevice>&& lowLatencyDevice);
        static void Reset();

        [[nodiscard]] static std::pair<NvapiVulkanLowLatencyDevice*, VkResult> GetOrCreate(VkDevice device);
        [[nodiscard]] static NvapiVulkanLowLatencyDevice* Get(VkDevice device);
        [[nodiscard]] static bool Destroy(VkDevice device);

        [[nodiscard]] static VkLatencyMarkerNV ToVkLatencyMarkerNV(NV_VULKAN_LATENCY_MARKER_TYPE marker);

#define PFN_PARAM(proc) PFN_##proc proc
        [[nodiscard]] explicit NvapiVulkanLowLatencyDevice(
            VkDevice device,
            VkSemaphore semaphore,
            PFN_PARAM(vkDestroySemaphore),
            PFN_PARAM(vkSetLatencySleepModeNV),
            PFN_PARAM(vkLatencySleepNV),
            PFN_PARAM(vkGetLatencyTimingsNV),
            PFN_PARAM(vkSetLatencyMarkerNV),
            PFN_PARAM(vkQueueNotifyOutOfBandNV));
#undef PFN_PARAM

        [[nodiscard]] virtual VkSemaphore GetSemaphore() const;

        [[nodiscard]] virtual bool GetLowLatencyMode() const;

        [[nodiscard]] virtual VkResult SetLatencySleepMode(std::nullptr_t);
        [[nodiscard]] virtual VkResult SetLatencySleepMode(bool lowLatencyMode, bool lowLatencyBoost, uint32_t minimumIntervalUs);
        [[nodiscard]] virtual VkResult LatencySleep(uint64_t value);
        [[nodiscard]] virtual bool GetLatencyTimings(std::array<VkLatencyTimingsFrameReportNV, 64>& timings);
        virtual void SetLatencyMarker(uint64_t presentID, VkLatencyMarkerNV marker);
        virtual void QueueNotifyOutOfBand(VkQueue queue, VkOutOfBandQueueTypeNV queueType);

      private:
        static std::unique_ptr<Vk> m_vk;
        static std::unordered_map<VkDevice, std::unique_ptr<NvapiVulkanLowLatencyDevice>> m_lowLatencyDeviceMap;
        static std::mutex m_mutex;

        VkDevice m_device{};
        VkSemaphore m_semaphore{};
        bool m_lowLatencyMode{};
#define PFN_MEMBER(proc) \
    PFN_##proc m_##proc {}
        PFN_MEMBER(vkDestroySemaphore);
        PFN_MEMBER(vkSetLatencySleepModeNV);
        PFN_MEMBER(vkLatencySleepNV);
        PFN_MEMBER(vkGetLatencyTimingsNV);
        PFN_MEMBER(vkSetLatencyMarkerNV);
        PFN_MEMBER(vkQueueNotifyOutOfBandNV);
#undef PFN_MEMBER
    };
}