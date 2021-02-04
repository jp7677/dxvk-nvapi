#pragma once

#include "../nvapi_private.h"
#include "../dxvk/dxvk_interfaces.h"
#include "../util/com_pointer.h"

namespace dxvk {

    class NvapiD3d11Device {

    public:

        static bool SetDepthBoundsTest(IUnknown* device, const u_int enable, const float minDepth, const float maxDepth);
        static bool BeginUAVOverlap(IUnknown* device);
        static bool EndUAVOverlap(IUnknown* device);
        static bool MultiDrawInstancedIndirect(ID3D11DeviceContext* context, const NvU32 drawCount, ID3D11Buffer* buffer, const NvU32 alignedByteOffsetForArgs, const NvU32 alignedByteStrideForArgs);
        static bool MultiDrawIndexedInstancedIndirect(ID3D11DeviceContext* context, const NvU32 drawCount, ID3D11Buffer* buffer, const NvU32 alignedByteOffsetForArgs, const NvU32 alignedByteStrideForArgs);

    private:

        static bool IsSupportedExtension(IUnknown* device, const D3D11_VK_EXTENSION extension, bool& alreadyTested);
        static Com<ID3D11VkExtContext> GetDxvkDeviceContext(IUnknown* device);

    };
}
