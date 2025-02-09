#pragma once

#include "../nvapi_private.h"
#include "./util_string.h"
#include "NvApiDriverSettings.h"

namespace dxvk::drs {
    static const std::map<std::string_view, NvU32, str::CaseInsensitiveCompare<std::string_view>> settings = {
#define SETTING_ID(x) x##_ID
#define ENTRY(x) {#x, SETTING_ID(x)}
        ENTRY(NGX_DLAA_OVERRIDE),
        ENTRY(NGX_DLSSG_MULTI_FRAME_COUNT),
        ENTRY(NGX_DLSS_FG_OVERRIDE),
        ENTRY(NGX_DLSS_RR_MODE),
        ENTRY(NGX_DLSS_OVERRIDE_OPTIMAL_SETTINGS),
        ENTRY(NGX_DLSS_RR_OVERRIDE),
        ENTRY(NGX_DLSS_RR_OVERRIDE_RENDER_PRESET_SELECTION),
        ENTRY(NGX_DLSS_SR_MODE),
        ENTRY(NGX_DLSS_SR_OVERRIDE),
        ENTRY(NGX_DLSS_SR_OVERRIDE_RENDER_PRESET_SELECTION),
#undef ENTRY
#undef SETTING_ID
    };

#define CAT2_(a, b) a##b
#define CAT2(a, b) CAT2_(a, b)
#define CAT3_(a, b, c) a##b##c
#define CAT3(a, b, c) CAT3_(a, b, c)
#define CAT5_(a, b, c, d, e) a##b##c##d##e
#define CAT5(a, b, c, d, e) CAT5_(a, b, c, d, e)
#define ENTRY1_(x, prefix) {#x, CAT3(prefix, _, x)}
#define ENTRY1(x) ENTRY1_(x, SETTING)
#define ENTRY2_(x, prefix) {#x, CAT5(prefix, _, prefix, _, x)}
#define ENTRY2(x) ENTRY2_(x, SETTING)
#define ID() CAT2(SETTING, _ID)

    static const std::unordered_map<NvU32, std::map<std::string_view, NvU32, str::CaseInsensitiveCompare<std::string_view>>> values = {
#define SETTING NGX_DLAA_OVERRIDE
        {ID(), {
                   ENTRY1(DLAA_DEFAULT),
                   ENTRY1(DLAA_ON),
                   ENTRY1(DEFAULT),
               }},
#undef SETTING
#define SETTING NGX_DLSSG_MULTI_FRAME_COUNT
        {ID(), {
                   ENTRY1(MIN),
                   ENTRY1(MAX),
                   ENTRY1(DEFAULT),
               }},
#undef SETTING
#define SETTING NGX_DLSS_FG_OVERRIDE
        {ID(), {
                   ENTRY1(OFF),
                   ENTRY1(ON),
                   ENTRY1(DEFAULT),
               }},
#undef SETTING
#define SETTING NGX_DLSS_OVERRIDE_OPTIMAL_SETTINGS
        {ID(), {
                   ENTRY2(NONE),
                   ENTRY2(PERF_TO_9X),
                   ENTRY1(DEFAULT),
               }},
#undef SETTING
#define SETTING NGX_DLSS_RR_MODE
        {ID(), {
                   ENTRY2(PERFORMANCE),
                   ENTRY2(BALANCED),
                   ENTRY2(QUALITY),
                   ENTRY2(SNIPPET_CONTROLLED),
                   ENTRY2(DLAA),
                   ENTRY2(ULTRA_PERFORMANCE),
                   ENTRY2(CUSTOM),
                   ENTRY1(DEFAULT),
               }},
#undef SETTING
#define SETTING NGX_DLSS_RR_OVERRIDE
        {ID(), {
                   ENTRY1(OFF),
                   ENTRY1(ON),
                   ENTRY1(DEFAULT),
               }},
#undef SETTING
#define SETTING NGX_DLSS_RR_OVERRIDE_RENDER_PRESET_SELECTION
        {ID(), {
                   ENTRY1(OFF),
                   ENTRY1(RENDER_PRESET_A),
                   ENTRY1(RENDER_PRESET_B),
                   ENTRY1(RENDER_PRESET_C),
                   ENTRY1(RENDER_PRESET_D),
                   ENTRY1(RENDER_PRESET_E),
                   ENTRY1(RENDER_PRESET_F),
                   ENTRY1(RENDER_PRESET_G),
                   ENTRY1(RENDER_PRESET_H),
                   ENTRY1(RENDER_PRESET_I),
                   ENTRY1(RENDER_PRESET_J),
                   ENTRY1(RENDER_PRESET_K),
                   ENTRY1(RENDER_PRESET_L),
                   ENTRY1(RENDER_PRESET_M),
                   ENTRY1(RENDER_PRESET_N),
                   ENTRY1(RENDER_PRESET_O),
                   ENTRY1(RENDER_PRESET_Latest),
                   ENTRY1(DEFAULT),
               }},
#undef SETTING
#define SETTING NGX_DLSS_SR_MODE
        {ID(), {
                   ENTRY2(PERFORMANCE),
                   ENTRY2(BALANCED),
                   ENTRY2(QUALITY),
                   ENTRY2(SNIPPET_CONTROLLED),
                   ENTRY2(DLAA),
                   ENTRY2(ULTRA_PERFORMANCE),
                   ENTRY2(CUSTOM),
                   ENTRY1(DEFAULT),
               }},
#undef SETTING
#define SETTING NGX_DLSS_SR_OVERRIDE
        {ID(), {
                   ENTRY1(OFF),
                   ENTRY1(ON),
                   ENTRY1(DEFAULT),
               }},
#undef SETTING
#define SETTING NGX_DLSS_SR_OVERRIDE_RENDER_PRESET_SELECTION
        {ID(), {
                   ENTRY1(OFF),
                   ENTRY1(RENDER_PRESET_A),
                   ENTRY1(RENDER_PRESET_B),
                   ENTRY1(RENDER_PRESET_C),
                   ENTRY1(RENDER_PRESET_D),
                   ENTRY1(RENDER_PRESET_E),
                   ENTRY1(RENDER_PRESET_F),
                   ENTRY1(RENDER_PRESET_G),
                   ENTRY1(RENDER_PRESET_H),
                   ENTRY1(RENDER_PRESET_I),
                   ENTRY1(RENDER_PRESET_J),
                   ENTRY1(RENDER_PRESET_K),
                   ENTRY1(RENDER_PRESET_L),
                   ENTRY1(RENDER_PRESET_M),
                   ENTRY1(RENDER_PRESET_N),
                   ENTRY1(RENDER_PRESET_O),
                   ENTRY1(RENDER_PRESET_Latest),
                   ENTRY1(DEFAULT),
               }},
#undef SETTING
    };

#undef CAT2_
#undef CAT2
#undef CAT3_
#undef CAT3
#undef CAT5_
#undef CAT5
#undef ENTRY1_
#undef ENTRY1
#undef ENTRY2_
#undef ENTRY2
#undef ID

    bool parsedrssetting(const std::string_view& str, NvU32& setting);

    bool parsedrsdwordvalue(NvU32 key, const std::string_view& str, NvU32& value);

    std::unordered_map<NvU32, NvU32> parsedrsdwordsettings(const std::string& str);

    std::unordered_map<NvU32, NvU32> enrichwithenv(std::unordered_map<NvU32, NvU32> map, const char* prefix);
}
