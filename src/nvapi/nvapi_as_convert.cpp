#include "nvapi_as_convert.h"
#include "../interfaces/vkd3d-proton_interfaces.h"
#include "../util/util_log.h"

namespace dxvk {

    NvAPI_Status BuildFlagsToD3d12(D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS& d3d12Flags,
        NVAPI_D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS_EX nvapiFlags,
        bool supportsOmm) {
        /* All currently known flags */
        const auto allFlags = static_cast<NVAPI_D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS_EX>(
            NVAPI_D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE_EX | NVAPI_D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_COMPACTION_EX | NVAPI_D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE_EX | NVAPI_D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD_EX | NVAPI_D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_MINIMIZE_MEMORY_EX | NVAPI_D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE_EX | NVAPI_D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_OMM_UPDATE_EX | NVAPI_D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_DISABLE_OMMS_EX | NVAPI_D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_OMM_OPACITY_STATES_UPDATE_EX | NVAPI_D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_DATA_ACCESS_EX);

        // D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_DATA_ACCESS is only
        // defined in DirectX-Headers preview-tag releases (added in v1.717.0-preview).
        // The vendored submodule tracks stable tags only, so we don't pull from preview,
        // and we carry this provisional 0x100 (the value the preview header defines) until
        // the flag ships in a stable DirectX-Headers tag and the submodule bump pulls it in.
        static constexpr auto D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_DATA_ACCESS_PROVISIONAL =
            static_cast<D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS>(0x100);

        if (nvapiFlags & ~allFlags) {
            log::info(str::format("Unsupported NVAPI build flags: ", nvapiFlags));
            return NVAPI_INVALID_ARGUMENT;
        }

        d3d12Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;

        if (nvapiFlags & NVAPI_D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE_EX)
            d3d12Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
        if (nvapiFlags & NVAPI_D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_COMPACTION_EX)
            d3d12Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_COMPACTION;
        if (nvapiFlags & NVAPI_D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE_EX)
            d3d12Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
        if (nvapiFlags & NVAPI_D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD_EX)
            d3d12Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD;
        if (nvapiFlags & NVAPI_D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_MINIMIZE_MEMORY_EX)
            d3d12Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_MINIMIZE_MEMORY;
        if (nvapiFlags & NVAPI_D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE_EX)
            d3d12Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;
        if (nvapiFlags & NVAPI_D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_OMM_UPDATE_EX) {
            if (!supportsOmm)
                log::info("Ignoring NVAPI_D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_OMM_UPDATE_EX OMM not supported");
            else
                d3d12Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_OMM_UPDATE;
        }
        if (nvapiFlags & NVAPI_D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_DISABLE_OMMS_EX) {
            if (!supportsOmm)
                log::info("Ignoring NVAPI_D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_DISABLE_OMMS_EX OMM not supported");
            else
                d3d12Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_DISABLE_OMMS;
        }
        if (nvapiFlags & NVAPI_D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_OMM_OPACITY_STATES_UPDATE_EX) {
            /* OPACITY_STATES_UPDATE_EX is a narrower data-only variant of ALLOW_OMM_UPDATE_EX. D3D12 only has the wide
             * ALLOW_OMM_UPDATE, so widen rather than drop. */
            if (!supportsOmm)
                log::info("Ignoring NVAPI_D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_OMM_OPACITY_STATES_UPDATE_EX OMM not supported");
            else
                d3d12Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_OMM_UPDATE;
        }
        if (nvapiFlags & NVAPI_D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_DATA_ACCESS_EX)
            d3d12Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_DATA_ACCESS_PROVISIONAL;

        return NVAPI_OK;
    }

