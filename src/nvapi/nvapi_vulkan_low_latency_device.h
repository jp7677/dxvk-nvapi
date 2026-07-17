#pragma once

#include "../nvapi_private.h"
#include "../shared/vk.h"

namespace dxvk {
    enum class LowLatencyDeviceImplementation : uint8_t {
        LowLatencyLegacy,
        LowLatency2,
        VkReflexFake
    };

    class NvapiVulkanLowLatencyDevice {

      public:
#define PFN_PARAM(proc) PFN_##proc proc
        explicit NvapiVulkanLowLatencyDevice(LowLatencyDeviceImplementation type, VkDevice device, VkSemaphore semaphore, PFN_PARAM(vkDestroySemaphore))
            : m_device(device), m_semaphore(semaphore), m_implementation(type), m_vkDestroySemaphore(vkDestroySemaphore) {}
#undef PFN_PARAM
        // Prevent default construction and copy semantics.
        NvapiVulkanLowLatencyDevice() = delete;
        NvapiVulkanLowLatencyDevice(const NvapiVulkanLowLatencyDevice&) = delete;
        NvapiVulkanLowLatencyDevice(NvapiVulkanLowLatencyDevice&&) = delete;
        NvapiVulkanLowLatencyDevice& operator=(const NvapiVulkanLowLatencyDevice&) = delete;
        NvapiVulkanLowLatencyDevice& operator=(NvapiVulkanLowLatencyDevice&&) = delete;

        [[nodiscard]] VkSemaphore GetSemaphore() const { return m_semaphore; }
        [[nodiscard]] NvBool GetLowLatencyMode();
        [[nodiscard]] VkResult SetLatencySleepMode(std::nullptr_t);
        [[nodiscard]] VkResult SetLatencySleepMode(bool lowLatencyMode, bool lowLatencyBoost, uint32_t minimumIntervalUs);
        [[nodiscard]] VkResult LatencySleep(uint64_t value);
        void GetLatencyTimings(std::span<NV_VULKAN_LATENCY_RESULT_PARAMS_V1::vkFrameReport, 64> frameReports);
        [[nodiscard]] bool SetLatencyMarker(uint64_t frameID, NV_VULKAN_LATENCY_MARKER_TYPE marker);
        void QueueNotifyOutOfBand(VkQueue queue, NV_VULKAN_OUT_OF_BAND_QUEUE_TYPE queueType);

        virtual ~NvapiVulkanLowLatencyDevice() { m_vkDestroySemaphore(m_device, m_semaphore, nullptr); }

      protected:
        VkDevice m_device;
        VkSemaphore m_semaphore;

      private:
        LowLatencyDeviceImplementation m_implementation;
        PFN_vkDestroySemaphore m_vkDestroySemaphore;
    };

    class NvapiVulkanLowLatencyLegacyDevice final : public NvapiVulkanLowLatencyDevice {

      public:
        [[nodiscard]] static std::pair<std::unique_ptr<NvapiVulkanLowLatencyLegacyDevice>, VkResult> TryCreate(Vk* vk, VkDevice device);

#define PFN_PARAM(proc) PFN_##proc proc
        [[nodiscard]] explicit NvapiVulkanLowLatencyLegacyDevice(
            VkDevice device,
            VkSemaphore semaphore,
            PFN_PARAM(vkDestroySemaphore),
            PFN_PARAM(vkSetLatencySleepModeLegacyNV),
            PFN_PARAM(vkLatencySleepLegacyNV),
            PFN_PARAM(vkGetLatencyTimingsLegacyNV),
            PFN_PARAM(vkSetLatencyMarkerLegacyNV),
            PFN_PARAM(vkQueueNotifyOutOfBandLegacyNV),
            PFN_PARAM(vkGetSleepStatusLegacyNV),
            PFN_PARAM(vkShutdownLatencyDeviceLegacyNV));
#undef PFN_PARAM

