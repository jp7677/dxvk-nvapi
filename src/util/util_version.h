#pragma once

#include "../nvapi_private.h"

namespace dxvk {

    // The Vulkan version number packing used by NVIDIA is slightly different
    // from the packing specified in the Vulkan specifications:
    // - The major version is a 10-bit integer packed into bits 31-22
    // - The minor version is an 8-bit integer packed into bits 21-14
    // - The patch version is an 8-bit integer packed into bits 13-6
    // See https://registry.khronos.org/vulkan/specs/1.3/html/vkspec.html#extendingvulkan-coreversions-versionnumbers

    inline uint32_t nvMakeVersion(const uint32_t major, const uint32_t minor, const uint32_t patch) {
        return (major << 22U) | (minor << 14U) | (patch << 6U);
    }

    inline uint32_t nvVersionMajor(const uint32_t version) {
        return (version >> 22U) & 0x3FFU;
    }

    inline uint32_t nvVersionMinor(const uint32_t version) {
        return (version >> 14U) & 0x0FFU;
    }

    inline uint32_t nvVersionPatch(const uint32_t version) {
        return (version >> 6U) & 0x0FFU;
    }
}
