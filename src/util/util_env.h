#pragma once

#include "../nvapi_private.h"

namespace dxvk::env {
    std::string getEnvVariable(const std::string& name);

    std::string getExecutableName();

    std::string getCurrentDateTime();

    bool needsAmpereSpoofing(NV_GPU_ARCHITECTURE_ID architectureId, void* pReturnAddress);

    bool needsPascalSpoofing(NV_GPU_ARCHITECTURE_ID architectureId);

    bool needsSucceededGpuQuery();
}
