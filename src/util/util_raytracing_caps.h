#pragma once

#include "../nvapi_private.h"

namespace dxvk {
    inline std::string fromRaytracingCaps(const uint32_t code) {
        static const std::map<int32_t, std::string> codes{
            {0, "NVAPI_D3D12_RAYTRACING_CAPS_TYPE_THREAD_REORDERING"},
            {1, "NVAPI_D3D12_RAYTRACING_CAPS_TYPE_OPACITY_MICROMAP"},
            {2, "NVAPI_D3D12_RAYTRACING_CAPS_TYPE_DISPLACEMENT_MICROMAP"},
            {3, "NVAPI_D3D12_RAYTRACING_CAPS_TYPE_CLUSTER_OPERATIONS"},
            {4, "NVAPI_D3D12_RAYTRACING_CAPS_TYPE_PARTITIONED_TLAS"},
            {5, "NVAPI_D3D12_RAYTRACING_CAPS_TYPE_SPHERES"},
            {6, "NVAPI_D3D12_RAYTRACING_CAPS_TYPE_LINEAR_SWEPT_SPHERES"},
            {-1, "NVAPI_D3D12_RAYTRACING_CAPS_TYPE_INVALID"},
        };

        auto it = codes.find(code);
        return it != codes.end() ? it->second : "UNKNOWN_NVAPI_D3D12_RAYTRACING_CAPS_TYPE";
    }
}
