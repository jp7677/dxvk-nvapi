#include "nvapi_tests_private.h"
#include "../src/util/util_drs.h"
#include "../src/util/util_log.h"
#include "../src/util/util_string.h"
#include "../src/util/util_version.h"

using namespace Catch::Matchers;

TEST_CASE("DRS", "[.util]") {
    SECTION("parsedrssetting") {
        auto [str, setting] = GENERATE(
            std::make_pair<std::string_view, NvU32>("NGX_DLAA_OVERRIDE", NGX_DLAA_OVERRIDE_ID),
            std::make_pair<std::string_view, NvU32>("ngx_dlssg_multi_frame_count", NGX_DLSSG_MULTI_FRAME_COUNT_ID),
            std::make_pair<std::string_view, NvU32>("283385347", NGX_DLSS_FG_OVERRIDE_ID),
            std::make_pair<std::string_view, NvU32>("0x10BD9423", NGX_DLSS_RR_MODE_ID));

        NvU32 result;
        REQUIRE(dxvk::drs::parsedrssetting(str, result));
        REQUIRE(result == setting);
    }

    SECTION("parsedrsdwordvalue") {
        auto [setting, str, value] = GENERATE(
            std::make_tuple<NvU32, std::string_view, NvU32>(NGX_DLAA_OVERRIDE_ID, "DLAA_ON", NGX_DLAA_OVERRIDE_DLAA_ON),
            std::make_tuple<NvU32, std::string_view, NvU32>(NGX_DLSSG_MULTI_FRAME_COUNT_ID, "max", NGX_DLSSG_MULTI_FRAME_COUNT_MAX),
            std::make_tuple<NvU32, std::string_view, NvU32>(NGX_DLSS_FG_OVERRIDE_ID, "1", NGX_DLSS_FG_OVERRIDE_ON),
            std::make_tuple<NvU32, std::string_view, NvU32>(NGX_DLSS_RR_MODE_ID, "balanced", NGX_DLSS_RR_MODE_NGX_DLSS_RR_MODE_BALANCED),
            std::make_tuple<NvU32, std::string_view, NvU32>(0, "1", 1),
            std::make_tuple<NvU32, std::string_view, NvU32>(INVALID_SETTING_ID, "0x1", 1));

        NvU32 result;
        REQUIRE(dxvk::drs::parsedrsdwordvalue(setting, str, result));
        REQUIRE(result == value);
    }

    SECTION("parsedrsdwordsettings") {
        auto dwords = dxvk::drs::parsedrsdwordsettings("NGX_DLAA_OVERRIDE=DLAA_ON,ngx_dlssg_multi_frame_count=3,NGX_DLSS_FG_OVERRIDE=on,0x10E41E01=1,0x10E41DF3=render_preset_latest");

        REQUIRE(dwords.size() == 5);
        REQUIRE(dwords.at(NGX_DLAA_OVERRIDE_ID) == NGX_DLAA_OVERRIDE_DLAA_ON);
        REQUIRE(dwords.at(NGX_DLSSG_MULTI_FRAME_COUNT_ID) == 3);
        REQUIRE(dwords.at(NGX_DLSS_FG_OVERRIDE_ID) == NGX_DLSS_FG_OVERRIDE_ON);
        REQUIRE(dwords.at(0x10E41E01) == 1);
        REQUIRE(dwords.at(0x10E41DF3) == 0xffffff);
    }

    SECTION("enrichwithenv") {
        ::SetEnvironmentVariableA("TEST_NGX_DLSS_SR_OVERRIDE", "1");
        ::SetEnvironmentVariableA("TEST_NGX_DLSS_SR_OVERRIDE_RENDER_PRESET_SELECTION", "render_preset_latest");
        ::SetEnvironmentVariableA("TEST_NGX_DLSS_SR_MODE", "PERFORMANCE");
        auto dwords = dxvk::drs::enrichwithenv({{NGX_DLAA_OVERRIDE_ID, NGX_DLAA_OVERRIDE_DLAA_ON}, {NGX_DLSS_SR_OVERRIDE_ID, 0}}, "TEST_");

        REQUIRE(dwords.size() == 4);
        REQUIRE(dwords.at(NGX_DLAA_OVERRIDE_ID) == NGX_DLAA_OVERRIDE_DLAA_ON);
        REQUIRE(dwords.at(NGX_DLSS_SR_OVERRIDE_ID) == 1);
        REQUIRE(dwords.at(NGX_DLSS_SR_OVERRIDE_RENDER_PRESET_SELECTION_ID) == NGX_DLSS_SR_OVERRIDE_RENDER_PRESET_SELECTION_RENDER_PRESET_Latest);
        REQUIRE(dwords.at(NGX_DLSS_SR_MODE_ID) == NGX_DLSS_SR_MODE_NGX_DLSS_SR_MODE_PERFORMANCE);
    }
}

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

    {
        NV_NGX_DLSS_OVERRIDE_GET_STATE_PARAMS params{};
        params.version = NV_NGX_DLSS_OVERRIDE_GET_STATE_PARAMS_VER1;
        params.processIdentifier = 5;
        REQUIRE(dxvk::log::fmt::ngx_dlss_override_get_state_params(&params) == "{version=65600,processIdentifier=5}");
    }

    {
        NV_NGX_DLSS_OVERRIDE_SET_STATE_PARAMS params{};
        params.version = NV_NGX_DLSS_OVERRIDE_SET_STATE_PARAMS_VER1;
        params.processIdentifier = 5;
        params.feature = NV_NGX_DLSS_OVERRIDE_FEATURE_INDEX_FG;
        params.feedbackMask = NV_NGX_DLSS_OVERRIDE_FLAG_ENABLED | NV_NGX_DLSS_OVERRIDE_FLAG_DLL_EXISTS;
        REQUIRE(dxvk::log::fmt::ngx_dlss_override_set_state_params(&params) == "{version=65592,processIdentifier=5,feature=3,feedbackMask=0x6}");
    }
}

