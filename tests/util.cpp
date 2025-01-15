#include "nvapi_tests_private.h"
#include "../src/util/util_log.h"
#include "../src/util/util_string.h"
#include "../src/util/util_version.h"

using namespace Catch::Matchers;

TEST_CASE("Log", "[.util]") {
    REQUIRE(dxvk::log::fmt::hnd(nullptr) == "hnd=0x0");
    // Avoid different hex padding between MSVC and GCC
    REQUIRE(dxvk::log::fmt::hnd((void*)0x1000000000000089) == "hnd=0x1000000000000089");
    REQUIRE(dxvk::log::fmt::ptr(nullptr) == "nullptr");
    REQUIRE(dxvk::log::fmt::ptr((void*)0x1000000000000089) == "ptr=0x1000000000000089");
    REQUIRE(dxvk::log::fmt::flt(0) == "0.0");
    REQUIRE(dxvk::log::fmt::flt(1) == "1.0");
    REQUIRE(dxvk::log::fmt::flt(0.45) == "0.45");
    REQUIRE(dxvk::log::fmt::flags(28) == "flags=0x001c");

    {
        NV_LATENCY_MARKER_PARAMS params{};
        params.version = NV_LATENCY_MARKER_PARAMS_VER1;
        params.frameID = 65;
        params.markerType = PC_LATENCY_PING;
        REQUIRE(dxvk::log::fmt::nv_latency_marker_params(&params) == "{version=65624,frameID=65,markerType=PC_LATENCY_PING,rsvd}");
    }

    {
        NV_ASYNC_FRAME_MARKER_PARAMS params{};
        params.version = NV_ASYNC_FRAME_MARKER_PARAMS_VER1;
        params.frameID = 65;
        params.markerType = PC_LATENCY_PING;
        params.presentFrameID = 63;
        REQUIRE(dxvk::log::fmt::nv_async_frame_marker_params(&params) == "{version=65624,frameID=65,markerType=PC_LATENCY_PING,presentFrameID=63,rsvd}");
    }

    {
        NV_VULKAN_GET_SLEEP_STATUS_PARAMS params{};
        params.version = NV_VULKAN_GET_SLEEP_STATUS_PARAMS_VER1;
        REQUIRE(dxvk::log::fmt::nv_vk_get_sleep_status_params(&params) == "{version=65672,...,rsvd}");
    }

    {
        NV_VULKAN_SET_SLEEP_MODE_PARAMS params{};
        params.version = NV_VULKAN_SET_SLEEP_MODE_PARAMS_VER1;
        params.bLowLatencyMode = 1;
        params.bLowLatencyBoost = 0;
        params.minimumIntervalUs = 5555;
        REQUIRE(dxvk::log::fmt::nv_vk_set_sleep_status_params(&params) == "{version=65580,bLowLatencyMode=1,bLowLatencyBoost=0,minimumIntervalUs=5555,rsvd}");
    }

    {
        NV_VULKAN_LATENCY_RESULT_PARAMS params{};
        params.version = NV_VULKAN_LATENCY_RESULT_PARAMS_VER1;
        REQUIRE(dxvk::log::fmt::nv_vk_latency_result_params(&params) == "{version=80936,...,rsvd}");
    }

    {
        NV_VULKAN_LATENCY_MARKER_PARAMS params{};
        params.version = NV_VULKAN_LATENCY_MARKER_PARAMS_VER1;
        params.frameID = 65;
        params.markerType = VULKAN_PC_LATENCY_PING;
        REQUIRE(dxvk::log::fmt::nv_vk_latency_marker_params(&params) == "{version=65624,frameID=65,markerType=8,rsvd}");
    }

    {
        D3D12_CPU_DESCRIPTOR_HANDLE handle{};
        auto ptr = dxvk::str::format(std::hex, handle.ptr);
        REQUIRE(dxvk::log::fmt::d3d12_cpu_descriptor_handle(handle) == dxvk::str::format("{ptr=", dxvk::log::fmt::hex_prefix, ptr, "}"));
    }
}

