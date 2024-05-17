#include "nvapi_tests_private.h"
#include "../src/util/util_string.h"
#include "../src/util/util_version.h"

using namespace Catch::Matchers;

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
