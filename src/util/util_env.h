#pragma once

#include "../nvapi_private.h"

namespace dxvk::env {
    std::string getEnvVariable(const std::string& name);

    std::string getExecutableName();

    std::string getCurrentDateTime();

    bool needsSucceededGpuQuery();

    bool needsUnsupportedLowLatencyDevice();

    bool needsLowLatencyDevice();

    bool isD3d12NvShaderExtnEnabled();

    bool isUnrealEngine();

    std::optional<NV_GPU_ARCHITECTURE_ID> needsGpuArchitectureSpoofing(NV_GPU_ARCHITECTURE_ID architectureId, void* returnAddress);
}
