#pragma once

#include "../nvapi_private.h"
#include "./nvapi_resource_factory.h"

namespace dxvk {
    enum class VulkanLowLatencyImplementation {
        Fake,
        VkNvLowLatency2,
    };

    inline std::ostream& operator<<(std::ostream& stream, VulkanLowLatencyImplementation implementation) {
        switch (implementation) {
#define CASE(x)                             \
    case VulkanLowLatencyImplementation::x: \
        return stream << #x;
            CASE(Fake)
            CASE(VkNvLowLatency2)
#undef CASE
            default:
                return stream << "Unknown";
        }
    }

    class NvapiVulkanLowLatencyDevice {
      public:
        static bool Initialize(NvapiResourceFactory& resourceFactory);
        static void Reset();

        [[nodiscard]] static std::pair<NvapiVulkanLowLatencyDevice*, VkResult> GetOrCreate(VkDevice device, bool allowFake);
        [[nodiscard]] static NvapiVulkanLowLatencyDevice* Get(VkDevice device);
        static bool Destroy(VkDevice device);

        // Prevent default construction and copy semantics.
        NvapiVulkanLowLatencyDevice() = delete;
        NvapiVulkanLowLatencyDevice(const NvapiVulkanLowLatencyDevice&) = delete;
        NvapiVulkanLowLatencyDevice& operator=(const NvapiVulkanLowLatencyDevice&) = delete;

        [[nodiscard]] VkSemaphore GetSemaphore() const;

        [[nodiscard]] virtual VulkanLowLatencyImplementation GetImplementation() const = 0;
        [[nodiscard]] virtual NvBool GetLowLatencyMode() const = 0;
        [[nodiscard]] virtual VkResult SetLatencySleepMode(std::nullptr_t) = 0;
        [[nodiscard]] virtual VkResult SetLatencySleepMode(bool lowLatencyMode, bool lowLatencyBoost, uint32_t minimumIntervalUs) = 0;
        [[nodiscard]] virtual VkResult LatencySleep(uint64_t value) = 0;
        virtual void GetLatencyTimings(std::span<NV_VULKAN_LATENCY_RESULT_PARAMS_V1::vkFrameReport, 64> frameReports) = 0;
        [[nodiscard]] virtual bool SetLatencyMarker(uint64_t frameID, NV_VULKAN_LATENCY_MARKER_TYPE marker) = 0;
        virtual void QueueNotifyOutOfBand(VkQueue queue, NV_VULKAN_OUT_OF_BAND_QUEUE_TYPE queueType) = 0;

        virtual ~NvapiVulkanLowLatencyDevice();

      protected:
        [[nodiscard]] static std::pair<VkSemaphore, VkResult> CreateVkSemaphore(VkDevice device, PFN_vkCreateSemaphore vkCreateSemaphore);
        [[nodiscard]] explicit NvapiVulkanLowLatencyDevice(
            VkDevice device,
            VkSemaphore semaphore,
            PFN_vkDestroySemaphore vkDestroySemaphore);

        VkDevice m_device{};
        VkSemaphore m_semaphore{};
        PFN_vkDestroySemaphore m_vkDestroySemaphore{};
        static std::unique_ptr<Vk> m_vk;

      private:
        static std::unordered_map<VkDevice, std::unique_ptr<NvapiVulkanLowLatencyDevice>> m_nvapiDeviceMap;
        static std::mutex m_mutex;
    };

    class NvapiVulkanLowLatencyFakeDevice final : public NvapiVulkanLowLatencyDevice {
      public:
#define PFN_PARAM(proc) PFN_##proc proc
        [[nodiscard]] explicit NvapiVulkanLowLatencyFakeDevice(
            VkDevice device,
            VkSemaphore semaphore,
            PFN_PARAM(vkDestroySemaphore),
            PFN_PARAM(vkSignalSemaphore));
        [[nodiscard]] static std::pair<std::unique_ptr<NvapiVulkanLowLatencyFakeDevice>, VkResult> TryCreate(
            VkDevice device,
            PFN_PARAM(vkDestroySemaphore),
            PFN_PARAM(vkCreateSemaphore));
#undef PFN_PARAM

