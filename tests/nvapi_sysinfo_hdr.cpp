#include "nvapi_tests_private.h"
#include "resource_factory_util.h"
#include "nvapi_sysinfo_mocks.h"

using namespace trompeloeil;
using namespace Catch::Matchers;

TEST_CASE("HDR related Sysinfo methods succeed", "[.sysinfo-hdr]") {
    auto dxgiFactory = std::make_unique<DXGIFactory1Mock>();
    auto vulkan = std::make_unique<VulkanMock>();
    auto nvml = std::make_unique<NvmlMock>();
    auto lfx = std::make_unique<LfxMock>();
    DXGIDxvkAdapterMock adapter;
    DXGIOutput6Mock output;

    auto e = ConfigureDefaultTestEnvironment(*dxgiFactory, *vulkan, *nvml, *lfx, adapter, output);
    auto primaryDisplayId = 0x00010001;

    auto desc1 = DXGI_OUTPUT_DESC1{
        L"Output1",
        {0, 0, 0, 0},
        1,
        DXGI_MODE_ROTATION_UNSPECIFIED,
        nullptr,
        8U,
        DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709,
        {0.640f, 0.330f},
        {0.300f, 0.600f},
        {0.150f, 0.060f},
        {0.3127f, 0.3290f},
        0.01f,
        1499.0f,
        799.0f};

    ALLOW_CALL(output, QueryInterface(__uuidof(IDXGIOutput6), _))
        .LR_SIDE_EFFECT(*_2 = static_cast<IDXGIOutput6*>(&output))
        .RETURN(S_OK);
    ALLOW_CALL(output, GetDesc1(_))
        .SIDE_EFFECT(*_1 = desc1)
        .RETURN(S_OK);

    SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml), std::move(lfx));
    REQUIRE(NvAPI_Initialize() == NVAPI_OK);

    SECTION("GetHdrCapabilities (V1) returns OK") {
        NV_HDR_CAPABILITIES_V1 capabilities;
        capabilities.version = NV_HDR_CAPABILITIES_VER1;
        REQUIRE(NvAPI_Disp_GetHdrCapabilities(primaryDisplayId, reinterpret_cast<NV_HDR_CAPABILITIES*>(&capabilities)) == NVAPI_OK);
        REQUIRE(capabilities.isST2084EotfSupported == false);
        REQUIRE(capabilities.isTraditionalHdrGammaSupported == false);
        REQUIRE(capabilities.isEdrSupported == false);
        REQUIRE(capabilities.driverExpandDefaultHdrParameters == false);
        REQUIRE(capabilities.isTraditionalSdrGammaSupported == false);
        REQUIRE(capabilities.display_data.displayPrimary_x0 == 32000);
        REQUIRE(capabilities.display_data.displayPrimary_y0 == 16500);
        REQUIRE(capabilities.display_data.displayPrimary_x1 == 15000);
        REQUIRE(capabilities.display_data.displayPrimary_y1 == 30000);
        REQUIRE(capabilities.display_data.displayPrimary_x2 == 7500);
        REQUIRE(capabilities.display_data.displayPrimary_y2 == 3000);
        REQUIRE(capabilities.display_data.displayWhitePoint_x == 15635);
        REQUIRE(capabilities.display_data.displayWhitePoint_y == 16450);
        REQUIRE(capabilities.display_data.desired_content_min_luminance == 0);
        REQUIRE(capabilities.display_data.desired_content_max_luminance == 1499);
        REQUIRE(capabilities.display_data.desired_content_max_frame_average_luminance == 799);
    }

    SECTION("GetHdrCapabilities (V2) returns OK") {
        NV_HDR_CAPABILITIES_V2 capabilities;
        capabilities.version = NV_HDR_CAPABILITIES_VER2;
        REQUIRE(NvAPI_Disp_GetHdrCapabilities(primaryDisplayId, reinterpret_cast<NV_HDR_CAPABILITIES*>(&capabilities)) == NVAPI_OK);
        REQUIRE(capabilities.isST2084EotfSupported == false);
        REQUIRE(capabilities.isTraditionalHdrGammaSupported == false);
        REQUIRE(capabilities.isEdrSupported == false);
        REQUIRE(capabilities.driverExpandDefaultHdrParameters == false);
        REQUIRE(capabilities.isTraditionalSdrGammaSupported == false);
        REQUIRE(capabilities.display_data.displayPrimary_x0 == 32000);
        REQUIRE(capabilities.display_data.displayPrimary_y0 == 16500);
        REQUIRE(capabilities.display_data.displayPrimary_x1 == 15000);
        REQUIRE(capabilities.display_data.displayPrimary_y1 == 30000);
        REQUIRE(capabilities.display_data.displayPrimary_x2 == 7500);
        REQUIRE(capabilities.display_data.displayPrimary_y2 == 3000);
        REQUIRE(capabilities.display_data.displayWhitePoint_x == 15635);
        REQUIRE(capabilities.display_data.displayWhitePoint_y == 16450);
        REQUIRE(capabilities.display_data.desired_content_min_luminance == 0);
        REQUIRE(capabilities.display_data.desired_content_max_luminance == 1499);
        REQUIRE(capabilities.display_data.desired_content_max_frame_average_luminance == 799);
    }

    SECTION("GetHdrCapabilities (V3) returns OK") {
        NV_HDR_CAPABILITIES_V3 capabilities;
        capabilities.version = NV_HDR_CAPABILITIES_VER3;
        REQUIRE(NvAPI_Disp_GetHdrCapabilities(primaryDisplayId, &capabilities) == NVAPI_OK);
        REQUIRE(capabilities.isST2084EotfSupported == false);
        REQUIRE(capabilities.isTraditionalHdrGammaSupported == false);
        REQUIRE(capabilities.isEdrSupported == false);
        REQUIRE(capabilities.driverExpandDefaultHdrParameters == false);
        REQUIRE(capabilities.isTraditionalSdrGammaSupported == false);
        REQUIRE(capabilities.isDolbyVisionSupported == false);
        REQUIRE(capabilities.display_data.displayPrimary_x0 == 32000);
        REQUIRE(capabilities.display_data.displayPrimary_y0 == 16500);
        REQUIRE(capabilities.display_data.displayPrimary_x1 == 15000);
        REQUIRE(capabilities.display_data.displayPrimary_y1 == 30000);
        REQUIRE(capabilities.display_data.displayPrimary_x2 == 7500);
        REQUIRE(capabilities.display_data.displayPrimary_y2 == 3000);
        REQUIRE(capabilities.display_data.displayWhitePoint_x == 15635);
        REQUIRE(capabilities.display_data.displayWhitePoint_y == 16450);
        REQUIRE(capabilities.display_data.desired_content_min_luminance == 0);
        REQUIRE(capabilities.display_data.desired_content_max_luminance == 1499);
        REQUIRE(capabilities.display_data.desired_content_max_frame_average_luminance == 799);
    }

    SECTION("GetHdrCapabilities with unknown struct version returns incompatible-struct-version") {
        NV_HDR_CAPABILITIES capabilities;
        capabilities.version = NV_HDR_CAPABILITIES_VER3 + 1;
        REQUIRE(NvAPI_Disp_GetHdrCapabilities(primaryDisplayId, &capabilities) == NVAPI_INCOMPATIBLE_STRUCT_VERSION);
    }

    SECTION("GetHdrCapabilities with current struct version returns not incompatible-struct-version") {
        // This test should fail when a header update provides a newer not yet implemented struct version
        NV_HDR_CAPABILITIES capabilities;
        capabilities.version = NV_HDR_CAPABILITIES_VER;
        REQUIRE(NvAPI_Disp_GetHdrCapabilities(primaryDisplayId, &capabilities) != NVAPI_INCOMPATIBLE_STRUCT_VERSION);
    }

    SECTION("HdrColorControl (V1) returns no-implementation") {
        NV_HDR_COLOR_DATA colorData;
        colorData.version = NV_HDR_COLOR_DATA_VER1;
        REQUIRE(NvAPI_Disp_HdrColorControl(primaryDisplayId, &colorData) == NVAPI_NO_IMPLEMENTATION);
    }

    SECTION("HdrColorControl (V2) returns no-implementation") {
        NV_HDR_COLOR_DATA colorData;
        colorData.version = NV_HDR_COLOR_DATA_VER2;
        REQUIRE(NvAPI_Disp_HdrColorControl(primaryDisplayId, &colorData) == NVAPI_NO_IMPLEMENTATION);
    }

    SECTION("GetHdrCapabilities with unknown struct version returns incompatible-struct-version") {
        NV_HDR_COLOR_DATA colorData;
        colorData.version = NV_HDR_COLOR_DATA_VER2 + 1;
        REQUIRE(NvAPI_Disp_HdrColorControl(primaryDisplayId, &colorData) == NVAPI_INCOMPATIBLE_STRUCT_VERSION);
    }

    SECTION("GetHdrCapabilities with current struct version returns not incompatible-struct-version") {
        // This test should fail when a header update provides a newer not yet implemented struct version
        NV_HDR_COLOR_DATA colorData;
        colorData.version = NV_HDR_COLOR_DATA_VER;
        REQUIRE(NvAPI_Disp_HdrColorControl(primaryDisplayId, &colorData) != NVAPI_INCOMPATIBLE_STRUCT_VERSION);
    }
}
