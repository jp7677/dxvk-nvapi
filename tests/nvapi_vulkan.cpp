#include "nvapi_tests_private.h"
#include "nvapi/resource_factory_util.h"

using namespace trompeloeil;

TEST_CASE("Vulkan methods succeed", "[.vulkan]") {
    auto dxgiFactory = std::make_unique<DXGIDxvkFactoryMock>();
    auto vk = std::make_unique<VkMock>();
    auto nvml = std::make_unique<NvmlMock>();

    SECTION("InitLowLatencyDevice fails to initialize when Vulkan is not available") {
        ALLOW_CALL(*vk, IsAvailable()).RETURN(false);
        FORBID_CALL(*vk, GetDeviceProcAddr(_, _));

        SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));

        auto vkDevice = std::make_unique<VkDeviceMock>();
        VkSemaphore result;
        REQUIRE(NvAPI_Vulkan_InitLowLatencyDevice(vkDevice.get(), reinterpret_cast<HANDLE*>(&result)) == NVAPI_ERROR);
    }

    SECTION("InitLowLatencyDevice fails to initialize when Vulkan Reflex layer is not available") {
        ALLOW_CALL(*vk, IsAvailable()).RETURN(true);
        auto e = VkMock::ConfigureDefaultPFN(*vk);

        REQUIRE_CALL(*vk, GetDeviceProcAddr(_, eq(std::string_view("vkSetLatencySleepModeNV"))))
            .RETURN(nullptr);
        REQUIRE_CALL(*vk, GetDeviceProcAddr(_, eq(std::string_view("vkLatencySleepNV"))))
            .RETURN(nullptr);
        REQUIRE_CALL(*vk, GetDeviceProcAddr(_, eq(std::string_view("vkGetLatencyTimingsNV"))))
            .RETURN(nullptr);
        REQUIRE_CALL(*vk, GetDeviceProcAddr(_, eq(std::string_view("vkSetLatencyMarkerNV"))))
            .RETURN(nullptr);
        REQUIRE_CALL(*vk, GetDeviceProcAddr(_, eq(std::string_view("vkQueueNotifyOutOfBandNV"))))
            .RETURN(nullptr);

        SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));

        auto vkDevice = std::make_unique<VkDeviceMock>();
        VkSemaphore result;
        REQUIRE(NvAPI_Vulkan_InitLowLatencyDevice(vkDevice.get(), reinterpret_cast<HANDLE*>(&result)) == NVAPI_NOT_SUPPORTED);
    }

    SECTION("InitLowLatencyDevice and DestroyLowLatencyDevice returns OK") {
        ALLOW_CALL(*vk, IsAvailable()).RETURN(true);
        auto e1 = VkMock::ConfigureDefaultPFN(*vk);
        auto e2 = VkMock::ConfigureLL2PFN(*vk);

        auto vkSemaphore = reinterpret_cast<VkSemaphore>(0x12345678);
        auto vkDevice = std::make_unique<VkDeviceMock>();

        REQUIRE_CALL(*vkDevice, vkCreateSemaphore(_, _, _, _))
            .LR_SIDE_EFFECT(*_4 = vkSemaphore)
            .RETURN(VK_SUCCESS);
        REQUIRE_CALL(*vkDevice, vkDestroySemaphore(_, _, _));

        SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));

        HANDLE signalSemaphoreHandle = VK_NULL_HANDLE;
        REQUIRE(NvAPI_Vulkan_InitLowLatencyDevice(vkDevice.get(), &signalSemaphoreHandle) == NVAPI_OK);
        REQUIRE(signalSemaphoreHandle == vkSemaphore);

        REQUIRE(NvAPI_Vulkan_DestroyLowLatencyDevice(vkDevice.get()) == NVAPI_OK);
    }

    SECTION("Other entrypoints succeed") {
        ALLOW_CALL(*vk, IsAvailable()).RETURN(true);
        auto e1 = VkMock::ConfigureDefaultPFN(*vk);
        auto e2 = VkMock::ConfigureLL2PFN(*vk);

        auto vkDevice = std::make_unique<VkDeviceMock>();
        auto vkQueue = std::make_unique<VkQueueMock>();
        HANDLE signalSemaphoreHandle = VK_NULL_HANDLE;

        ALLOW_CALL(*vkDevice, vkCreateSemaphore(_, _, _, _))
            .RETURN(VK_SUCCESS);
        ALLOW_CALL(*vkDevice, vkDestroySemaphore(_, _, _));

        SECTION("SetSleepMode / GetSleepStatus returns OK") {
            sequence seq1, seq2;

            REQUIRE_CALL(*vkDevice, vkSetLatencySleepModeNV(_, _, _))
                .IN_SEQUENCE(seq1)
                .WITH(_3->lowLatencyMode == true && _3->lowLatencyBoost == true && _3->minimumIntervalUs == 4)
                .RETURN(VK_SUCCESS);
            REQUIRE_CALL(*vkDevice, vkSetLatencySleepModeNV(_, _, _))
                .IN_SEQUENCE(seq2)
                .WITH(_3->lowLatencyMode == false && _3->lowLatencyBoost == false)
                .RETURN(VK_SUCCESS);

            SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));

            REQUIRE(NvAPI_Vulkan_InitLowLatencyDevice(vkDevice.get(), &signalSemaphoreHandle) == NVAPI_OK);

            NV_VULKAN_SET_SLEEP_MODE_PARAMS setSleepModeParams;
            setSleepModeParams.version = NV_VULKAN_SET_SLEEP_MODE_PARAMS_VER1;
            setSleepModeParams.bLowLatencyMode = true;
            setSleepModeParams.bLowLatencyBoost = true;
            setSleepModeParams.minimumIntervalUs = 4;
            REQUIRE(NvAPI_Vulkan_SetSleepMode(vkDevice.get(), &setSleepModeParams) == NVAPI_OK);

            NV_VULKAN_GET_SLEEP_STATUS_PARAMS getSleepStatusParams{};
            getSleepStatusParams.version = NV_VULKAN_GET_SLEEP_STATUS_PARAMS_VER1;
            REQUIRE(NvAPI_Vulkan_GetSleepStatus(vkDevice.get(), &getSleepStatusParams) == NVAPI_OK);
            REQUIRE(getSleepStatusParams.bLowLatencyMode == true);

            setSleepModeParams.bLowLatencyMode = false;
            setSleepModeParams.bLowLatencyBoost = false;
            REQUIRE(NvAPI_Vulkan_SetSleepMode(vkDevice.get(), &setSleepModeParams) == NVAPI_OK);

            REQUIRE(NvAPI_Vulkan_GetSleepStatus(vkDevice.get(), &getSleepStatusParams) == NVAPI_OK);
            REQUIRE(getSleepStatusParams.bLowLatencyMode == false);

            REQUIRE(NvAPI_Vulkan_DestroyLowLatencyDevice(vkDevice.get()) == NVAPI_OK);
        }

        SECTION("GetSleepStatus returns previous value if `vkSetLatencySleepModeNV` fails") {
            sequence seq1, seq2;

            REQUIRE_CALL(*vkDevice, vkSetLatencySleepModeNV(_, _, _))
                .IN_SEQUENCE(seq1)
                .WITH(_3->lowLatencyMode == true && _3->lowLatencyBoost == true && _3->minimumIntervalUs == 4)
                .RETURN(VK_SUCCESS);
            REQUIRE_CALL(*vkDevice, vkSetLatencySleepModeNV(_, _, _))
                .IN_SEQUENCE(seq2)
                .WITH(_3->lowLatencyMode == false && _3->lowLatencyBoost == false)
                .RETURN(VK_ERROR_UNKNOWN);

            SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));

            REQUIRE(NvAPI_Vulkan_InitLowLatencyDevice(vkDevice.get(), &signalSemaphoreHandle) == NVAPI_OK);

            NV_VULKAN_SET_SLEEP_MODE_PARAMS setSleepModeParams;
            setSleepModeParams.version = NV_VULKAN_SET_SLEEP_MODE_PARAMS_VER1;
            setSleepModeParams.bLowLatencyMode = true;
            setSleepModeParams.bLowLatencyBoost = true;
            setSleepModeParams.minimumIntervalUs = 4;
            REQUIRE(NvAPI_Vulkan_SetSleepMode(vkDevice.get(), &setSleepModeParams) == NVAPI_OK);

            NV_VULKAN_GET_SLEEP_STATUS_PARAMS getSleepStatusParams{};
            getSleepStatusParams.version = NV_VULKAN_GET_SLEEP_STATUS_PARAMS_VER1;
            REQUIRE(NvAPI_Vulkan_GetSleepStatus(vkDevice.get(), &getSleepStatusParams) == NVAPI_OK);
            REQUIRE(getSleepStatusParams.bLowLatencyMode == true);

            setSleepModeParams.bLowLatencyMode = false;
            setSleepModeParams.bLowLatencyBoost = false;
            REQUIRE(NvAPI_Vulkan_SetSleepMode(vkDevice.get(), &setSleepModeParams) == NVAPI_ERROR);

            REQUIRE(NvAPI_Vulkan_GetSleepStatus(vkDevice.get(), &getSleepStatusParams) == NVAPI_OK);
            REQUIRE(getSleepStatusParams.bLowLatencyMode == true);

            REQUIRE(NvAPI_Vulkan_DestroyLowLatencyDevice(vkDevice.get()) == NVAPI_OK);
        }

        SECTION("GetSleepStatus with unknown struct version returns incompatible-struct-version") {
            NV_VULKAN_GET_SLEEP_STATUS_PARAMS params;
            params.version = NV_VULKAN_GET_SLEEP_STATUS_PARAMS_VER1 + 1;
            REQUIRE(NvAPI_Vulkan_GetSleepStatus(vkDevice.get(), &params) == NVAPI_INCOMPATIBLE_STRUCT_VERSION);
        }

        SECTION("GetSleepStatus with current struct version returns not incompatible-struct-version") {
            // This test should fail when a header update provides a newer not yet implemented struct version
            NV_VULKAN_GET_SLEEP_STATUS_PARAMS params;
            params.version = NV_VULKAN_GET_SLEEP_STATUS_PARAMS_VER;
            REQUIRE(NvAPI_Vulkan_GetSleepStatus(vkDevice.get(), &params) != NVAPI_INCOMPATIBLE_STRUCT_VERSION);
        }

        SECTION("SetSleepMode with unknown struct version returns incompatible-struct-version") {
            NV_VULKAN_SET_SLEEP_MODE_PARAMS params;
            params.version = NV_VULKAN_SET_SLEEP_MODE_PARAMS_VER1 + 1;
            REQUIRE(NvAPI_Vulkan_SetSleepMode(vkDevice.get(), &params) == NVAPI_INCOMPATIBLE_STRUCT_VERSION);
        }

        SECTION("SetSleepMode with current struct version returns not incompatible-struct-version") {
            // This test should fail when a header update provides a newer not yet implemented struct version
            NV_VULKAN_SET_SLEEP_MODE_PARAMS params;
            params.version = NV_VULKAN_SET_SLEEP_MODE_PARAMS_VER;
            REQUIRE(NvAPI_Vulkan_SetSleepMode(vkDevice.get(), &params) != NVAPI_INCOMPATIBLE_STRUCT_VERSION);
        }

        SECTION("Sleep returns OK") {
            auto signal = 1365ULL;

            REQUIRE_CALL(*vkDevice, vkLatencySleepNV(_, _, _))
                .WITH(_3->value == signal)
                .RETURN(VK_SUCCESS);

            SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));

            REQUIRE(NvAPI_Vulkan_InitLowLatencyDevice(vkDevice.get(), &signalSemaphoreHandle) == NVAPI_OK);
            REQUIRE(NvAPI_Vulkan_Sleep(vkDevice.get(), signal) == NVAPI_OK);
            REQUIRE(NvAPI_Vulkan_DestroyLowLatencyDevice(vkDevice.get()) == NVAPI_OK);
        }

        SECTION("GetLatency returns OK") {
            REQUIRE_CALL(*vkDevice, vkGetLatencyTimingsNV(_, _, _))
                .SIDE_EFFECT({
                    _3->timingCount = 64;
                    _3->pTimings[0].presentID = 5;
                    _3->pTimings[0].inputSampleTimeUs = 8;
                    _3->pTimings[1].presentID = 6;
                    _3->pTimings[1].inputSampleTimeUs = 7;
                });

            SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));

            REQUIRE(NvAPI_Vulkan_InitLowLatencyDevice(vkDevice.get(), &signalSemaphoreHandle) == NVAPI_OK);

            NV_VULKAN_LATENCY_RESULT_PARAMS params{};
            params.version = NV_VULKAN_LATENCY_RESULT_PARAMS_VER1;
            REQUIRE(NvAPI_Vulkan_GetLatency(vkDevice.get(), &params) == NVAPI_OK);
            REQUIRE(params.frameReport[0].frameID == 5);
            REQUIRE(params.frameReport[0].inputSampleTime == 8);
            REQUIRE(params.frameReport[1].frameID == 6);
            REQUIRE(params.frameReport[1].inputSampleTime == 7);

            REQUIRE(NvAPI_Vulkan_DestroyLowLatencyDevice(vkDevice.get()) == NVAPI_OK);
        }

        SECTION("GetLatency with other than 64 reports clears timings returns OK") {
            REQUIRE_CALL(*vkDevice, vkGetLatencyTimingsNV(_, _, _))
                .SIDE_EFFECT({
                    _3->timingCount = 1;
                    _3->pTimings[0].presentID = 5;
                });

            SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));

            REQUIRE(NvAPI_Vulkan_InitLowLatencyDevice(vkDevice.get(), &signalSemaphoreHandle) == NVAPI_OK);

            NV_VULKAN_LATENCY_RESULT_PARAMS params{};
            params.version = NV_VULKAN_LATENCY_RESULT_PARAMS_VER1;
            for (auto i = 0U; i < 64; i++)
                params.frameReport[i].frameID = std::numeric_limits<uint32_t>::max();
            REQUIRE(NvAPI_Vulkan_GetLatency(vkDevice.get(), &params) == NVAPI_OK);
            for (auto i = 0U; i < 64; i++)
                REQUIRE(params.frameReport[i].frameID == 0);

            REQUIRE(NvAPI_Vulkan_DestroyLowLatencyDevice(vkDevice.get()) == NVAPI_OK);
        }

        SECTION("GetLatency with unknown struct version returns incompatible-struct-version") {
            NV_VULKAN_LATENCY_RESULT_PARAMS params;
            params.version = NV_VULKAN_LATENCY_RESULT_PARAMS_VER1 + 1;
            REQUIRE(NvAPI_Vulkan_GetLatency(vkDevice.get(), &params) == NVAPI_INCOMPATIBLE_STRUCT_VERSION);
        }

        SECTION("GetLatency with current struct version returns not incompatible-struct-version") {
            // This test should fail when a header update provides a newer not yet implemented struct version
            NV_VULKAN_LATENCY_RESULT_PARAMS params;
            params.version = NV_VULKAN_LATENCY_RESULT_PARAMS_VER;
            REQUIRE(NvAPI_Vulkan_GetLatency(vkDevice.get(), &params) != NVAPI_INCOMPATIBLE_STRUCT_VERSION);
        }

        SECTION("SetLatencyMarker returns OK") {
            REQUIRE_CALL(*vkDevice, vkSetLatencyMarkerNV(_, _, _))
                .WITH(_3->marker == VK_LATENCY_MARKER_PRESENT_START_NV);

            SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));

            REQUIRE(NvAPI_Vulkan_InitLowLatencyDevice(vkDevice.get(), &signalSemaphoreHandle) == NVAPI_OK);

            NV_VULKAN_LATENCY_MARKER_PARAMS params;
            params.version = NV_VULKAN_LATENCY_MARKER_PARAMS_VER1;
            params.markerType = VULKAN_PRESENT_START;
            REQUIRE(NvAPI_Vulkan_SetLatencyMarker(vkDevice.get(), &params) == NVAPI_OK);

            REQUIRE(NvAPI_Vulkan_DestroyLowLatencyDevice(vkDevice.get()) == NVAPI_OK);
        }

        SECTION("SetLatencyMarker drops unknown marker types and returns OK") {
            FORBID_CALL(*vkDevice, vkSetLatencyMarkerNV(_, _, _));

            SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));

            REQUIRE(NvAPI_Vulkan_InitLowLatencyDevice(vkDevice.get(), &signalSemaphoreHandle) == NVAPI_OK);

            NV_VULKAN_LATENCY_MARKER_PARAMS params;
            params.version = NV_VULKAN_LATENCY_MARKER_PARAMS_VER1;
            params.markerType = static_cast<NV_VULKAN_LATENCY_MARKER_TYPE>(15);
            REQUIRE(NvAPI_Vulkan_SetLatencyMarker(vkDevice.get(), &params) == NVAPI_OK);

            REQUIRE(NvAPI_Vulkan_DestroyLowLatencyDevice(vkDevice.get()) == NVAPI_OK);
        }

        SECTION("SetLatencyMarker with unknown struct version returns incompatible-struct-version") {
            NV_VULKAN_LATENCY_MARKER_PARAMS params;
            params.version = NV_VULKAN_LATENCY_MARKER_PARAMS_VER1 + 1;
            REQUIRE(NvAPI_Vulkan_SetLatencyMarker(vkDevice.get(), &params) == NVAPI_INCOMPATIBLE_STRUCT_VERSION);
        }

        SECTION("SetLatencyMarker with current struct version returns not incompatible-struct-version") {
            // This test should fail when a header update provides a newer not yet implemented struct version
            NV_VULKAN_LATENCY_MARKER_PARAMS params;
            params.version = NV_VULKAN_LATENCY_MARKER_PARAMS_VER;
            REQUIRE(NvAPI_Vulkan_SetLatencyMarker(vkDevice.get(), &params) != NVAPI_INCOMPATIBLE_STRUCT_VERSION);
        }

        SECTION("NotifyOutOfBandVkQueue returns OK") {
            REQUIRE_CALL(*vkQueue, vkQueueNotifyOutOfBandNV(_, _))
                .WITH(_2->queueType == VK_OUT_OF_BAND_QUEUE_TYPE_PRESENT_NV);

            SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));

            REQUIRE(NvAPI_Vulkan_InitLowLatencyDevice(vkDevice.get(), &signalSemaphoreHandle) == NVAPI_OK);
            REQUIRE(NvAPI_Vulkan_NotifyOutOfBandVkQueue(vkDevice.get(), vkQueue.get(), VULKAN_OUT_OF_BAND_QUEUE_TYPE_PRESENT) == NVAPI_OK);
            REQUIRE(NvAPI_Vulkan_DestroyLowLatencyDevice(vkDevice.get()) == NVAPI_OK);
        }
    }
}
