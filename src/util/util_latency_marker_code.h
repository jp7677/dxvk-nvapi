#pragma once

#include "../nvapi_private.h"
#include "util_string.h"

namespace dxvk {
    inline std::string fromLatencyMarkerType(const uint32_t type) {
        static const std::map<uint32_t, std::string> types{
            {0, "SIMULATION_START"},
            {1, "SIMULATION_END"},
            {2, "RENDERSUBMIT_START"},
            {3, "RENDERSUBMIT_END"},
            {4, "PRESENT_START"},
            {5, "PRESENT_END"},
            {6, "INPUT_SAMPLE"},
            {7, "TRIGGER_FLASH"},
            {8, "PC_LATENCY_PING"},
            {9, "OUT_OF_BAND_RENDERSUBMIT_START"},
            {10, "OUT_OF_BAND_RENDERSUBMIT_END"},
            {11, "OUT_OF_BAND_PRESENT_START"},
            {12, "OUT_OF_BAND_PRESENT_END"},
        };

        auto it = types.find(type);
        return it != types.end() ? it->second : str::format("UNKNOWN_TYPE/", type);
    }
}
