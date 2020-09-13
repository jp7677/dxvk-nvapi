extern "C" {
    using namespace dxvk;

    NvAPI_Status __cdecl NvAPI_Disp_GetHdrCapabilities(NvU32 displayId, NV_HDR_CAPABILITIES *pHdrCapabilities) {
        constexpr auto n = "NvAPI_Disp_GetHdrCapabilities";

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (pHdrCapabilities == nullptr)
            return InvalidArgument(n);

        if (pHdrCapabilities->version != NV_HDR_CAPABILITIES_VER1 && pHdrCapabilities->version != NV_HDR_CAPABILITIES_VER2)
            return IncompatibleStructVersion(n);

        pHdrCapabilities->isST2084EotfSupported = false;
        pHdrCapabilities->isTraditionalHdrGammaSupported = false;
        pHdrCapabilities->isEdrSupported = false;
        pHdrCapabilities->driverExpandDefaultHdrParameters = false;
        pHdrCapabilities->isTraditionalSdrGammaSupported = false;
        // pHdrCapabilities->display_data

        if (pHdrCapabilities->version == NV_HDR_CAPABILITIES_VER2)
            pHdrCapabilities->isDolbyVisionSupported = false;
            // pHdrCapabilities->dv_static_metadata

        return Ok(str::format(n, " ", displayId));
    }

    NvAPI_Status __cdecl NvAPI_DISP_GetDisplayIdByDisplayName(const char *displayName, NvU32* displayId) {
        constexpr auto n = "NvAPI_DISP_GetDisplayIdByDisplayName";

        if (nvapiAdapterRegistry == nullptr)
            return ApiNotInitialized(n);

        if (displayName == nullptr || displayId == nullptr)
            return InvalidArgument(n);

        auto id = nvapiAdapterRegistry->GetOutputId(std::string(displayName));
        if (id == -1)
            return InvalidArgument(str::format(n, " ", displayName));

        *displayId = id;

        return Ok(str::format(n, " ", displayName));
    }
}
