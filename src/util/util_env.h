#pragma once

#include "../nvapi_private.h"

namespace dxvk::env {
    std::string getEnvVariable(const std::string& name);

    std::string getExecutablePath();

    std::string getExecutableName();
}
