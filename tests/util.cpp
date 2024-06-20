#include "nvapi_tests_private.h"
#include "../src/util/util_log.h"
#include "../src/util/util_string.h"
#include "../src/util/util_version.h"

using namespace Catch::Matchers;

TEST_CASE("Log", "[.util]") {
    REQUIRE(dxvk::log::fmt::hnd(nullptr) == std::string("hnd=0x0"));
    // Avoid different hex padding between MSVC and GCC
    REQUIRE(dxvk::log::fmt::hnd((void*)0x1000000000000089) == std::string("hnd=0x1000000000000089"));
    REQUIRE(dxvk::log::fmt::ptr(nullptr) == std::string("nullptr"));
    REQUIRE(dxvk::log::fmt::ptr((void*)0x1000000000000089) == std::string("ptr=0x1000000000000089"));
    REQUIRE(dxvk::log::fmt::flt(0) == std::string("0.0"));
    REQUIRE(dxvk::log::fmt::flt(1) == std::string("1.0"));
    REQUIRE(dxvk::log::fmt::flt(0.45) == std::string("0.45"));
    REQUIRE(dxvk::log::fmt::flags(28) == std::string("flags=0x001c"));

    NV_LATENCY_MARKER_PARAMS params1{};
    params1.version = NV_LATENCY_MARKER_PARAMS_VER1;
    params1.frameID = 65;
    params1.markerType = PC_LATENCY_PING;
    REQUIRE(dxvk::log::fmt::nv_latency_marker_params(&params1) == std::string("{version=65624,frameID=65,markerType=PC_LATENCY_PING,rsvd}"));

    NV_ASYNC_FRAME_MARKER_PARAMS params2{};
    params2.version = NV_ASYNC_FRAME_MARKER_PARAMS_VER1;
    params2.frameID = 65;
    params2.markerType = PC_LATENCY_PING;
    params2.presentFrameID = 63;
    REQUIRE(dxvk::log::fmt::nv_async_frame_marker_params(&params2) == std::string("{version=65624,frameID=65,markerType=PC_LATENCY_PING,presentFrameID=63,rsvd}"));

    D3D12_CPU_DESCRIPTOR_HANDLE handle{};
    auto ptr = dxvk::str::format(std::hex, handle.ptr);
    REQUIRE(dxvk::log::fmt::d3d12_cpu_descriptor_handle(handle) == dxvk::str::format("{ptr=", dxvk::log::fmt::hex_prefix, ptr, "}"));
}

TEST_CASE("String", "[.util]") {
    SECTION("NVAPI Unicode-String") {
        NvAPI_UnicodeString us = {'U', 'n', 'i', 'c', 'o', 'd', 'e'};
        REQUIRE(dxvk::str::fromnvus(us) == std::string("Unicode"));
    }

    SECTION("NVAPI Short-String") {
        NvAPI_ShortString ss{};

        dxvk::str::tonvss(ss, std::string("Short-String"));
        REQUIRE_THAT(ss, Equals("Short-String"));

        dxvk::str::tonvss(ss, std::string("Longer-Than-Short-String-Longer-Than-Short-String-Longer-Than-Short-String"));
        REQUIRE_THAT(ss, SizeIs(64));
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
