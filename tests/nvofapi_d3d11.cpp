#include "nvofapi_tests_private.h"
#include "nvofapi/mock_factory.h"

using namespace trompeloeil;

TEST_CASE("CreateInstanceD3D11 returns not-available", "[.d3d11]") {
    NV_OF_D3D11_API_FUNCTION_LIST functionList{};
    REQUIRE(NvOFAPICreateInstanceD3D11(0, &functionList) == NV_OF_ERR_OF_NOT_AVAILABLE);
}