        [[nodiscard]] VulkanLowLatencyImplementation GetImplementation() const override { return VulkanLowLatencyImplementation::Fake; }
        [[nodiscard]] NvBool GetLowLatencyMode() const override;
        [[nodiscard]] VkResult SetLatencySleepMode(std::nullptr_t) override;
        [[nodiscard]] VkResult SetLatencySleepMode(bool lowLatencyMode, bool lowLatencyBoost, uint32_t minimumIntervalUs) override;
        [[nodiscard]] VkResult LatencySleep(uint64_t value) override;
        void GetLatencyTimings(std::span<NV_VULKAN_LATENCY_RESULT_PARAMS_V1::vkFrameReport, 64> frameReports) override;
        [[nodiscard]] bool SetLatencyMarker(uint64_t frameID, NV_VULKAN_LATENCY_MARKER_TYPE marker) override;
        void QueueNotifyOutOfBand(VkQueue queue, NV_VULKAN_OUT_OF_BAND_QUEUE_TYPE queueType) override;

      private:
        PFN_vkSignalSemaphore m_vkSignalSemaphore{};
    };

    class NvapiVulkanLowLatency2LayerDevice final : public NvapiVulkanLowLatencyDevice {
      public:
#define PFN_PARAM(proc) PFN_##proc proc
        [[nodiscard]] explicit NvapiVulkanLowLatency2LayerDevice(
            VkDevice device,
            VkSemaphore semaphore,
            PFN_PARAM(vkDestroySemaphore),
            PFN_PARAM(vkSetLatencySleepModeNV),
            PFN_PARAM(vkLatencySleepNV),
            PFN_PARAM(vkGetLatencyTimingsNV),
            PFN_PARAM(vkSetLatencyMarkerNV),
            PFN_PARAM(vkQueueNotifyOutOfBandNV));
        [[nodiscard]] static std::pair<std::unique_ptr<NvapiVulkanLowLatency2LayerDevice>, VkResult> TryCreate(
            VkDevice device,
            PFN_PARAM(vkDestroySemaphore),
            PFN_PARAM(vkCreateSemaphore));
#undef PFN_PARAM

        [[nodiscard]] VulkanLowLatencyImplementation GetImplementation() const override { return VulkanLowLatencyImplementation::VkNvLowLatency2; }
        [[nodiscard]] NvBool GetLowLatencyMode() const override;
        [[nodiscard]] VkResult SetLatencySleepMode(std::nullptr_t) override;
        [[nodiscard]] VkResult SetLatencySleepMode(bool lowLatencyMode, bool lowLatencyBoost, uint32_t minimumIntervalUs) override;
        [[nodiscard]] VkResult LatencySleep(uint64_t value) override;
        void GetLatencyTimings(std::span<NV_VULKAN_LATENCY_RESULT_PARAMS_V1::vkFrameReport, 64> frameReports) override;
        [[nodiscard]] bool SetLatencyMarker(uint64_t frameID, NV_VULKAN_LATENCY_MARKER_TYPE marker) override;
        void QueueNotifyOutOfBand(VkQueue queue, NV_VULKAN_OUT_OF_BAND_QUEUE_TYPE queueType) override;

      private:
        bool m_lowLatencyMode{};
#define PFN_MEMBER(proc) \
    PFN_##proc m_##proc {}
        PFN_MEMBER(vkSetLatencySleepModeNV);
        PFN_MEMBER(vkLatencySleepNV);
        PFN_MEMBER(vkGetLatencyTimingsNV);
        PFN_MEMBER(vkSetLatencyMarkerNV);
        PFN_MEMBER(vkQueueNotifyOutOfBandNV);
#undef PFN_MEMBER
    };
}
