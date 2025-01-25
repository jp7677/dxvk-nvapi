#include "nvapi_private.h"
#include "nvapi_globals.h"
#include "util/util_statuscode.h"
#include "util/util_string.h"

extern "C" {
    using namespace dxvk;

    static DXGI_COLOR_SPACE_TYPE HDRModeToColorSpace(NV_HDR_MODE mode) {
        switch (mode) {
            default:
                log::info(str::format("Unhandled HDR type: ", mode, " Falling back to sRGB."));
                [[fallthrough]];
            case NV_HDR_MODE_SDR:
            case NV_HDR_MODE_OFF:
                return DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;

            case NV_HDR_MODE_UHDA:
                return DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709;
            case NV_HDR_MODE_UHDA_PASSTHROUGH:
                return DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020;
        }
    }

    static NV_HDR_MODE ColorSpaceToHDRMode(DXGI_COLOR_SPACE_TYPE colorspace) {
        switch (colorspace) {
            default:
                log::info(str::format("Unhandled colorspace type: ", colorspace, " Falling back to OFF."));
                [[fallthrough]];
            case DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709:
                return NV_HDR_MODE_OFF;

            case DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709:
                return NV_HDR_MODE_UHDA;
            case DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020:
                return NV_HDR_MODE_UHDA_PASSTHROUGH;
        }
    }

    NvAPI_Status __cdecl NvAPI_Disp_GetHdrCapabilities(NvU32 displayId, NV_HDR_CAPABILITIES* pHdrCapabilities) {
        constexpr auto n = __func__;

        if (log::tracing())
            log::trace(n, displayId, log::fmt::ptr(pHdrCapabilities));

        if (!nvapiAdapterRegistry)
            return ApiNotInitialized(n);

        if (!pHdrCapabilities)
            return InvalidArgument(n);

        auto output = nvapiAdapterRegistry->FindOutput(displayId);
        if (!output)
            return InvalidArgument(n);

        const auto& data = output->GetColorData();

        switch (pHdrCapabilities->version) {
            case NV_HDR_CAPABILITIES_VER1: {
                auto pHdrCapabilitiesV1 = reinterpret_cast<NV_HDR_CAPABILITIES_V1*>(pHdrCapabilities);
                *pHdrCapabilitiesV1 = {};
                pHdrCapabilitiesV1->version = NV_HDR_CAPABILITIES_VER1;
                pHdrCapabilitiesV1->isST2084EotfSupported = data.HasST2084Support;
                pHdrCapabilitiesV1->display_data.displayPrimary_x0 = data.RedPrimaryX;
                pHdrCapabilitiesV1->display_data.displayPrimary_y0 = data.RedPrimaryY;
                pHdrCapabilitiesV1->display_data.displayPrimary_x1 = data.GreenPrimaryX;
                pHdrCapabilitiesV1->display_data.displayPrimary_y1 = data.GreenPrimaryY;
                pHdrCapabilitiesV1->display_data.displayPrimary_x2 = data.BluePrimaryX;
                pHdrCapabilitiesV1->display_data.displayPrimary_y2 = data.BluePrimaryY;
                pHdrCapabilitiesV1->display_data.displayWhitePoint_x = data.WhitePointX;
                pHdrCapabilitiesV1->display_data.displayWhitePoint_y = data.WhitePointY;
                pHdrCapabilitiesV1->display_data.desired_content_min_luminance = data.MinLuminance;
                pHdrCapabilitiesV1->display_data.desired_content_max_luminance = data.MaxLuminance;
                pHdrCapabilitiesV1->display_data.desired_content_max_frame_average_luminance = data.MaxFullFrameLuminance;
                break;
            }
            case NV_HDR_CAPABILITIES_VER2: {
                auto pHdrCapabilitiesV2 = reinterpret_cast<NV_HDR_CAPABILITIES_V2*>(pHdrCapabilities);
                *pHdrCapabilitiesV2 = {};
                pHdrCapabilitiesV2->version = NV_HDR_CAPABILITIES_VER2;
                pHdrCapabilitiesV2->isST2084EotfSupported = data.HasST2084Support;
                pHdrCapabilitiesV2->display_data.displayPrimary_x0 = data.RedPrimaryX;
                pHdrCapabilitiesV2->display_data.displayPrimary_y0 = data.RedPrimaryY;
                pHdrCapabilitiesV2->display_data.displayPrimary_x1 = data.GreenPrimaryX;
                pHdrCapabilitiesV2->display_data.displayPrimary_y1 = data.GreenPrimaryY;
                pHdrCapabilitiesV2->display_data.displayPrimary_x2 = data.BluePrimaryX;
                pHdrCapabilitiesV2->display_data.displayPrimary_y2 = data.BluePrimaryY;
                pHdrCapabilitiesV2->display_data.displayWhitePoint_x = data.WhitePointX;
                pHdrCapabilitiesV2->display_data.displayWhitePoint_y = data.WhitePointY;
                pHdrCapabilitiesV2->display_data.desired_content_min_luminance = data.MinLuminance;
                pHdrCapabilitiesV2->display_data.desired_content_max_luminance = data.MaxLuminance;
                pHdrCapabilitiesV2->display_data.desired_content_max_frame_average_luminance = data.MaxFullFrameLuminance;
                break;
            }
            case NV_HDR_CAPABILITIES_VER3:
                *pHdrCapabilities = {};
                pHdrCapabilities->version = NV_HDR_CAPABILITIES_VER3;
                pHdrCapabilities->isST2084EotfSupported = data.HasST2084Support;
                pHdrCapabilities->display_data.displayPrimary_x0 = data.RedPrimaryX;
                pHdrCapabilities->display_data.displayPrimary_y0 = data.RedPrimaryY;
                pHdrCapabilities->display_data.displayPrimary_x1 = data.GreenPrimaryX;
                pHdrCapabilities->display_data.displayPrimary_y1 = data.GreenPrimaryY;
                pHdrCapabilities->display_data.displayPrimary_x2 = data.BluePrimaryX;
                pHdrCapabilities->display_data.displayPrimary_y2 = data.BluePrimaryY;
                pHdrCapabilities->display_data.displayWhitePoint_x = data.WhitePointX;
                pHdrCapabilities->display_data.displayWhitePoint_y = data.WhitePointY;
                pHdrCapabilities->display_data.desired_content_min_luminance = data.MinLuminance;
                pHdrCapabilities->display_data.desired_content_max_luminance = data.MaxLuminance;
                pHdrCapabilities->display_data.desired_content_max_frame_average_luminance = data.MaxFullFrameLuminance;
                break;
            default:
                return IncompatibleStructVersion(n, pHdrCapabilities->version);
        }

        return Ok(str::format(n, " (0x", std::hex, displayId, ")"));
    }

    NvAPI_Status __cdecl NvAPI_Disp_HdrColorControl(NvU32 displayId, NV_HDR_COLOR_DATA* pHdrColorData) {
        constexpr auto n = __func__;

        if (log::tracing())
            log::trace(n, displayId, log::fmt::ptr(pHdrColorData));

        if (!nvapiAdapterRegistry)
            return ApiNotInitialized(n);

        if (!pHdrColorData)
            return InvalidArgument(n);

        auto output = nvapiAdapterRegistry->FindOutput(displayId);
        if (!output)
            return InvalidArgument(n);

        auto interop = nvapiAdapterRegistry->GetInteropFactory();
        if (!interop)
            return NoImplementation(str::format(n, " (0x", std::hex, displayId, ")"));

        auto& data = output->GetColorData();
        // V2 is based on V1.
        if (pHdrColorData->version == NV_HDR_COLOR_DATA_VER1 || pHdrColorData->version == NV_HDR_COLOR_DATA_VER2) {
            auto pHDRColorDataV1 = reinterpret_cast<NV_HDR_COLOR_DATA_V1*>(pHdrColorData);
            if (pHDRColorDataV1->cmd == NV_HDR_CMD_GET) {
                DXGI_COLOR_SPACE_TYPE colorspace;
                DXGI_HDR_METADATA_HDR10 metadata;
                if (interop->GetGlobalHDRState(&colorspace, &metadata) == S_OK) {
                    pHDRColorDataV1->hdrMode = ColorSpaceToHDRMode(colorspace);
                    pHDRColorDataV1->static_metadata_descriptor_id = NV_STATIC_METADATA_TYPE_1;
                    pHDRColorDataV1->mastering_display_data.displayPrimary_x0 = metadata.RedPrimary[0];
                    pHDRColorDataV1->mastering_display_data.displayPrimary_y0 = metadata.RedPrimary[1];
                    pHDRColorDataV1->mastering_display_data.displayPrimary_x1 = metadata.GreenPrimary[0];
                    pHDRColorDataV1->mastering_display_data.displayPrimary_y1 = metadata.GreenPrimary[1];
                    pHDRColorDataV1->mastering_display_data.displayPrimary_x2 = metadata.BluePrimary[0];
                    pHDRColorDataV1->mastering_display_data.displayPrimary_y2 = metadata.BluePrimary[1];
                    pHDRColorDataV1->mastering_display_data.displayWhitePoint_x = metadata.WhitePoint[0];
                    pHDRColorDataV1->mastering_display_data.displayWhitePoint_y = metadata.WhitePoint[1];
                    pHDRColorDataV1->mastering_display_data.max_display_mastering_luminance = metadata.MaxMasteringLuminance;
                    pHDRColorDataV1->mastering_display_data.min_display_mastering_luminance = metadata.MinMasteringLuminance;
                    pHDRColorDataV1->mastering_display_data.max_content_light_level = metadata.MaxContentLightLevel;
                    pHDRColorDataV1->mastering_display_data.max_frame_average_light_level = metadata.MaxFrameAverageLightLevel;
                } else {
                    // If we have ST2084 support (only enabled with DXVK_HDR), say we are in UHDA scRGB currently HDR to
                    // encourage apps to use it by default.
                    pHDRColorDataV1->hdrMode = data.HasST2084Support ? NV_HDR_MODE_UHDA : NV_HDR_MODE_OFF;
                    pHDRColorDataV1->static_metadata_descriptor_id = NV_STATIC_METADATA_TYPE_1;
                    pHDRColorDataV1->mastering_display_data.displayPrimary_x0 = data.RedPrimaryX;
                    pHDRColorDataV1->mastering_display_data.displayPrimary_y0 = data.RedPrimaryY;
                    pHDRColorDataV1->mastering_display_data.displayPrimary_x1 = data.GreenPrimaryX;
                    pHDRColorDataV1->mastering_display_data.displayPrimary_y1 = data.GreenPrimaryY;
                    pHDRColorDataV1->mastering_display_data.displayPrimary_x2 = data.BluePrimaryX;
                    pHDRColorDataV1->mastering_display_data.displayPrimary_y2 = data.BluePrimaryY;
                    pHDRColorDataV1->mastering_display_data.displayWhitePoint_x = data.WhitePointX;
                    pHDRColorDataV1->mastering_display_data.displayWhitePoint_y = data.WhitePointY;
                    pHDRColorDataV1->mastering_display_data.max_display_mastering_luminance = data.MaxLuminance;
                    pHDRColorDataV1->mastering_display_data.min_display_mastering_luminance = data.MinLuminance;
                    pHDRColorDataV1->mastering_display_data.max_content_light_level = data.MaxLuminance;
                    pHDRColorDataV1->mastering_display_data.max_frame_average_light_level = data.MaxFullFrameLuminance;
                }
            } else {
                DXGI_COLOR_SPACE_TYPE colorspace = HDRModeToColorSpace(pHDRColorDataV1->hdrMode);
                DXGI_HDR_METADATA_HDR10 metadata = {
                    .RedPrimary = {pHDRColorDataV1->mastering_display_data.displayPrimary_x0, pHDRColorDataV1->mastering_display_data.displayPrimary_y0},
                    .GreenPrimary = {pHDRColorDataV1->mastering_display_data.displayPrimary_x1, pHDRColorDataV1->mastering_display_data.displayPrimary_y1},
                    .BluePrimary = {pHDRColorDataV1->mastering_display_data.displayPrimary_x2, pHDRColorDataV1->mastering_display_data.displayPrimary_y2},
                    .WhitePoint = {pHDRColorDataV1->mastering_display_data.displayWhitePoint_x, pHDRColorDataV1->mastering_display_data.displayWhitePoint_y},
                    .MaxMasteringLuminance = pHDRColorDataV1->mastering_display_data.max_display_mastering_luminance,
                    .MinMasteringLuminance = pHDRColorDataV1->mastering_display_data.min_display_mastering_luminance,
                    .MaxContentLightLevel = pHDRColorDataV1->mastering_display_data.max_content_light_level,
                    .MaxFrameAverageLightLevel = pHDRColorDataV1->mastering_display_data.max_frame_average_light_level,
                };
                if (FAILED(interop->SetGlobalHDRState(colorspace, &metadata)))
                    return InvalidArgument(n);
            }
        } else {
            return IncompatibleStructVersion(n, pHdrColorData->version);
        }

        if (pHdrColorData->version == NV_HDR_COLOR_DATA_VER2) {
            auto pHDRColorDataV2 = reinterpret_cast<NV_HDR_COLOR_DATA_V2*>(pHdrColorData);
            if (pHDRColorDataV2->cmd == NV_HDR_CMD_GET) {
                pHDRColorDataV2->hdrColorFormat = NV_COLOR_FORMAT_RGB;
                // NV_DYNAMIC_RANGE_VESA is RGB full range
                // NV_DYNAMIC_RANGE_CEA is RGB/YCbCr limited range
                // NV_DYNAMIC_RANGE_AUTO automatically chooses something (probably reads some EDID/CTA-861 entries)
                pHDRColorDataV2->hdrDynamicRange = NV_DYNAMIC_RANGE_VESA;
                pHDRColorDataV2->hdrBpc = data.BitsPerColor;
            } else {
                // Ignoring some extended properties of HDRColorDataV2.
                // Nothing to set. It's all random useless garbage that you would NEVER trust an app to set.
            }
        }

        return Ok(str::format(n, " (0x", std::hex, displayId, "/", (pHdrColorData->cmd == NV_HDR_CMD_GET ? "Get" : "Set"), ")"));
    }

    NvAPI_Status __cdecl NvAPI_DISP_GetDisplayIdByDisplayName(const char* displayName, NvU32* displayId) {
        constexpr auto n = __func__;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(displayName), log::fmt::ptr(displayId));

        if (!nvapiAdapterRegistry)
            return ApiNotInitialized(n);

        if (!displayName || !displayId)
            return InvalidArgument(n);

        auto output = nvapiAdapterRegistry->FindOutput(std::string(displayName));
        if (!output)
            return InvalidArgument(str::format(n, " (", displayName, ")"));

        *displayId = output->GetId();

        return Ok(str::format(n, " (", displayName, ")"));
    }

    NvAPI_Status __cdecl NvAPI_DISP_GetGDIPrimaryDisplayId(NvU32* displayId) {
        constexpr auto n = __func__;

        if (log::tracing())
            log::trace(n, log::fmt::ptr(displayId));

        if (!nvapiAdapterRegistry)
            return ApiNotInitialized(n);

        if (!displayId)
            return InvalidArgument(n);

        auto output = nvapiAdapterRegistry->FindPrimaryOutput();
        if (!output)
            return NvidiaDeviceNotFound(n);

        *displayId = output->GetId();

        return Ok(n);
    }
}
