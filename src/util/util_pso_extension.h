#pragma once

#include "../nvapi_private.h"

namespace dxvk {
    inline std::string fromPsoExtension(const uint32_t code) {
        static const std::map<uint32_t, std::string> codes{
            {0, "NV_PSO_RASTER_EXTENSION"},
            {1, "NV_PSO_REQUEST_FASTGS_EXTENSION"},
            {2, "NV_PSO_GEOMETRY_SHADER_EXTENSION"},
            {3, "NV_PSO_ENABLE_DEPTH_BOUND_TEST_EXTENSION"},
            {4, "NV_PSO_EXPLICIT_FASTGS_EXTENSION"},
            {5, "NV_PSO_SET_SHADER_EXTENSION_SLOT_AND_SPACE"},
            {6, "NV_PSO_VERTEX_SHADER_EXTENSION"},
            {7, "NV_PSO_DOMAIN_SHADER_EXTENSION"},
            {9, "NV_PSO_HULL_SHADER_EXTENSION"},
        };

        auto it = codes.find(code);
        return it != codes.end() ? it->second : "UNKNOWN_SHADER_EXTENSION";
    }
}
