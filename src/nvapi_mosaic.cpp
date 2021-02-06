#include "nvapi_private.h"
#include "util/util_statuscode.h"

extern "C" {
    using namespace dxvk;

    NvAPI_Status __cdecl NvAPI_Mosaic_GetDisplayViewportsByResolution(NvU32 displayId, NvU32 srcWidth, NvU32 srcHeight, NV_RECT viewports[NV_MOSAIC_MAX_DISPLAYS], NvU8* bezelCorrected) {
        return MosaicNotActive("NvAPI_Mosaic_GetDisplayViewportsByResolution");
    }
}
