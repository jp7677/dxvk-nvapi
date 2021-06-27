#include "../inc/catch.hpp"
#include "../inc/catch2/trompeloeil.hpp"
#include "../src/nvapi.cpp"

TEST_CASE("Initialize and unloads") {
    NvAPI_Initialize();
    NvAPI_Unload();
    REQUIRE(true);
}
