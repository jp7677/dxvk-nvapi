#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"

#include "../inc/catch.hpp"
#include "../inc/catch2/trompeloeil.hpp"
#include "../src/nvapi_d3d12.cpp"
#include "nvapi_d3d12_mocks.cpp"

using namespace trompeloeil;

TEST_CASE("D3D12 methods succeed", "[.][d3d12]") {
    D3D12DeviceMock device;

    SECTION("IsNvShaderExtnOpCodeSupported with device returns OK") {
        bool supported = true;
        REQUIRE(NvAPI_D3D12_IsNvShaderExtnOpCodeSupported(&device, 1U, &supported) == NVAPI_OK);
        REQUIRE(supported == false);
    }
}
