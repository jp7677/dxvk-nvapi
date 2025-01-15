#include "nvapi_tests_private.h"
#include "resource_factory_util.h"

using namespace trompeloeil;

TEST_CASE("Vulkan low latency fails to initialize when Vulkan is not available", "[.vulkan]") {
    auto dxgiFactory = std::make_unique<DXGIDxvkFactoryMock>();
    auto vk = std::make_unique<VkMock>();
    auto nvml = std::make_unique<NvmlMock>();
    auto lfx = std::make_unique<LfxMock>();

    ALLOW_CALL(*vk, IsAvailable()).RETURN(false);
    FORBID_CALL(*vk, GetDeviceProcAddr(_, _));

    SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml), std::move(lfx));

    dxvk::NvapiVulkanLowLatencyDevice::Reset();

    REQUIRE_FALSE(dxvk::NvapiVulkanLowLatencyDevice::Initialize(*resourceFactory));

    auto vkDevice = reinterpret_cast<VkDevice>(__LINE__);

    auto [device, result] = dxvk::NvapiVulkanLowLatencyDevice::GetOrCreate(vkDevice);

    REQUIRE(device == nullptr);
    REQUIRE(result == VK_ERROR_INITIALIZATION_FAILED);

    dxvk::NvapiVulkanLowLatencyDevice::Reset();
}

TEST_CASE("Vulkan low latency device methods succeed", "[.vulkan]") {
    PFN_vkCreateSemaphore vkCreateSemaphore = [](VkDevice, const VkSemaphoreCreateInfo*, const VkAllocationCallbacks*, VkSemaphore*) { return VK_ERROR_OUT_OF_HOST_MEMORY; };
    PFN_vkDestroySemaphore vkDestroySemaphore = [](VkDevice, VkSemaphore semaphore, const VkAllocationCallbacks*) { delete reinterpret_cast<uint64_t*>(semaphore); };
    PFN_vkSetLatencySleepModeNV vkSetLatencySleepModeNV = [](VkDevice, VkSwapchainKHR, const VkLatencySleepModeInfoNV*) { return VK_SUCCESS; };
    PFN_vkLatencySleepNV vkLatencySleepNV = [](VkDevice, VkSwapchainKHR, const VkLatencySleepInfoNV* latencySleepInfo) { *reinterpret_cast<uint64_t*>(latencySleepInfo->signalSemaphore) = latencySleepInfo->value; return VK_SUCCESS; };
    PFN_vkGetLatencyTimingsNV vkGetLatencyTimingsNV = [](VkDevice, VkSwapchainKHR, VkGetLatencyMarkerInfoNV* getLatencyMarkerInfo) { getLatencyMarkerInfo->pTimings[0].presentID = 42; };
    PFN_vkSetLatencyMarkerNV vkSetLatencyMarkerNV = [](VkDevice, VkSwapchainKHR, const VkSetLatencyMarkerInfoNV*) {};
    PFN_vkQueueNotifyOutOfBandNV vkQueueNotifyOutOfBandNV = [](VkQueue queue, const VkOutOfBandQueueTypeInfoNV* outOfBandQueueTypeInfo) { *reinterpret_cast<VkOutOfBandQueueTypeNV*>(queue) = outOfBandQueueTypeInfo->queueType; };

    auto dxgiFactory = std::make_unique<DXGIDxvkFactoryMock>();
    auto vk = std::make_unique<VkMock>();
    auto nvml = std::make_unique<NvmlMock>();
    auto lfx = std::make_unique<LfxMock>();
    auto vulkan = &*vk;

    ALLOW_CALL(*vulkan, IsAvailable()).RETURN(true);
    ALLOW_CALL(*vulkan, GetDeviceProcAddr(_, _)).RETURN(nullptr);

    SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml), std::move(lfx));

    dxvk::NvapiVulkanLowLatencyDevice::Reset();

    REQUIRE(dxvk::NvapiVulkanLowLatencyDevice::Initialize(*resourceFactory));

    SECTION("Low latency device creation fails when Vulkan device does not support semaphore commands") {
        auto vkDevice = reinterpret_cast<VkDevice>(__LINE__);

        auto [device, result] = dxvk::NvapiVulkanLowLatencyDevice::GetOrCreate(vkDevice);

        REQUIRE(device == nullptr);
        REQUIRE(result == VK_ERROR_INCOMPATIBLE_DRIVER);
    }

