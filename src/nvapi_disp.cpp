#include "nvapi_private.h"
#include "nvapi_globals.h"
#include "util/util_statuscode.h"
#include "util/util_string.h"

extern "C" {
    using namespace dxvk;

    NvAPI_Status __cdecl NvAPI_Disp_GetHdrCapabilities(NvU32 displayId, NV_HDR_CAPABILITIES* pHdrCapabilities) {
        constexpr auto n = __func__;

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (pHdrCapabilities == nullptr)
            return InvalidArgument(n);

        auto output = nvapiAdapterRegistry->FindOutput(displayId);
        if (output == nullptr)
            return InvalidArgument(n);

        const auto& data = output->GetColorData();

        // Report that HDR is not available
        // Note that DXVK starts to support HDR at the time of writing, so we could report HDR capabilities
        // somewhat correctly by looking at DXGI_OUTPUT_DESC1 (IDXGIOutput6->GetDesc1).
        // Doing so would only make sense when also fully implementing NvAPI_Disp_HdrColorControl to set HDR properties,
        // but implementing that function correctly is not possible since DXVK/DX sets HDR properties on a
        // SwapChain and not on a display/output.
        // Reference (a.o.): https://www.asawicki.info/news_1703_programming_hdr_monitor_support_in_direct3d
        switch (pHdrCapabilities->version) {
            case NV_HDR_CAPABILITIES_VER1: {
                auto pHdrCapabilitiesV1 = reinterpret_cast<NV_HDR_CAPABILITIES_V1*>(pHdrCapabilities);
                *pHdrCapabilitiesV1 = {};
                pHdrCapabilitiesV1->isST2084EotfSupported = false; // data.HasST2084Support;
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
                pHdrCapabilitiesV2->isST2084EotfSupported = false; // data.HasST2084Support;
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
                pHdrCapabilities->isST2084EotfSupported = false; // data.HasST2084Support;
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
                return IncompatibleStructVersion(n);
        }

        return Ok(str::format(n, " (0x", std::hex, displayId, ")"));
    }

    NvAPI_Status __cdecl NvAPI_Disp_HdrColorControl(NvU32 displayId, NV_HDR_COLOR_DATA* pHdrColorData) {
        constexpr auto n = __func__;

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (pHdrColorData == nullptr)
            return InvalidArgument(n);

        auto output = nvapiAdapterRegistry->FindOutput(displayId);
        if (output == nullptr)
            return InvalidArgument(n);

        switch (pHdrColorData->version) {
            case NV_HDR_COLOR_DATA_VER1:
            case NV_HDR_COLOR_DATA_VER2:
                break;
            default:
                return IncompatibleStructVersion(n);
        }

        return NoImplementation(str::format(n, " (0x", std::hex, displayId, ")")); // See comment in NvAPI_Disp_GetHdrCapabilities
    }

    NvAPI_Status __cdecl NvAPI_DISP_GetDisplayIdByDisplayName(const char* displayName, NvU32* displayId) {
        constexpr auto n = __func__;

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (displayName == nullptr || displayId == nullptr)
            return InvalidArgument(n);

        auto output = nvapiAdapterRegistry->FindOutput(std::string(displayName));
        if (output == nullptr)
            return InvalidArgument(str::format(n, " (", displayName, ")"));

        *displayId = output->GetId();

        return Ok(str::format(n, " (", displayName, ")"));
    }

    NvAPI_Status __cdecl NvAPI_DISP_GetGDIPrimaryDisplayId(NvU32* displayId) {
        constexpr auto n = __func__;

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (displayId == nullptr)
            return InvalidArgument(n);

        auto output = nvapiAdapterRegistry->FindPrimaryOutput();
        if (output == nullptr)
            return NvidiaDeviceNotFound(n);

        *displayId = output->GetId();

        return Ok(n);
    }
}
