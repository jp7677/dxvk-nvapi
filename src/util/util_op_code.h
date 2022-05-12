#pragma once

namespace dxvk {
    inline std::string fromCode(const uint32_t code) {
        static const std::map<uint32_t, std::string> codes{
            {1, "NV_EXTN_OP_SHFL"},
            {2, "NV_EXTN_OP_SHFL_UP"},
            {3, "NV_EXTN_OP_SHFL_DOWN"},
            {4, "NV_EXTN_OP_SHFL_XOR"},
            {5, "NV_EXTN_OP_VOTE_ALL"},
            {6, "NV_EXTN_OP_VOTE_ANY"},
            {7, "NV_EXTN_OP_VOTE_BALLOT"},
            {8, "NV_EXTN_OP_GET_LANE_ID"},
            {12, "NV_EXTN_OP_FP16_ATOMIC"},
            {13, "NV_EXTN_OP_FP32_ATOMIC"},
            {19, "NV_EXTN_OP_GET_SPECIAL"},
            {20, "NV_EXTN_OP_UINT64_ATOMIC"},
            {21, "NV_EXTN_OP_MATCH_ANY"},
            {28, "NV_EXTN_OP_FOOTPRINT"},
            {29, "NV_EXTN_OP_FOOTPRINT_BIAS"},
            {30, "NV_EXTN_OP_GET_SHADING_RATE"},
            {31, "NV_EXTN_OP_FOOTPRINT_LEVEL"},
            {32, "NV_EXTN_OP_FOOTPRINT_GRAD"},
            {33, "NV_EXTN_OP_SHFL_GENERIC"},
            {51, "NV_EXTN_OP_VPRS_EVAL_ATTRIB_AT_SAMPLE"},
            {52, "NV_EXTN_OP_VPRS_EVAL_ATTRIB_SNAPPED"}};

        auto it = codes.find(code);
        return it != codes.end() ? it->second : "UNKNOWN_EXTN_OP";
    }
}