#define ALLOW_GET_DEVICE_PROC_ADDR_CALL(proc) ALLOW_CALL(*vulkan, GetDeviceProcAddr(_, eq(std::string_view(#proc)))).RETURN(reinterpret_cast<PFN_vkVoidFunction>(proc))

    ALLOW_GET_DEVICE_PROC_ADDR_CALL(vkCreateSemaphore);
    ALLOW_GET_DEVICE_PROC_ADDR_CALL(vkDestroySemaphore);

    SECTION("Low latency device creation fails when Vulkan device does not support VK_NV_low_latency2 commands") {
        auto vkDevice = reinterpret_cast<VkDevice>(__LINE__);

        auto [device, result] = dxvk::NvapiVulkanLowLatencyDevice::GetOrCreate(vkDevice);

        REQUIRE(device == nullptr);
        REQUIRE(result == VK_ERROR_EXTENSION_NOT_PRESENT);
    }

    ALLOW_GET_DEVICE_PROC_ADDR_CALL(vkSetLatencySleepModeNV);
    ALLOW_GET_DEVICE_PROC_ADDR_CALL(vkLatencySleepNV);
    ALLOW_GET_DEVICE_PROC_ADDR_CALL(vkGetLatencyTimingsNV);
    ALLOW_GET_DEVICE_PROC_ADDR_CALL(vkSetLatencyMarkerNV);
    ALLOW_GET_DEVICE_PROC_ADDR_CALL(vkQueueNotifyOutOfBandNV);

    SECTION("Low latency device creation fails when Vulkan device support required commands but semaphore creation fails") {
        auto vkDevice = reinterpret_cast<VkDevice>(__LINE__);

        auto [device, result] = dxvk::NvapiVulkanLowLatencyDevice::GetOrCreate(vkDevice);

        REQUIRE(device == nullptr);
        REQUIRE(result == VK_ERROR_OUT_OF_HOST_MEMORY);
    }

    vkCreateSemaphore = [](VkDevice, const VkSemaphoreCreateInfo*, const VkAllocationCallbacks*, VkSemaphore* semaphore) { return (*semaphore = reinterpret_cast<VkSemaphore>(new uint64_t[1]{0})) ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY; };

    ALLOW_GET_DEVICE_PROC_ADDR_CALL(vkCreateSemaphore);

    SECTION("Low latency device creation succeeds when Vulkan device support required commands and semaphore creation succeeds") {
        auto vkDevice = reinterpret_cast<VkDevice>(__LINE__);

        auto [device, result] = dxvk::NvapiVulkanLowLatencyDevice::GetOrCreate(vkDevice);

        REQUIRE(device);
        REQUIRE(result == VK_SUCCESS);
        CHECK(dxvk::NvapiVulkanLowLatencyDevice::Get(vkDevice) == device);

        auto semaphore = reinterpret_cast<uint64_t*>(device->GetSemaphore());

        REQUIRE(semaphore);

        SECTION("Low latency device remembers set latency mode") {
            CHECK_FALSE(device->GetLowLatencyMode());
            CHECK(device->SetLatencySleepMode(true, false, 5555) == VK_SUCCESS);
            CHECK(device->GetLowLatencyMode());
            CHECK(device->SetLatencySleepMode(false, false, 0) == VK_SUCCESS);
            CHECK_FALSE(device->GetLowLatencyMode());
            CHECK(device->SetLatencySleepMode(true, true, 5555) == VK_SUCCESS);
            CHECK(device->GetLowLatencyMode());
            CHECK(device->SetLatencySleepMode(nullptr) == VK_SUCCESS);
            CHECK_FALSE(device->GetLowLatencyMode());
        }

        SECTION("Low latency device performs latency sleep") {
            REQUIRE(device->LatencySleep(42) == VK_SUCCESS);
            REQUIRE(*semaphore == 42);
        }

        SECTION("Low latency device retrieves latency timings") {
            std::array<VkLatencyTimingsFrameReportNV, 64> reports;
            REQUIRE(device->GetLatencyTimings(reports));
            REQUIRE(reports[0].presentID == 42);
        }

        SECTION("Low latency device notifies queue out of band") {
            VkOutOfBandQueueTypeNV type = VK_OUT_OF_BAND_QUEUE_TYPE_RENDER_NV;
            device->QueueNotifyOutOfBand(reinterpret_cast<VkQueue>(&type), VK_OUT_OF_BAND_QUEUE_TYPE_PRESENT_NV);
            REQUIRE(type == VK_OUT_OF_BAND_QUEUE_TYPE_PRESENT_NV);
        }

        REQUIRE(dxvk::NvapiVulkanLowLatencyDevice::Destroy(vkDevice));
    }

    dxvk::NvapiVulkanLowLatencyDevice::Reset();
}