TEST_CASE("String", "[.util]") {
    SECTION("fromnvus") {
        NvAPI_UnicodeString us = {'U', 'n', 'i', 'c', 'o', 'd', 'e'};
        REQUIRE(dxvk::str::fromnvus(us) == "Unicode");
    }

    SECTION("copynvus") {
        NvAPI_UnicodeString us = {'U', 'n', 'i', 'c', 'o', 'd', 'e'};
        NvAPI_UnicodeString dst{};

        dxvk::str::copynvus(dst, us);
        REQUIRE(dxvk::str::fromnvus(dst) == "Unicode");
    }

    SECTION("tonvss") {
        NvAPI_ShortString ss{};

        dxvk::str::tonvss(ss, "Short-String");
        REQUIRE_THAT(ss, Equals("Short-String"));

        dxvk::str::tonvss(ss, "Longer-Than-Short-String-Longer-Than-Short-String-Longer-Than-Short-String");
        REQUIRE_THAT(ss, SizeIs(64));
    }

    SECTION("fromnullable") {
        REQUIRE(dxvk::str::fromnullable(nullptr).empty());
        REQUIRE(dxvk::str::fromnullable("").empty());
        REQUIRE(dxvk::str::fromnullable("string") == "string");
    }

    SECTION("split") {
        REQUIRE(dxvk::str::split<std::set<std::string>>("", std::regex(",")).empty());

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
        REQUIRE(dxvk::str::implode(",", std::vector<std::string_view>{}).empty());
        REQUIRE(dxvk::str::implode(",", std::vector<std::string_view>{"foo"}) == "foo");
        REQUIRE(dxvk::str::implode(" ", std::vector<std::string_view>{"foo", "bar"}) == "foo bar");
        REQUIRE(dxvk::str::implode(", ", std::vector<std::string_view>{"foo", "bar", "baz"}) == "foo, bar, baz");
    }

    SECTION("parsedword") {
        NvU32 value;

        REQUIRE_FALSE(dxvk::str::parsedword("", value));
        REQUIRE_FALSE(dxvk::str::parsedword("f", value));
        REQUIRE_FALSE(dxvk::str::parsedword("0f", value));
        REQUIRE_FALSE(dxvk::str::parsedword("f0", value));

        REQUIRE(dxvk::str::parsedword("0", value));
        REQUIRE(value == 0);
        REQUIRE(dxvk::str::parsedword("10", value));
        REQUIRE(value == 10);
        REQUIRE(dxvk::str::parsedword("0x0f", value));
        REQUIRE(value == 15);
        REQUIRE(dxvk::str::parsedword("0XFFFFFFFF", value));
        REQUIRE(value == std::numeric_limits<NvU32>::max());
    }

    SECTION("parsedwords") {
        auto dwords = dxvk::str::parsedwords(",0X10E41DF3=0x00ffffff,0x10E41DF7=0X00FFFFFF,0xf,invalid,9,=,4=,=5,,0x104D6667=3,1=2");

        REQUIRE(dwords.size() == 4);
        REQUIRE(dwords.at(0x10e41df3) == 0x00ffffff);
        REQUIRE(dwords.at(0x10e41df7) == 0x00ffffff);
        REQUIRE(dwords.at(0x104d6667) == 3);
        REQUIRE(dwords.at(1) == 2);
    }

    SECTION("parsekeydwords") {
        std::set<std::string_view, dxvk::str::CaseInsensitiveCompare<std::string_view>> keys = {"Logging", "DLSSIndicator", "DLSSGIndicator"};
        auto map = dxvk::str::parsekeydwords(",logging=0,dlssindicator=1,DLSSGIndicator=0x2,invalid,,9,=,4=,=5,,0x104D6667=3", keys);
        keys.clear();

        REQUIRE(map.size() == 3);
        REQUIRE(map.at("Logging") == 0x0);
        REQUIRE(map.at("DLSSIndicator") == 0x1);
        REQUIRE(map.at("DLSSGIndicator") == 0x2);
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