        [[nodiscard]] NvBool GetLowLatencyModeImpl();
        [[nodiscard]] VkResult SetLatencySleepModeImpl(std::nullptr_t);
        [[nodiscard]] VkResult SetLatencySleepModeImpl(bool lowLatencyMode, bool lowLatencyBoost, uint32_t minimumIntervalUs);
        [[nodiscard]] VkResult LatencySleepImpl(uint64_t value);
        void GetLatencyTimingsImpl(std::span<NV_VULKAN_LATENCY_RESULT_PARAMS_V1::vkFrameReport, 64> frameReports);
        [[nodiscard]] bool SetLatencyMarkerImpl(uint64_t frameID, NV_VULKAN_LATENCY_MARKER_TYPE marker);
        void QueueNotifyOutOfBandImpl(VkQueue queue, NV_VULKAN_OUT_OF_BAND_QUEUE_TYPE queueType);

        ~NvapiVulkanLowLatencyLegacyDevice() override { m_vkShutdownLatencyDeviceLegacyNV(m_device); }

      private:
#define PFN_MEMBER(proc) \
    PFN_##proc m_##proc {}
        PFN_MEMBER(vkSetLatencySleepModeLegacyNV);
        PFN_MEMBER(vkLatencySleepLegacyNV);
        PFN_MEMBER(vkGetLatencyTimingsLegacyNV);
        PFN_MEMBER(vkSetLatencyMarkerLegacyNV);
        PFN_MEMBER(vkQueueNotifyOutOfBandLegacyNV);
        PFN_MEMBER(vkGetSleepStatusLegacyNV);
        PFN_MEMBER(vkShutdownLatencyDeviceLegacyNV);
#undef PFN_MEMBER
    };

    class NvapiVulkanLowLatency2LayerDevice final : public NvapiVulkanLowLatencyDevice {

      public:
        [[nodiscard]] static std::pair<std::unique_ptr<NvapiVulkanLowLatency2LayerDevice>, VkResult> TryCreate(Vk* vk, VkDevice device);

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
#undef PFN_PARAM

        [[nodiscard]] NvBool GetLowLatencyModeImpl();
        [[nodiscard]] VkResult SetLatencySleepModeImpl(std::nullptr_t);
        [[nodiscard]] VkResult SetLatencySleepModeImpl(bool lowLatencyMode, bool lowLatencyBoost, uint32_t minimumIntervalUs);
        [[nodiscard]] VkResult LatencySleepImpl(uint64_t value);
        void GetLatencyTimingsImpl(std::span<NV_VULKAN_LATENCY_RESULT_PARAMS_V1::vkFrameReport, 64> frameReports);
        [[nodiscard]] bool SetLatencyMarkerImpl(uint64_t frameID, NV_VULKAN_LATENCY_MARKER_TYPE marker);
        void QueueNotifyOutOfBandImpl(VkQueue queue, NV_VULKAN_OUT_OF_BAND_QUEUE_TYPE queueType);

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

    class NvapiVulkanLowLatencyFakeDevice final : public NvapiVulkanLowLatencyDevice {

      public:
        [[nodiscard]] static std::pair<std::unique_ptr<NvapiVulkanLowLatencyFakeDevice>, VkResult> TryCreate(Vk* vk, VkDevice device);

#define PFN_PARAM(proc) PFN_##proc proc
        [[nodiscard]] explicit NvapiVulkanLowLatencyFakeDevice(
            VkDevice device,
            VkSemaphore semaphore,
            PFN_PARAM(vkDestroySemaphore),
            PFN_PARAM(vkSignalSemaphore));
#undef PFN_PARAM

        [[nodiscard]] NvBool GetLowLatencyModeImpl();
        [[nodiscard]] VkResult SetLatencySleepModeImpl(std::nullptr_t);
        [[nodiscard]] VkResult SetLatencySleepModeImpl(bool lowLatencyMode, bool lowLatencyBoost, uint32_t minimumIntervalUs);
        [[nodiscard]] VkResult LatencySleepImpl(uint64_t value);
        void GetLatencyTimingsImpl(std::span<NV_VULKAN_LATENCY_RESULT_PARAMS_V1::vkFrameReport, 64> frameReports);
        [[nodiscard]] bool SetLatencyMarkerImpl(uint64_t frameID, NV_VULKAN_LATENCY_MARKER_TYPE marker);
        void QueueNotifyOutOfBandImpl(VkQueue queue, NV_VULKAN_OUT_OF_BAND_QUEUE_TYPE queueType);

      private:
        PFN_vkSignalSemaphore m_vkSignalSemaphore{};
    };
}
