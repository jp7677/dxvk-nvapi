#include "util_log.h"

namespace dxvk::log {
    void write(const std::string& message) {
        std::cerr << message << std::endl;
    }
}
