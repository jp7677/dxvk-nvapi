#pragma once

#include "../nvapi_private.h"
#include "util_string.h"

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
            {52, "NV_EXTN_OP_VPRS_EVAL_ATTRIB_SNAPPED"},
            {67, "NV_EXTN_OP_HIT_OBJECT_TRACE_RAY"},
            {68, "NV_EXTN_OP_HIT_OBJECT_MAKE_HIT"},
            {69, "NV_EXTN_OP_HIT_OBJECT_MAKE_HIT_WITH_RECORD_INDEX"},
            {70, "NV_EXTN_OP_HIT_OBJECT_MAKE_MISS"},
            {71, "NV_EXTN_OP_HIT_OBJECT_REORDER_THREAD"},
            {72, "NV_EXTN_OP_HIT_OBJECT_INVOKE"},
            {73, "NV_EXTN_OP_HIT_OBJECT_IS_MISS"},
            {74, "NV_EXTN_OP_HIT_OBJECT_GET_INSTANCE_ID"},
            {75, "NV_EXTN_OP_HIT_OBJECT_GET_INSTANCE_INDEX"},
            {76, "NV_EXTN_OP_HIT_OBJECT_GET_PRIMITIVE_INDEX"},
            {77, "NV_EXTN_OP_HIT_OBJECT_GET_GEOMETRY_INDEX"},
            {78, "NV_EXTN_OP_HIT_OBJECT_GET_HIT_KIND"},
            {79, "NV_EXTN_OP_HIT_OBJECT_GET_RAY_DESC"},
            {80, "NV_EXTN_OP_HIT_OBJECT_GET_ATTRIBUTES"},
            {81, "NV_EXTN_OP_HIT_OBJECT_GET_SHADER_TABLE_INDEX"},
            {82, "NV_EXTN_OP_HIT_OBJECT_LOAD_LOCAL_ROOT_TABLE_CONSTANT"},
            {83, "NV_EXTN_OP_HIT_OBJECT_IS_HIT"},
            {84, "NV_EXTN_OP_HIT_OBJECT_IS_NOP"},
            {85, "NV_EXTN_OP_HIT_OBJECT_MAKE_NOP"},
            {86, "NV_EXTN_OP_RT_TRIANGLE_OBJECT_POSITIONS"},
            {87, "NV_EXTN_OP_RT_MICRO_TRIANGLE_OBJECT_POSITIONS"},
            {88, "NV_EXTN_OP_RT_MICRO_TRIANGLE_BARYCENTRICS"},
            {89, "NV_EXTN_OP_RT_IS_MICRO_TRIANGLE_HIT"},
            {90, "NV_EXTN_OP_RT_IS_BACK_FACING"},
            {91, "NV_EXTN_OP_RT_MICRO_VERTEX_OBJECT_POSITION"},
            {92, "NV_EXTN_OP_RT_MICRO_VERTEX_BARYCENTRICS"}};

        auto it = codes.find(code);
        return it != codes.end() ? it->second : str::format("UNKNOWN_EXTN_OP/", code);
    }
}
