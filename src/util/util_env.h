#pragma once

#include "../nvapi_private.h"

namespace dxvk::env {
    std::string getEnvVar(const std::string& name);

    std::string getExePath();

    std::string getExeName();
}
