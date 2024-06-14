#pragma once

#include "../nvapi_private.h"

namespace dxvk::env {
    std::string getEnvVariable(const std::string& name);

    std::string getExecutableName();

    std::string getCurrentDateTime();

    bool needsSucceededGpuQuery();

    std::optional<NV_GPU_ARCHITECTURE_ID> needsGpuArchitectureSpoofing(NV_GPU_ARCHITECTURE_ID architectureId, void* returnAddress);
}
