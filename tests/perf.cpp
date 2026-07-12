#include "nvapi_tests_private.h"
#include "../src/util/util_string.h"

using namespace Catch::Matchers;

TEST_CASE("Caching and looking simulations", "[.benchmarks]") {
    constexpr auto element_count = 25;

    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(1, element_count - 1);

    auto values = std::vector<std::string>();
    for (auto i = 0U; i < element_count; i++)
        values.emplace_back(dxvk::str::format("Some string representing an instance to something else - ", i));

    std::unordered_map<uint64_t*, std::string> umap;
    std::map<uint64_t*, std::string> map;
    std::vector<std::pair<uint64_t*, std::string>> vec;
    static std::mutex m_mutex;
    static std::shared_mutex m_smutex;

    for (auto& s : values) {
        umap.emplace(reinterpret_cast<uint64_t*>(&s), s);
        map.emplace(reinterpret_cast<uint64_t*>(&s), s);
        vec.emplace_back(reinterpret_cast<uint64_t*>(&s), s);
    }

    REQUIRE(values.size() == element_count);
    REQUIRE(umap.size() == element_count);
    REQUIRE(map.size() == element_count);
    REQUIRE(vec.size() == element_count);

    BENCHMARK("unordered_map") {
        auto& k = values[dist(rng)];
        if (const auto it = umap.find(reinterpret_cast<uint64_t*>(&k)); it != umap.end())
            return it->second;

        throw std::runtime_error("not found");
    };

    BENCHMARK("scoped_lock") {
        std::scoped_lock lock{m_mutex};
        auto& k = values[dist(rng)];
        if (const auto it = umap.find(reinterpret_cast<uint64_t*>(&k)); it != umap.end())
            return it->second;

        throw std::runtime_error("not found");
    };

    BENCHMARK("shared_lock") {
        std::shared_lock lock{m_smutex};
        auto& k = values[dist(rng)];
        if (const auto it = umap.find(reinterpret_cast<uint64_t*>(&k)); it != umap.end())
            return it->second;

        throw std::runtime_error("not found");
    };

    BENCHMARK("map") {
        auto& k = values[dist(rng)];
        if (const auto it = map.find(reinterpret_cast<uint64_t*>(&k)); it != map.end())
            return it->second;

        throw std::runtime_error("not found");
    };

    BENCHMARK("vector") {
        auto& k = values[dist(rng)];
        if (const auto it = std::find_if(vec.begin(), vec.end(), [&k](auto& item) { return item.first == reinterpret_cast<uint64_t*>(&k); }); it != vec.end())
            return it->second;

        throw std::runtime_error("not found");
    };
}

TEST_CASE("NVAPI-Vulkan benchmarks", "[.benchmarks]") {
    constexpr auto element_count = 500;

    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(1, element_count - 1);

    auto values = std::vector<NvU32>();
    for (auto i = 0U; i < element_count; i++)
        values.emplace_back(i);

    auto t = std::make_unique<DefaultTestEnvironment>();
    auto e = t->ConfigureExpectations();
    auto e1 = VkMock::ConfigureDefaultPFN(*t->Vk());
    auto e2 = VkMock::ConfigureLL2PFN(*t->Vk());

    auto vkDevice = std::make_unique<VkDeviceMock>();
    auto vkQueue = std::make_unique<VkQueueMock>();

    REQUIRE_CALL(*t->Vk(), IsAvailable()).RETURN(true).TIMES(AT_LEAST(1));
    REQUIRE_CALL(*vkDevice, vkCreateSemaphore(_, _, _, _)).RETURN(VK_SUCCESS).TIMES(AT_LEAST(1));
    REQUIRE_CALL(*vkDevice, vkDestroySemaphore(_, _, _)).TIMES(AT_LEAST(1));
    REQUIRE_CALL(*vkDevice, vkSetLatencySleepModeNV(_, _, _)).RETURN(VK_SUCCESS).TIMES(AT_LEAST(1));
    REQUIRE_CALL(*vkDevice, vkSetLatencyMarkerNV(_, _, _)).TIMES(AT_LEAST(1));
    REQUIRE_CALL(*vkDevice, vkLatencySleepNV(_, _, _)).RETURN(VK_SUCCESS).TIMES(AT_LEAST(1));
    REQUIRE_CALL(*vkQueue, vkQueueNotifyOutOfBandNV(_, _)).TIMES(AT_LEAST(1));

    HANDLE signalSemaphoreHandle = VK_NULL_HANDLE;
    REQUIRE(NvAPI_Vulkan_InitLowLatencyDevice(vkDevice.get(), &signalSemaphoreHandle) == NVAPI_OK);

    NV_VULKAN_SET_SLEEP_MODE_PARAMS setSleepModeParams{};
    setSleepModeParams.version = NV_VULKAN_SET_SLEEP_MODE_PARAMS_VER1;
    setSleepModeParams.bLowLatencyMode = true;
    setSleepModeParams.bLowLatencyBoost = true;

    NV_VULKAN_GET_SLEEP_STATUS_PARAMS getSleepStatusParams{};
    getSleepStatusParams.version = NV_VULKAN_GET_SLEEP_STATUS_PARAMS_VER1;

    NV_VULKAN_LATENCY_MARKER_PARAMS latencyMarkerParams{};
    latencyMarkerParams.version = NV_VULKAN_LATENCY_MARKER_PARAMS_VER1;
    latencyMarkerParams.markerType = VULKAN_PRESENT_START;

    BENCHMARK("Reflex") {
        setSleepModeParams.minimumIntervalUs = values[dist(rng)];
        for (auto i = 0U; i < 1000; i++) {
            REQUIRE(NvAPI_Vulkan_SetSleepMode(vkDevice.get(), &setSleepModeParams) == NVAPI_OK);
            REQUIRE(NvAPI_Vulkan_GetSleepStatus(vkDevice.get(), &getSleepStatusParams) == NVAPI_OK);
            REQUIRE(NvAPI_Vulkan_SetLatencyMarker(vkDevice.get(), &latencyMarkerParams) == NVAPI_OK);
            REQUIRE(NvAPI_Vulkan_Sleep(vkDevice.get(), values[dist(rng)]) == NVAPI_OK);
            REQUIRE(NvAPI_Vulkan_NotifyOutOfBandVkQueue(vkDevice.get(), vkQueue.get(), VULKAN_OUT_OF_BAND_QUEUE_TYPE_PRESENT) == NVAPI_OK);
            REQUIRE(NvAPI_Vulkan_SetLatencyMarker(vkDevice.get(), &latencyMarkerParams) == NVAPI_OK);
            REQUIRE(NvAPI_Vulkan_Sleep(vkDevice.get(), values[dist(rng)]) == NVAPI_OK);
            REQUIRE(NvAPI_Vulkan_NotifyOutOfBandVkQueue(vkDevice.get(), vkQueue.get(), VULKAN_OUT_OF_BAND_QUEUE_TYPE_PRESENT) == NVAPI_OK);
            REQUIRE(NvAPI_Vulkan_SetLatencyMarker(vkDevice.get(), &latencyMarkerParams) == NVAPI_OK);
            REQUIRE(NvAPI_Vulkan_Sleep(vkDevice.get(), values[dist(rng)]) == NVAPI_OK);
        }
        return values[dist(rng)];
    };

    REQUIRE(NvAPI_Vulkan_DestroyLowLatencyDevice(vkDevice.get()) == NVAPI_OK);
}
