#include "nvapi_tests_private.h"
#include "nvapi/resource_factory_util.h"

using namespace trompeloeil;
using namespace Catch::Matchers;

TEST_CASE("HDR related sysinfo methods succeed", "[.sysinfo-hdr]") {
    auto dxgiFactory = std::make_unique<DXGIDxvkFactoryMock>();
    auto vk = std::make_unique<VkMock>();
    auto nvml = std::make_unique<NvmlMock>();
    DXGIDxvkAdapterMock* adapter = CreateDXGIDxvkAdapterMock();
    DXGIOutput6Mock* output = CreateDXGIOutput6Mock();

    auto e = ConfigureDefaultTestEnvironment(*dxgiFactory, *vk, *nvml, *adapter, *output);
    auto primaryDisplayId = 0x00010001;

    auto desc1 = DXGI_OUTPUT_DESC1{
        L"Output1",
        {0, 0, 0, 0},
        1,
        DXGI_MODE_ROTATION_UNSPECIFIED,
        nullptr,
        10U,
        DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020,
        {0.640f, 0.330f},
        {0.300f, 0.600f},
        {0.150f, 0.060f},
        {0.3127f, 0.3290f},
        0.01f,
        1499.0f,
        799.0f};

    ALLOW_CALL(*output, QueryInterface(__uuidof(IDXGIOutput6), _))
        .LR_SIDE_EFFECT(*_2 = static_cast<IDXGIOutput6*>(output))
        .RETURN(S_OK);
    ALLOW_CALL(*output, GetDesc1(_))
        .SIDE_EFFECT(*_1 = desc1)
        .RETURN(S_OK);

    SECTION("GetHdrCapabilities succeeds") {
        SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        SECTION("GetHdrCapabilities (V1) returns OK") {
            NV_HDR_CAPABILITIES_V1 capabilities{};
            capabilities.version = NV_HDR_CAPABILITIES_VER1;
            REQUIRE(NvAPI_Disp_GetHdrCapabilities(primaryDisplayId, reinterpret_cast<NV_HDR_CAPABILITIES*>(&capabilities)) == NVAPI_OK);
            REQUIRE(capabilities.isST2084EotfSupported == true);
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
            REQUIRE(capabilities.display_data.desired_content_min_luminance == 100);
            REQUIRE(capabilities.display_data.desired_content_max_luminance == 1499);
            REQUIRE(capabilities.display_data.desired_content_max_frame_average_luminance == 799);
        }

        SECTION("GetHdrCapabilities (V2) returns OK") {
            NV_HDR_CAPABILITIES_V2 capabilities{};
            capabilities.version = NV_HDR_CAPABILITIES_VER2;
            REQUIRE(NvAPI_Disp_GetHdrCapabilities(primaryDisplayId, reinterpret_cast<NV_HDR_CAPABILITIES*>(&capabilities)) == NVAPI_OK);
            REQUIRE(capabilities.isST2084EotfSupported == true);
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
            REQUIRE(capabilities.display_data.desired_content_min_luminance == 100);
            REQUIRE(capabilities.display_data.desired_content_max_luminance == 1499);
            REQUIRE(capabilities.display_data.desired_content_max_frame_average_luminance == 799);
        }

        SECTION("GetHdrCapabilities (V3) returns OK") {
            NV_HDR_CAPABILITIES_V3 capabilities{};
            capabilities.version = NV_HDR_CAPABILITIES_VER3;
            REQUIRE(NvAPI_Disp_GetHdrCapabilities(primaryDisplayId, &capabilities) == NVAPI_OK);
            REQUIRE(capabilities.isST2084EotfSupported == true);
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
            REQUIRE(capabilities.display_data.desired_content_min_luminance == 100);
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
    }

    SECTION("HdrColorControl succeeds") {
        DXGI_COLOR_SPACE_TYPE outColorSpace = DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020;
        DXGI_HDR_METADATA_HDR10 outMetadata{
            .RedPrimary = {32000, 16500},
            .GreenPrimary = {15000, 30000},
            .BluePrimary = {7500, 3000},
            .WhitePoint = {15635, 16450},
            .MaxMasteringLuminance = 1499,
            .MinMasteringLuminance = 100,
            .MaxContentLightLevel = 1199,
            .MaxFrameAverageLightLevel = 799};
        ALLOW_CALL(*dxgiFactory, GetGlobalHDRState(_, _))
            .LR_SIDE_EFFECT({
                *_1 = outColorSpace;
                *_2 = outMetadata;
            })
            .RETURN(S_OK);

        DXGI_COLOR_SPACE_TYPE inColorSpace = DXGI_COLOR_SPACE_RESERVED;
        DXGI_HDR_METADATA_HDR10 inMetadata{};
        ALLOW_CALL(*dxgiFactory, SetGlobalHDRState(_, _))
            .LR_SIDE_EFFECT({
                inColorSpace = _1;
                inMetadata = *_2;
            })
            .RETURN(S_OK);

        SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        SECTION("HdrColorControl (V1) with get command returns OK") {
            NV_HDR_COLOR_DATA colorData{};
            colorData.version = NV_HDR_COLOR_DATA_VER1;
            colorData.cmd = NV_HDR_CMD_GET;
            REQUIRE(NvAPI_Disp_HdrColorControl(primaryDisplayId, &colorData) == NVAPI_OK);
            REQUIRE(colorData.hdrMode == NV_HDR_MODE_UHDA_PASSTHROUGH);
            REQUIRE(colorData.static_metadata_descriptor_id == NV_STATIC_METADATA_TYPE_1);
            REQUIRE(colorData.mastering_display_data.displayPrimary_x0 == 32000);
            REQUIRE(colorData.mastering_display_data.displayPrimary_y0 == 16500);
            REQUIRE(colorData.mastering_display_data.displayPrimary_x1 == 15000);
            REQUIRE(colorData.mastering_display_data.displayPrimary_y1 == 30000);
            REQUIRE(colorData.mastering_display_data.displayPrimary_x2 == 7500);
            REQUIRE(colorData.mastering_display_data.displayPrimary_y2 == 3000);
            REQUIRE(colorData.mastering_display_data.displayWhitePoint_x == 15635);
            REQUIRE(colorData.mastering_display_data.displayWhitePoint_y == 16450);
            REQUIRE(colorData.mastering_display_data.min_display_mastering_luminance == 100);
            REQUIRE(colorData.mastering_display_data.max_display_mastering_luminance == 1499);
            REQUIRE(colorData.mastering_display_data.max_content_light_level == 1199);
            REQUIRE(colorData.mastering_display_data.max_frame_average_light_level == 799);
        }

        SECTION("HdrColorControl (V1) with set command returns OK") {
            NV_HDR_COLOR_DATA colorData{};
            colorData.version = NV_HDR_COLOR_DATA_VER1;
            colorData.cmd = NV_HDR_CMD_SET;
            colorData.hdrMode = NV_HDR_MODE_UHDA_PASSTHROUGH;
            colorData.mastering_display_data.displayPrimary_x0 = 32000;
            colorData.mastering_display_data.displayPrimary_y0 = 16500;
            colorData.mastering_display_data.displayPrimary_x1 = 15000;
            colorData.mastering_display_data.displayPrimary_y1 = 30000;
            colorData.mastering_display_data.displayPrimary_x2 = 7500;
            colorData.mastering_display_data.displayPrimary_y2 = 3000;
            colorData.mastering_display_data.displayWhitePoint_x = 15635;
            colorData.mastering_display_data.displayWhitePoint_y = 16450;
            colorData.mastering_display_data.min_display_mastering_luminance = 100;
            colorData.mastering_display_data.max_display_mastering_luminance = 1499;
            colorData.mastering_display_data.max_content_light_level = 1199;
            colorData.mastering_display_data.max_frame_average_light_level = 799;

            REQUIRE(NvAPI_Disp_HdrColorControl(primaryDisplayId, &colorData) == NVAPI_OK);
            REQUIRE(inColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020);
            REQUIRE(inMetadata.RedPrimary[0] == 32000);
            REQUIRE(inMetadata.RedPrimary[1] == 16500);
            REQUIRE(inMetadata.GreenPrimary[0] == 15000);
            REQUIRE(inMetadata.GreenPrimary[1] == 30000);
            REQUIRE(inMetadata.BluePrimary[0] == 7500);
            REQUIRE(inMetadata.BluePrimary[1] == 3000);
            REQUIRE(inMetadata.WhitePoint[0] == 15635);
            REQUIRE(inMetadata.WhitePoint[1] == 16450);
            REQUIRE(inMetadata.MinMasteringLuminance == 100);
            REQUIRE(inMetadata.MaxMasteringLuminance == 1499);
            REQUIRE(inMetadata.MaxContentLightLevel == 1199);
            REQUIRE(inMetadata.MaxFrameAverageLightLevel == 799);
        }

        SECTION("HdrColorControl (V2) with get command returns OK") {
            NV_HDR_COLOR_DATA colorData{};
            colorData.version = NV_HDR_COLOR_DATA_VER2;
            colorData.cmd = NV_HDR_CMD_GET;
            REQUIRE(NvAPI_Disp_HdrColorControl(primaryDisplayId, &colorData) == NVAPI_OK);
            REQUIRE(colorData.hdrMode == NV_HDR_MODE_UHDA_PASSTHROUGH);
            REQUIRE(colorData.static_metadata_descriptor_id == NV_STATIC_METADATA_TYPE_1);
            REQUIRE(colorData.mastering_display_data.displayPrimary_x0 == 32000);
            REQUIRE(colorData.mastering_display_data.displayPrimary_y0 == 16500);
            REQUIRE(colorData.mastering_display_data.displayPrimary_x1 == 15000);
            REQUIRE(colorData.mastering_display_data.displayPrimary_y1 == 30000);
            REQUIRE(colorData.mastering_display_data.displayPrimary_x2 == 7500);
            REQUIRE(colorData.mastering_display_data.displayPrimary_y2 == 3000);
            REQUIRE(colorData.mastering_display_data.displayWhitePoint_x == 15635);
            REQUIRE(colorData.mastering_display_data.displayWhitePoint_y == 16450);
            REQUIRE(colorData.mastering_display_data.min_display_mastering_luminance == 100);
            REQUIRE(colorData.mastering_display_data.max_display_mastering_luminance == 1499);
            REQUIRE(colorData.mastering_display_data.max_content_light_level == 1199);
            REQUIRE(colorData.mastering_display_data.max_frame_average_light_level == 799);
            REQUIRE(colorData.hdrColorFormat == NV_COLOR_FORMAT_RGB);
            REQUIRE(colorData.hdrDynamicRange == NV_DYNAMIC_RANGE_VESA);
            REQUIRE(colorData.hdrBpc == NV_BPC_10);
        }

        SECTION("HdrColorControl (V2) with set command returns OK") {
            NV_HDR_COLOR_DATA colorData{};
            colorData.version = NV_HDR_COLOR_DATA_VER2;
            colorData.cmd = NV_HDR_CMD_SET;
            colorData.hdrMode = NV_HDR_MODE_UHDA_PASSTHROUGH;
            colorData.mastering_display_data.displayPrimary_x0 = 32000;
            colorData.mastering_display_data.displayPrimary_y0 = 16500;
            colorData.mastering_display_data.displayPrimary_x1 = 15000;
            colorData.mastering_display_data.displayPrimary_y1 = 30000;
            colorData.mastering_display_data.displayPrimary_x2 = 7500;
            colorData.mastering_display_data.displayPrimary_y2 = 3000;
            colorData.mastering_display_data.displayWhitePoint_x = 15635;
            colorData.mastering_display_data.displayWhitePoint_y = 16450;
            colorData.mastering_display_data.min_display_mastering_luminance = 100;
            colorData.mastering_display_data.max_display_mastering_luminance = 1499;
            colorData.mastering_display_data.max_content_light_level = 1199;
            colorData.mastering_display_data.max_frame_average_light_level = 799;

            REQUIRE(NvAPI_Disp_HdrColorControl(primaryDisplayId, &colorData) == NVAPI_OK);
            REQUIRE(inColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020);
            REQUIRE(inMetadata.RedPrimary[0] == 32000);
            REQUIRE(inMetadata.RedPrimary[1] == 16500);
            REQUIRE(inMetadata.GreenPrimary[0] == 15000);
            REQUIRE(inMetadata.GreenPrimary[1] == 30000);
            REQUIRE(inMetadata.BluePrimary[0] == 7500);
            REQUIRE(inMetadata.BluePrimary[1] == 3000);
            REQUIRE(inMetadata.WhitePoint[0] == 15635);
            REQUIRE(inMetadata.WhitePoint[1] == 16450);
            REQUIRE(inMetadata.MinMasteringLuminance == 100);
            REQUIRE(inMetadata.MaxMasteringLuminance == 1499);
            REQUIRE(inMetadata.MaxContentLightLevel == 1199);
            REQUIRE(inMetadata.MaxFrameAverageLightLevel == 799);
        }

        SECTION("HdrColorControl with unknown struct version returns incompatible-struct-version") {
            NV_HDR_COLOR_DATA colorData;
            colorData.version = NV_HDR_COLOR_DATA_VER2 + 1;
            REQUIRE(NvAPI_Disp_HdrColorControl(primaryDisplayId, &colorData) == NVAPI_INCOMPATIBLE_STRUCT_VERSION);
        }

        SECTION("HdrColorControl with current struct version returns not incompatible-struct-version") {
            // This test should fail when a header update provides a newer not yet implemented struct version
            NV_HDR_COLOR_DATA colorData;
            colorData.version = NV_HDR_COLOR_DATA_VER;
            REQUIRE(NvAPI_Disp_HdrColorControl(primaryDisplayId, &colorData) != NVAPI_INCOMPATIBLE_STRUCT_VERSION);
        }
    }

    SECTION("HdrColorControl with failed get-global-hdr-state succeeds") {
        ALLOW_CALL(*dxgiFactory, GetGlobalHDRState(_, _))
            .RETURN(E_FAIL);

        SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

        SECTION("HdrColorControl (V1) with get command returns OK") {
            NV_HDR_COLOR_DATA colorData{};
            colorData.version = NV_HDR_COLOR_DATA_VER1;
            colorData.cmd = NV_HDR_CMD_GET;
            REQUIRE(NvAPI_Disp_HdrColorControl(primaryDisplayId, &colorData) == NVAPI_OK);
            REQUIRE(colorData.hdrMode == NV_HDR_MODE_UHDA);
            REQUIRE(colorData.static_metadata_descriptor_id == NV_STATIC_METADATA_TYPE_1);
            REQUIRE(colorData.mastering_display_data.displayPrimary_x0 == 32000);
            REQUIRE(colorData.mastering_display_data.displayPrimary_y0 == 16500);
            REQUIRE(colorData.mastering_display_data.displayPrimary_x1 == 15000);
            REQUIRE(colorData.mastering_display_data.displayPrimary_y1 == 30000);
            REQUIRE(colorData.mastering_display_data.displayPrimary_x2 == 7500);
            REQUIRE(colorData.mastering_display_data.displayPrimary_y2 == 3000);
            REQUIRE(colorData.mastering_display_data.displayWhitePoint_x == 15635);
            REQUIRE(colorData.mastering_display_data.displayWhitePoint_y == 16450);
            REQUIRE(colorData.mastering_display_data.min_display_mastering_luminance == 100);
            REQUIRE(colorData.mastering_display_data.max_display_mastering_luminance == 1499);
            REQUIRE(colorData.mastering_display_data.max_content_light_level == 1499);
            REQUIRE(colorData.mastering_display_data.max_frame_average_light_level == 799);
        }

        SECTION("HdrColorControl (V2) with get command returns OK") {
            NV_HDR_COLOR_DATA colorData{};
            colorData.version = NV_HDR_COLOR_DATA_VER2;
            colorData.cmd = NV_HDR_CMD_GET;
            REQUIRE(NvAPI_Disp_HdrColorControl(primaryDisplayId, &colorData) == NVAPI_OK);
            REQUIRE(colorData.hdrMode == NV_HDR_MODE_UHDA);
            REQUIRE(colorData.static_metadata_descriptor_id == NV_STATIC_METADATA_TYPE_1);
            REQUIRE(colorData.mastering_display_data.displayPrimary_x0 == 32000);
            REQUIRE(colorData.mastering_display_data.displayPrimary_y0 == 16500);
            REQUIRE(colorData.mastering_display_data.displayPrimary_x1 == 15000);
            REQUIRE(colorData.mastering_display_data.displayPrimary_y1 == 30000);
            REQUIRE(colorData.mastering_display_data.displayPrimary_x2 == 7500);
            REQUIRE(colorData.mastering_display_data.displayPrimary_y2 == 3000);
            REQUIRE(colorData.mastering_display_data.displayWhitePoint_x == 15635);
            REQUIRE(colorData.mastering_display_data.displayWhitePoint_y == 16450);
            REQUIRE(colorData.mastering_display_data.min_display_mastering_luminance == 100);
            REQUIRE(colorData.mastering_display_data.max_display_mastering_luminance == 1499);
            REQUIRE(colorData.mastering_display_data.max_content_light_level == 1499);
            REQUIRE(colorData.mastering_display_data.max_frame_average_light_level == 799);
            REQUIRE(colorData.hdrColorFormat == NV_COLOR_FORMAT_RGB);
            REQUIRE(colorData.hdrDynamicRange == NV_DYNAMIC_RANGE_VESA);
            REQUIRE(colorData.hdrBpc == NV_BPC_10);
        }
    }

    SECTION("HdrColorControl without DXGI interop returns no-implementation") {
        ALLOW_CALL(*dxgiFactory, QueryInterface(__uuidof(IDXGIVkInteropFactory1), _))
            .RETURN(E_NOINTERFACE);

        SetupResourceFactory(std::move(dxgiFactory), std::move(vk), std::move(nvml));
        REQUIRE(NvAPI_Initialize() == NVAPI_OK);

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
    }
}
