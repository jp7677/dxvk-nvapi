extern "C" {
    using namespace dxvk;

    NvAPI_Status __cdecl NvAPI_Disp_GetHdrCapabilities(NvU32 displayId, NV_HDR_CAPABILITIES *pHdrCapabilities) {
        return NoImplementation(str::format("NvAPI_Disp_GetHdrCapabilities ", displayId));
    }

    NvAPI_Status __cdecl NvAPI_DISP_GetDisplayIdByDisplayName(const char *displayName, NvU32* displayId) {
        auto id = nvapiAdapterRegistry->GetOutputId(std::string(displayName));
        if (id == -1)
            return InvalidArgument(str::format("NvAPI_DISP_GetDisplayIdByDisplayName ", displayName));

        *displayId = id;

        return Ok(str::format("NvAPI_DISP_GetDisplayIdByDisplayName ", displayName));
    }
}