    NvAPI_Status GeomDescTypeToD3d12(D3D12_RAYTRACING_GEOMETRY_TYPE& d3d12Type,
        NVAPI_D3D12_RAYTRACING_GEOMETRY_TYPE_EX nvapiType) {
        switch (nvapiType) {
            case NVAPI_D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES_EX:
                d3d12Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
                return NVAPI_OK;
            case NVAPI_D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS_EX:
                d3d12Type = D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS;
                return NVAPI_OK;
            case NVAPI_D3D12_RAYTRACING_GEOMETRY_TYPE_OMM_TRIANGLES_EX:
                d3d12Type = D3D12_RAYTRACING_GEOMETRY_TYPE_OMM_TRIANGLES;
                return NVAPI_OK;
            case NVAPI_D3D12_RAYTRACING_GEOMETRY_TYPE_DMM_TRIANGLES_EX:
                log::info("Unsupported NVAPI_D3D12_RAYTRACING_GEOMETRY_TYPE_DMM_TRIANGLES_EX");
                return NVAPI_NOT_SUPPORTED;
            case NVAPI_D3D12_RAYTRACING_GEOMETRY_TYPE_SPHERES_EX:
                d3d12Type = D3D12_VK_EXT_RAYTRACING_GEOMETRY_TYPE_SPHERES_NVAPI;
                return NVAPI_OK;
            case NVAPI_D3D12_RAYTRACING_GEOMETRY_TYPE_LSS_EX:
                d3d12Type = D3D12_VK_EXT_RAYTRACING_GEOMETRY_TYPE_LSS_NVAPI;
                return NVAPI_OK;
            default:
                log::info(str::format("Unsupported NVAPI_D3D12_RAYTRACING_GEOMETRY_TYPE_ ", nvapiType));
                return NVAPI_INVALID_ARGUMENT;
        }
    }

    const NVAPI_D3D12_RAYTRACING_GEOMETRY_DESC_EX& GeomDesc(
        const NVAPI_D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS_EX& nvapiDesc, NvU32 index) {
        if (nvapiDesc.descsLayout == D3D12_ELEMENTS_LAYOUT_ARRAY_OF_POINTERS)
            return *nvapiDesc.ppGeometryDescs[index];
        return *reinterpret_cast<const NVAPI_D3D12_RAYTRACING_GEOMETRY_DESC_EX*>(
            reinterpret_cast<const uint8_t*>(nvapiDesc.pGeometryDescs) + nvapiDesc.geometryDescStrideInBytes * index);
    }

    NvAPI_Status OmmArrayBuildFlagsToD3d12(D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS& d3d12Flags,
        NVAPI_D3D12_RAYTRACING_OPACITY_MICROMAP_ARRAY_BUILD_FLAGS nvapiFlags) {
        const auto allFlags = static_cast<NVAPI_D3D12_RAYTRACING_OPACITY_MICROMAP_ARRAY_BUILD_FLAGS>(
            NVAPI_D3D12_RAYTRACING_OPACITY_MICROMAP_ARRAY_BUILD_FLAG_PREFER_FAST_TRACE | NVAPI_D3D12_RAYTRACING_OPACITY_MICROMAP_ARRAY_BUILD_FLAG_PREFER_FAST_BUILD);

        if (nvapiFlags & ~allFlags) {
            log::info(str::format("Unsupported NVAPI OMM-Array build flags: ", nvapiFlags));
            return NVAPI_INVALID_ARGUMENT;
        }

        d3d12Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;

        if (nvapiFlags & NVAPI_D3D12_RAYTRACING_OPACITY_MICROMAP_ARRAY_BUILD_FLAG_PREFER_FAST_TRACE)
            d3d12Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
        if (nvapiFlags & NVAPI_D3D12_RAYTRACING_OPACITY_MICROMAP_ARRAY_BUILD_FLAG_PREFER_FAST_BUILD)
            d3d12Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD;

        return NVAPI_OK;
    }

    NvAPI_Status OmmArrayInputsToD3d12(D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& d3d12Inputs,
        D3D12_RAYTRACING_OPACITY_MICROMAP_ARRAY_DESC& arrayDesc,
        const NVAPI_D3D12_BUILD_RAYTRACING_OPACITY_MICROMAP_ARRAY_INPUTS& nvapiInputs) {

        memset(&d3d12Inputs, 0x00, sizeof(d3d12Inputs));
        memset(&arrayDesc, 0x00, sizeof(arrayDesc));

        /* NVAPI_..._USAGE_COUNT and D3D12_..._HISTOGRAM_ENTRY are byte-compatible:
         * { UINT count, UINT subdivisionLevel, OMM_FORMAT format }. Field sizes
         * match (NvU32 == UINT == 4 bytes) and OMM_FORMAT enum values match
         * (OC1_2_STATE=0x1, OC1_4_STATE=0x2 in both). Cast directly. */
        arrayDesc.NumOmmHistogramEntries = nvapiInputs.numOMMUsageCounts;
        arrayDesc.pOmmHistogram = reinterpret_cast<const D3D12_RAYTRACING_OPACITY_MICROMAP_HISTOGRAM_ENTRY*>(
            nvapiInputs.pOMMUsageCounts);
        arrayDesc.InputBuffer = nvapiInputs.inputBuffer;
        arrayDesc.PerOmmDescs = nvapiInputs.perOMMDescs;

        d3d12Inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_OPACITY_MICROMAP_ARRAY;
        d3d12Inputs.NumDescs = 1;
        d3d12Inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
        d3d12Inputs.pOpacityMicromapArrayDesc = &arrayDesc;

        return OmmArrayBuildFlagsToD3d12(d3d12Inputs.Flags, nvapiInputs.flags);
    }

