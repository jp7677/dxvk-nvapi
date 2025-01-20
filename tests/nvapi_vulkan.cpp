#include "nvapi_tests_private.h"
#include "nvapi_vulkan_mocks.h"
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

TEST_CASE("Vulkan methods succeed", "[.vulkan]") {
    auto dxgiFactory = std::make_unique<DXGIDxvkFactoryMock>();
    auto vk = std::make_unique<VkMock>();
    auto nvml = std::make_unique<NvmlMock>();
    auto lfx = std::make_unique<LfxMock>();

    ALLOW_CALL(*vk, IsAvailable()).RETURN(true);

    SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml), std::move(lfx));

    dxvk::NvapiVulkanLowLatencyDevice::Reset();

    REQUIRE(dxvk::NvapiVulkanLowLatencyDevice::Initialize(*resourceFactory));

    auto vkDevice = reinterpret_cast<VkDevice>(__LINE__);

    auto [device, inserted] = dxvk::NvapiVulkanLowLatencyDevice::Insert(std::make_unique<NvapiVulkanLowLatencyDeviceMock>(vkDevice));

    REQUIRE(inserted);

    auto deviceMock = reinterpret_cast<NvapiVulkanLowLatencyDeviceMock*>(device);

    SECTION("InitLowLatencyDevice returns a timeline semaphore") {
        auto semaphore = reinterpret_cast<VkSemaphore>(__LINE__);
        ALLOW_CALL(*deviceMock, GetSemaphore()).RETURN(semaphore);
        VkSemaphore result;
        REQUIRE(NvAPI_Vulkan_InitLowLatencyDevice(vkDevice, reinterpret_cast<HANDLE*>(&result)) == NVAPI_OK);
        REQUIRE(result == semaphore);
    }

    SECTION("DestroyLowLatencyDevice destroys the device") {
        REQUIRE(NvAPI_Vulkan_DestroyLowLatencyDevice(vkDevice) == NVAPI_OK);
        REQUIRE_FALSE(dxvk::NvapiVulkanLowLatencyDevice::Get(vkDevice));
        REQUIRE(NvAPI_Vulkan_DestroyLowLatencyDevice(vkDevice) == NVAPI_HANDLE_INVALIDATED);
    }

    SECTION("GetSleepStatus returns low latency mode") {
        REQUIRE(NvAPI_Vulkan_GetSleepStatus(vkDevice, nullptr) == NVAPI_INVALID_POINTER);
        NV_VULKAN_GET_SLEEP_STATUS_PARAMS_V1 params{};
        REQUIRE(NvAPI_Vulkan_GetSleepStatus(vkDevice, &params) == NVAPI_INCOMPATIBLE_STRUCT_VERSION);
        params.version = NV_VULKAN_GET_SLEEP_STATUS_PARAMS_VER1;
        REQUIRE(NvAPI_Vulkan_GetSleepStatus(nullptr, &params) == NVAPI_INVALID_ARGUMENT);
        REQUIRE(NvAPI_Vulkan_GetSleepStatus(reinterpret_cast<VkDevice>(__LINE__), &params) == NVAPI_HANDLE_INVALIDATED);
        REQUIRE_CALL(*deviceMock, GetLowLatencyMode()).RETURN(true);
        REQUIRE(NvAPI_Vulkan_GetSleepStatus(vkDevice, &params) == NVAPI_OK);
        REQUIRE(params.bLowLatencyMode);
    }

    SECTION("SetSleepMode sets latency sleep mode") {
        NV_VULKAN_SET_SLEEP_MODE_PARAMS_V1 params{};
        REQUIRE(NvAPI_Vulkan_SetSleepMode(vkDevice, &params) == NVAPI_INCOMPATIBLE_STRUCT_VERSION);
        params.version = NV_VULKAN_SET_SLEEP_MODE_PARAMS_VER1;
        params.bLowLatencyMode = true;
        params.bLowLatencyBoost = true;
        params.minimumIntervalUs = 5555;
        REQUIRE(NvAPI_Vulkan_SetSleepMode(nullptr, &params) == NVAPI_INVALID_ARGUMENT);
        REQUIRE(NvAPI_Vulkan_SetSleepMode(reinterpret_cast<VkDevice>(__LINE__), &params) == NVAPI_HANDLE_INVALIDATED);
        REQUIRE_CALL(*deviceMock, SetLatencySleepMode(params.bLowLatencyMode, params.bLowLatencyBoost, params.minimumIntervalUs)).RETURN(VK_SUCCESS);
        REQUIRE(NvAPI_Vulkan_SetSleepMode(vkDevice, &params) == NVAPI_OK);
    }

    SECTION("Sleep performs latency sleep") {
        NvU64 signalValue = 42;
        REQUIRE(NvAPI_Vulkan_Sleep(nullptr, signalValue) == NVAPI_INVALID_ARGUMENT);
        REQUIRE(NvAPI_Vulkan_Sleep(reinterpret_cast<VkDevice>(__LINE__), signalValue) == NVAPI_HANDLE_INVALIDATED);
        REQUIRE_CALL(*deviceMock, LatencySleep(signalValue)).RETURN(VK_SUCCESS);
        REQUIRE(NvAPI_Vulkan_Sleep(vkDevice, signalValue) == NVAPI_OK);
    }

    SECTION("GetLatency retrieves latency reports") {
        NV_VULKAN_LATENCY_RESULT_PARAMS_V1 params{};
        REQUIRE(NvAPI_Vulkan_GetLatency(vkDevice, &params) == NVAPI_INCOMPATIBLE_STRUCT_VERSION);
        params.version = NV_VULKAN_LATENCY_RESULT_PARAMS_VER1;
        memset(&params.frameReport[0].frameID, 0xef, sizeof(NV_VULKAN_LATENCY_RESULT_PARAMS_V1::vkFrameReport) - offsetof(NV_VULKAN_LATENCY_RESULT_PARAMS_V1::vkFrameReport, rsvd));
        REQUIRE(NvAPI_Vulkan_GetLatency(nullptr, &params) == NVAPI_INVALID_ARGUMENT);
        REQUIRE(NvAPI_Vulkan_GetLatency(reinterpret_cast<VkDevice>(__LINE__), &params) == NVAPI_HANDLE_INVALIDATED);

        REQUIRE_CALL(*deviceMock, GetLatencyTimings(_))
            .SIDE_EFFECT(std::memset(&_1[0].presentID, 0xef, sizeof(VkLatencyTimingsFrameReportNV) - offsetof(VkLatencyTimingsFrameReportNV, presentID)))
            .RETURN(false);

        REQUIRE(NvAPI_Vulkan_GetLatency(vkDevice, &params) == NVAPI_OK);

        for (auto& report : params.frameReport) {
            REQUIRE(report.frameID == 0);
            REQUIRE(report.inputSampleTime == 0);
            REQUIRE(report.simStartTime == 0);
            REQUIRE(report.simEndTime == 0);
            REQUIRE(report.renderSubmitStartTime == 0);
            REQUIRE(report.renderSubmitEndTime == 0);
            REQUIRE(report.presentStartTime == 0);
            REQUIRE(report.presentEndTime == 0);
            REQUIRE(report.driverStartTime == 0);
            REQUIRE(report.driverEndTime == 0);
            REQUIRE(report.osRenderQueueStartTime == 0);
            REQUIRE(report.osRenderQueueEndTime == 0);
            REQUIRE(report.gpuRenderStartTime == 0);
            REQUIRE(report.gpuRenderEndTime == 0);
        }

        REQUIRE_CALL(*deviceMock, GetLatencyTimings(_))
            .SIDE_EFFECT(
                for (auto i = 0; i < 64; ++i)
                    std::memset(&_1[i].presentID, i, sizeof(VkLatencyTimingsFrameReportNV) - offsetof(VkLatencyTimingsFrameReportNV, presentID)))
            .RETURN(true);

        REQUIRE(NvAPI_Vulkan_GetLatency(vkDevice, &params) == NVAPI_OK);

        for (auto i = 0; i < 64; ++i) {
            auto& report = params.frameReport[i];
            NvU64 value;
            std::memset(&value, i, sizeof(value));
            REQUIRE(report.frameID == value);
            REQUIRE(report.inputSampleTime == value);
            REQUIRE(report.simStartTime == value);
            REQUIRE(report.simEndTime == value);
            REQUIRE(report.renderSubmitStartTime == value);
            REQUIRE(report.renderSubmitEndTime == value);
            REQUIRE(report.presentStartTime == value);
            REQUIRE(report.presentEndTime == value);
            REQUIRE(report.driverStartTime == value);
            REQUIRE(report.driverEndTime == value);
            REQUIRE(report.osRenderQueueStartTime == value);
            REQUIRE(report.osRenderQueueEndTime == value);
            REQUIRE(report.gpuRenderStartTime == value);
            REQUIRE(report.gpuRenderEndTime == value);
        }
    }

    SECTION("SetLatencyMarker drops calls with invalid parameters") {
        FORBID_CALL(*deviceMock, SetLatencyMarker(_, _));
        NV_VULKAN_LATENCY_MARKER_PARAMS_V1 params{};
        CHECK(NvAPI_Vulkan_SetLatencyMarker(vkDevice, &params) == NVAPI_INCOMPATIBLE_STRUCT_VERSION);
        params.version = NV_VULKAN_LATENCY_MARKER_PARAMS_VER1;
        params.frameID = 42;
        params.markerType = static_cast<NV_VULKAN_LATENCY_MARKER_TYPE>(42);
        CHECK(NvAPI_Vulkan_SetLatencyMarker(nullptr, &params) == NVAPI_INVALID_ARGUMENT);
        CHECK(NvAPI_Vulkan_SetLatencyMarker(reinterpret_cast<VkDevice>(__LINE__), &params) == NVAPI_HANDLE_INVALIDATED);
        REQUIRE(NvAPI_Vulkan_SetLatencyMarker(vkDevice, &params) == NVAPI_OK);
    }

    SECTION("SetLatencyMarker translates NVAPI latency marker types to Vulkan latency markers") {
        auto [nvapiLatencyMarkerType, vulkanLatencyMarker] = GENERATE(
            std::make_pair(VULKAN_SIMULATION_START, VK_LATENCY_MARKER_SIMULATION_START_NV),
            std::make_pair(VULKAN_SIMULATION_END, VK_LATENCY_MARKER_SIMULATION_END_NV),
            std::make_pair(VULKAN_RENDERSUBMIT_START, VK_LATENCY_MARKER_RENDERSUBMIT_START_NV),
            std::make_pair(VULKAN_RENDERSUBMIT_END, VK_LATENCY_MARKER_RENDERSUBMIT_END_NV),
            std::make_pair(VULKAN_PRESENT_START, VK_LATENCY_MARKER_PRESENT_START_NV),
            std::make_pair(VULKAN_PRESENT_END, VK_LATENCY_MARKER_PRESENT_END_NV),
            std::make_pair(VULKAN_INPUT_SAMPLE, VK_LATENCY_MARKER_INPUT_SAMPLE_NV),
            std::make_pair(VULKAN_TRIGGER_FLASH, VK_LATENCY_MARKER_TRIGGER_FLASH_NV),
            std::make_pair(VULKAN_OUT_OF_BAND_RENDERSUBMIT_START, VK_LATENCY_MARKER_OUT_OF_BAND_RENDERSUBMIT_START_NV),
            std::make_pair(VULKAN_OUT_OF_BAND_RENDERSUBMIT_END, VK_LATENCY_MARKER_OUT_OF_BAND_RENDERSUBMIT_END_NV),
            std::make_pair(VULKAN_OUT_OF_BAND_PRESENT_START, VK_LATENCY_MARKER_OUT_OF_BAND_PRESENT_START_NV),
            std::make_pair(VULKAN_OUT_OF_BAND_PRESENT_END, VK_LATENCY_MARKER_OUT_OF_BAND_PRESENT_END_NV));

        NV_VULKAN_LATENCY_MARKER_PARAMS_V1 params{};
        params.version = NV_VULKAN_LATENCY_MARKER_PARAMS_VER1;
        params.frameID = 42;
        params.markerType = nvapiLatencyMarkerType;
        REQUIRE_CALL(*deviceMock, SetLatencyMarker(params.frameID, vulkanLatencyMarker));
        REQUIRE(NvAPI_Vulkan_SetLatencyMarker(vkDevice, &params) == NVAPI_OK);
    }

    SECTION("NotifyOutOfBandVkQueue notifies queue out of band") {
        auto [nvapiQueueType, vulkanQueueType] = GENERATE(
            std::make_pair(VULKAN_OUT_OF_BAND_QUEUE_TYPE_RENDER, VK_OUT_OF_BAND_QUEUE_TYPE_RENDER_NV),
            std::make_pair(VULKAN_OUT_OF_BAND_QUEUE_TYPE_PRESENT, VK_OUT_OF_BAND_QUEUE_TYPE_PRESENT_NV));

        auto queue = reinterpret_cast<VkQueue>(__LINE__);
        CHECK(NvAPI_Vulkan_NotifyOutOfBandVkQueue(vkDevice, nullptr, nvapiQueueType) == NVAPI_INVALID_ARGUMENT);
        CHECK(NvAPI_Vulkan_NotifyOutOfBandVkQueue(nullptr, queue, nvapiQueueType) == NVAPI_INVALID_ARGUMENT);
        CHECK(NvAPI_Vulkan_NotifyOutOfBandVkQueue(reinterpret_cast<VkDevice>(__LINE__), queue, nvapiQueueType) == NVAPI_HANDLE_INVALIDATED);
        REQUIRE_CALL(*deviceMock, QueueNotifyOutOfBand(queue, vulkanQueueType));
        REQUIRE(NvAPI_Vulkan_NotifyOutOfBandVkQueue(vkDevice, queue, nvapiQueueType) == NVAPI_OK);
    }

    dxvk::NvapiVulkanLowLatencyDevice::Reset();
}