TEST_CASE("String", "[.util]") {
    SECTION("fromnvus") {
        NvAPI_UnicodeString us = {'U', 'n', 'i', 'c', 'o', 'd', 'e'};
        REQUIRE(dxvk::str::fromnvus(us) == "Unicode");
    }

    SECTION("tonvss") {
        NvAPI_ShortString ss{};

        dxvk::str::tonvss(ss, "Short-String");
        REQUIRE_THAT(ss, Equals("Short-String"));

        dxvk::str::tonvss(ss, "Longer-Than-Short-String-Longer-Than-Short-String-Longer-Than-Short-String");
        REQUIRE_THAT(ss, SizeIs(64));
    }

    SECTION("fromnullable") {
        REQUIRE(dxvk::str::fromnullable(nullptr) == std::string());
        REQUIRE(dxvk::str::fromnullable("") == std::string());
        REQUIRE(dxvk::str::fromnullable("string") == "string");
    }

    SECTION("split") {
        REQUIRE(dxvk::str::split<std::set<std::string>>("", std::regex(",")).size() == 0);

        auto result = dxvk::str::split<std::set<std::string>>("foo,bar,baz", std::regex(","));

        REQUIRE(result.size() == 3);
        REQUIRE(result.contains("foo"));
        REQUIRE(result.contains("bar"));
        REQUIRE(result.contains("baz"));
    }

    SECTION("CaseInsensitiveCompare") {
        dxvk::str::CaseInsensitiveCompare<std::string_view> compare;

        struct Data {
            const char* lhs;
            const char* rhs;
            bool result;
        };

        auto args = GENERATE(
            Data{"aa", "aa", false},
            Data{"ab", "aa", false},
            Data{"aa", "ab", true},
            Data{"AA", "aa", false},
            Data{"AB", "aa", false},
            Data{"AA", "ab", true},
            Data{"aa", "AA", false},
            Data{"ab", "AA", false},
            Data{"aa", "AB", true});

        REQUIRE(compare(args.lhs, args.rhs) == args.result);

        std::set<std::string_view, dxvk::str::CaseInsensitiveCompare<std::string_view>> set{"NvAPI_Initialize"};

        REQUIRE(set.contains("NvAPI_Initialize"));
        REQUIRE(set.contains("nvapi_initialize"));
        REQUIRE(set.contains("NVAPI_INITIALIZE"));
    }

    SECTION("implode") {
        REQUIRE(dxvk::str::implode(",", std::vector<std::string_view>{}) == std::string());
        REQUIRE(dxvk::str::implode(",", std::vector<std::string_view>{"foo"}) == "foo");
        REQUIRE(dxvk::str::implode(" ", std::vector<std::string_view>{"foo", "bar"}) == "foo bar");
        REQUIRE(dxvk::str::implode(", ", std::vector<std::string_view>{"foo", "bar", "baz"}) == "foo, bar, baz");
    }
}

TEST_CASE("Version", "[.util]") {
    SECTION("Vulkan version packing") {
        struct Data {
            uint32_t driverVersion;
            uint32_t major;
            uint32_t minor;
            uint32_t patch;
        };
        auto args = GENERATE(
            Data{0x0, 0x0, 0x0, 0x0},
            Data{0xffffffff, 0x3ff, 0xff, 0xff},
            Data{0x85eac100, 535, 171, 04});

        REQUIRE(dxvk::nvVersionMajor(args.driverVersion) == args.major);
        REQUIRE(dxvk::nvVersionMinor(args.driverVersion) == args.minor);
        REQUIRE(dxvk::nvVersionPatch(args.driverVersion) == args.patch);
    }

    SECTION("Make Vulkan version") {
        REQUIRE(dxvk::nvMakeVersion(0xffff, 0xffff, 0xffff) == 0xffffffc0);
    }
}