    NvAPI_Status NvapiAsConverter::Convert(D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& d3d12Desc,
        const NVAPI_D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS_EX& nvapiDesc,
        bool supportsOmm, bool supportsSpheres, bool supportsLinearSweptSpheres) {
        memset(&d3d12Desc, 0x00, sizeof(d3d12Desc));

        d3d12Desc.Type = nvapiDesc.type;
        d3d12Desc.NumDescs = nvapiDesc.numDescs;

        if (auto status = BuildFlagsToD3d12(d3d12Desc.Flags, nvapiDesc.flags, supportsOmm); status != NVAPI_OK)
            return status;

        if (nvapiDesc.type == D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL) {
            d3d12Desc.DescsLayout = nvapiDesc.descsLayout;
            d3d12Desc.InstanceDescs = nvapiDesc.instanceDescs;
        } else {
            const bool usesStride = nvapiDesc.descsLayout != D3D12_ELEMENTS_LAYOUT_ARRAY_OF_POINTERS;

            Reserve(nvapiDesc.numDescs);

            d3d12Desc.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
            d3d12Desc.pGeometryDescs = m_geometryDescs;

            for (auto i = 0U; i < nvapiDesc.numDescs; i++) {
                const NVAPI_D3D12_RAYTRACING_GEOMETRY_DESC_EX& nvapiGeomDesc = GeomDesc(nvapiDesc, i);
                D3D12_RAYTRACING_GEOMETRY_DESC& d3d12GeomDesc = m_geometryDescs[i];
                D3D12_RAYTRACING_GEOMETRY_OMM_LINKAGE_DESC& d3d12OpacityMicromapLinkageDesc =
                    m_opacityMicromapLinkageDescs[i];

                d3d12GeomDesc.Flags = nvapiGeomDesc.flags;
                if (auto status = GeomDescTypeToD3d12(d3d12GeomDesc.Type, nvapiGeomDesc.type); status != NVAPI_OK)
                    return status;

                switch (nvapiGeomDesc.type) {
                    case NVAPI_D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES_EX:
                        if (usesStride && nvapiDesc.geometryDescStrideInBytes < offsetof(NVAPI_D3D12_RAYTRACING_GEOMETRY_DESC_EX, triangles) + sizeof(D3D12_RAYTRACING_GEOMETRY_TRIANGLES_DESC)) {
                            log::info(str::format("geometryDescStrideInBytes ", nvapiDesc.geometryDescStrideInBytes,
                                " too small for TRIANGLES geometry."));
                            return NVAPI_INVALID_ARGUMENT;
                        }

                        d3d12GeomDesc.Triangles = nvapiGeomDesc.triangles;
                        break;

                    case NVAPI_D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS_EX:
                        if (usesStride && nvapiDesc.geometryDescStrideInBytes < offsetof(NVAPI_D3D12_RAYTRACING_GEOMETRY_DESC_EX, aabbs) + sizeof(D3D12_RAYTRACING_GEOMETRY_AABBS_DESC)) {
                            log::info(str::format("geometryDescStrideInBytes ", nvapiDesc.geometryDescStrideInBytes,
                                " too small for AABBS geometry."));
                            return NVAPI_INVALID_ARGUMENT;
                        }

                        d3d12GeomDesc.AABBs = nvapiGeomDesc.aabbs;
                        break;

                    case NVAPI_D3D12_RAYTRACING_GEOMETRY_TYPE_OMM_TRIANGLES_EX:
                        if (!supportsOmm) {
                            log::info("Triangles with OMM attachment passed to acceleration structure build when OMM is not supported");
                            return NVAPI_NOT_SUPPORTED;
                        }
                        if (usesStride && nvapiDesc.geometryDescStrideInBytes < offsetof(NVAPI_D3D12_RAYTRACING_GEOMETRY_DESC_EX, ommTriangles) + sizeof(NVAPI_D3D12_RAYTRACING_GEOMETRY_OMM_TRIANGLES_DESC)) {
                            log::info(str::format("geometryDescStrideInBytes ", nvapiDesc.geometryDescStrideInBytes,
                                " too small for OMM_TRIANGLES geometry."));
                            return NVAPI_INVALID_ARGUMENT;
                        }

                        d3d12GeomDesc.OmmTriangles.pTriangles = &nvapiGeomDesc.ommTriangles.triangles;
                        d3d12GeomDesc.OmmTriangles.pOmmLinkage = &d3d12OpacityMicromapLinkageDesc;

                        d3d12OpacityMicromapLinkageDesc.OpacityMicromapIndexBuffer =
                            nvapiGeomDesc.ommTriangles.ommAttachment.opacityMicromapIndexBuffer;
                        d3d12OpacityMicromapLinkageDesc.OpacityMicromapIndexFormat =
                            nvapiGeomDesc.ommTriangles.ommAttachment.opacityMicromapIndexFormat;
                        d3d12OpacityMicromapLinkageDesc.OpacityMicromapBaseLocation =
                            nvapiGeomDesc.ommTriangles.ommAttachment.opacityMicromapBaseLocation;
                        d3d12OpacityMicromapLinkageDesc.OpacityMicromapArray =
                            nvapiGeomDesc.ommTriangles.ommAttachment.opacityMicromapArray;

                        break;

                    case NVAPI_D3D12_RAYTRACING_GEOMETRY_TYPE_SPHERES_EX: {
                        if (!supportsSpheres) {
                            log::info("SPHERES geometry passed to acceleration structure build when not supported");
                            return NVAPI_NOT_SUPPORTED;
                        }
                        if (usesStride && nvapiDesc.geometryDescStrideInBytes < offsetof(NVAPI_D3D12_RAYTRACING_GEOMETRY_DESC_EX, spheres) + sizeof(NVAPI_D3D12_RAYTRACING_GEOMETRY_SPHERES_DESC)) {
                            log::info(str::format("geometryDescStrideInBytes ", nvapiDesc.geometryDescStrideInBytes,
                                " too small for SPHERES geometry."));
                            return NVAPI_INVALID_ARGUMENT;
                        }

                        // vkd3d-proton API expects the union to be a pointer directly to NVAPI-compatible desc struct
                        const auto* desc = &nvapiGeomDesc.spheres;
                        memcpy(&d3d12GeomDesc.Triangles, &desc, sizeof(void*));
                        break;
                    }

                    case NVAPI_D3D12_RAYTRACING_GEOMETRY_TYPE_LSS_EX: {
                        if (!supportsLinearSweptSpheres) {
                            log::info("LSS geometry passed to acceleration structure build when not supported");
                            return NVAPI_NOT_SUPPORTED;
                        }
                        if (usesStride && nvapiDesc.geometryDescStrideInBytes < offsetof(NVAPI_D3D12_RAYTRACING_GEOMETRY_DESC_EX, lss) + sizeof(NVAPI_D3D12_RAYTRACING_GEOMETRY_LSS_DESC)) {
                            log::info(str::format("geometryDescStrideInBytes ", nvapiDesc.geometryDescStrideInBytes,
                                " too small for LSS geometry."));
                            return NVAPI_INVALID_ARGUMENT;
                        }

                        // See above
                        const auto* desc = &nvapiGeomDesc.lss;
                        memcpy(&d3d12GeomDesc.Triangles, &desc, sizeof(void*));
                        break;
                    }

                    default:
                        log::info(str::format("Unsupported geometry type ", nvapiGeomDesc.type));
                        return NVAPI_INVALID_ARGUMENT;
                }
            }
        }

        return NVAPI_OK;
    }
}
