#pragma once

#include "../nvapi_private.h"
#include "util_string.h"

namespace dxvk {
    inline std::string fromLatencyMarkerType(const uint32_t type) {
        static const std::map<uint32_t, std::string> types{
            MAP_ENUM_VALUE(SIMULATION_START),
            MAP_ENUM_VALUE(SIMULATION_END),
            MAP_ENUM_VALUE(RENDERSUBMIT_START),
            MAP_ENUM_VALUE(RENDERSUBMIT_END),
            MAP_ENUM_VALUE(PRESENT_START),
            MAP_ENUM_VALUE(PRESENT_END),
            MAP_ENUM_VALUE(INPUT_SAMPLE),
            MAP_ENUM_VALUE(TRIGGER_FLASH),
            MAP_ENUM_VALUE(PC_LATENCY_PING),
            MAP_ENUM_VALUE(OUT_OF_BAND_RENDERSUBMIT_START),
            MAP_ENUM_VALUE(OUT_OF_BAND_RENDERSUBMIT_END),
            MAP_ENUM_VALUE(OUT_OF_BAND_PRESENT_START),
            MAP_ENUM_VALUE(OUT_OF_BAND_PRESENT_END),
        };

        auto it = types.find(type);
        return it != types.end() ? it->second : str::format("UNKNOWN_TYPE/", type);
    }
}
