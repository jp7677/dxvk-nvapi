#pragma once

#include "../nvapi_private.h"

namespace dxvk::log {

    void write(const std::string& severity, const std::string& message);

    inline void info(const std::string& message) {
        log::write("info", message);
    }

}
