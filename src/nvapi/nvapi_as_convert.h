#pragma once

#include "../nvapi_private.h"

namespace dxvk {
    [[nodiscard]] NvAPI_Status OmmArrayInputsToD3d12(D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& d3d12Inputs,
        D3D12_RAYTRACING_OPACITY_MICROMAP_ARRAY_DESC& arrayDesc,
        const NVAPI_D3D12_BUILD_RAYTRACING_OPACITY_MICROMAP_ARRAY_INPUTS& nvapiInputs);

    class NvapiAsConverter {
      public:
        NvapiAsConverter() {
            m_reserved = StackReserve;
            m_geometryDescs = m_geometryDescsStack;
            m_opacityMicromapLinkageDescs = m_opacityMicromapLinkageDescsStack;
        }
        ~NvapiAsConverter() {
            if (m_geometryDescs != m_geometryDescsStack)
                delete[] m_geometryDescs;
            if (m_opacityMicromapLinkageDescs != m_opacityMicromapLinkageDescsStack)
                delete[] m_opacityMicromapLinkageDescs;
        }
        NvapiAsConverter(const NvapiAsConverter&) = delete;
        NvapiAsConverter& operator=(const NvapiAsConverter&) = delete;

        [[nodiscard]] NvAPI_Status Convert(D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& d3d12Desc,
            const NVAPI_D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS_EX& nvapiDesc,
            bool supportsOmm);

      private:
        static constexpr size_t StackReserve = 16;

        // Convert is destructive.
        void Reserve(size_t reserveSize) {
            if (reserveSize > m_reserved) {
                if (m_geometryDescs != m_geometryDescsStack)
                    delete[] m_geometryDescs;
                if (m_opacityMicromapLinkageDescs != m_opacityMicromapLinkageDescsStack)
                    delete[] m_opacityMicromapLinkageDescs;
                m_geometryDescs = new D3D12_RAYTRACING_GEOMETRY_DESC[reserveSize];
                m_opacityMicromapLinkageDescs = new D3D12_RAYTRACING_GEOMETRY_OMM_LINKAGE_DESC[reserveSize];
                m_reserved = reserveSize;
            }
        }

        size_t m_reserved;
        D3D12_RAYTRACING_GEOMETRY_DESC m_geometryDescsStack[StackReserve];
        D3D12_RAYTRACING_GEOMETRY_OMM_LINKAGE_DESC m_opacityMicromapLinkageDescsStack[StackReserve];
        D3D12_RAYTRACING_GEOMETRY_DESC* m_geometryDescs;
        D3D12_RAYTRACING_GEOMETRY_OMM_LINKAGE_DESC* m_opacityMicromapLinkageDescs;
    };
}
