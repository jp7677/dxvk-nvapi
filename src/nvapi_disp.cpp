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

        // Report that HDR is not available
        // Note that DXVK starts to support HDR at the time of writing, so we could report HDR capabilities
        // somewhat correctly by looking at DXGI_OUTPUT_DESC1 (IDXGIOutput6->GetDesc1).
        // Doing so would only make sense when also adding NvAPI_Disp_HdrColorControl to set HDR properties,
        // but implementing that function correctly is not possible since DXVK/DX sets HDR properties on a
        // SwapChain and not on a display/output.
        // Reference (a.o.): https://www.asawicki.info/news_1703_programming_hdr_monitor_support_in_direct3d
        switch (pHdrCapabilities->version) {
            case NV_HDR_CAPABILITIES_VER1: {
                memset(pHdrCapabilities, 0, sizeof(NV_HDR_CAPABILITIES_V1));
                pHdrCapabilities->isST2084EotfSupported = false;
                pHdrCapabilities->display_data.displayPrimary_x0 = output->GetColorData().RedPrimaryX;
                pHdrCapabilities->display_data.displayPrimary_y0 = output->GetColorData().RedPrimaryY;
                pHdrCapabilities->display_data.displayPrimary_x1 = output->GetColorData().GreenPrimaryX;
                pHdrCapabilities->display_data.displayPrimary_y1 = output->GetColorData().GreenPrimaryY;
                pHdrCapabilities->display_data.displayPrimary_x2 = output->GetColorData().BluePrimaryX;
                pHdrCapabilities->display_data.displayPrimary_y2 = output->GetColorData().BluePrimaryY;
                pHdrCapabilities->display_data.displayWhitePoint_x = output->GetColorData().WhitePointX;
                pHdrCapabilities->display_data.displayWhitePoint_y = output->GetColorData().WhitePointY;
                pHdrCapabilities->display_data.desired_content_min_luminance = output->GetColorData().MinLuminance;
                pHdrCapabilities->display_data.desired_content_max_luminance = output->GetColorData().MaxLuminance;
                pHdrCapabilities->display_data.desired_content_max_frame_average_luminance = output->GetColorData().MaxFullFrameLuminance;
                break;
            }
            case NV_HDR_CAPABILITIES_VER2: {
                memset(pHdrCapabilities, 0, sizeof(NV_HDR_CAPABILITIES_V2));
                pHdrCapabilities->isST2084EotfSupported = false;
                pHdrCapabilities->display_data.displayPrimary_x0 = output->GetColorData().RedPrimaryX;
                pHdrCapabilities->display_data.displayPrimary_y0 = output->GetColorData().RedPrimaryY;
                pHdrCapabilities->display_data.displayPrimary_x1 = output->GetColorData().GreenPrimaryX;
                pHdrCapabilities->display_data.displayPrimary_y1 = output->GetColorData().GreenPrimaryY;
                pHdrCapabilities->display_data.displayPrimary_x2 = output->GetColorData().BluePrimaryX;
                pHdrCapabilities->display_data.displayPrimary_y2 = output->GetColorData().BluePrimaryY;
                pHdrCapabilities->display_data.displayWhitePoint_x = output->GetColorData().WhitePointX;
                pHdrCapabilities->display_data.displayWhitePoint_y = output->GetColorData().WhitePointY;
                pHdrCapabilities->display_data.desired_content_min_luminance = output->GetColorData().MinLuminance;
                pHdrCapabilities->display_data.desired_content_max_luminance = output->GetColorData().MaxLuminance;
                pHdrCapabilities->display_data.desired_content_max_frame_average_luminance = output->GetColorData().MaxFullFrameLuminance;
                break;
            }
            case NV_HDR_CAPABILITIES_VER3:
                memset(pHdrCapabilities, 0, sizeof(NV_HDR_CAPABILITIES_V3));
                pHdrCapabilities->isST2084EotfSupported = false;
                pHdrCapabilities->display_data.displayPrimary_x0 = output->GetColorData().RedPrimaryX;
                pHdrCapabilities->display_data.displayPrimary_y0 = output->GetColorData().RedPrimaryY;
                pHdrCapabilities->display_data.displayPrimary_x1 = output->GetColorData().GreenPrimaryX;
                pHdrCapabilities->display_data.displayPrimary_y1 = output->GetColorData().GreenPrimaryY;
                pHdrCapabilities->display_data.displayPrimary_x2 = output->GetColorData().BluePrimaryX;
                pHdrCapabilities->display_data.displayPrimary_y2 = output->GetColorData().BluePrimaryY;
                pHdrCapabilities->display_data.displayWhitePoint_x = output->GetColorData().WhitePointX;
                pHdrCapabilities->display_data.displayWhitePoint_y = output->GetColorData().WhitePointY;
                pHdrCapabilities->display_data.desired_content_min_luminance = output->GetColorData().MinLuminance;
                pHdrCapabilities->display_data.desired_content_max_luminance = output->GetColorData().MaxLuminance;
                pHdrCapabilities->display_data.desired_content_max_frame_average_luminance = output->GetColorData().MaxFullFrameLuminance;
                break;
            default:
                return IncompatibleStructVersion(n);
        }

        return Ok(str::format(n, " (", displayId, ")"));
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
