#pragma once

#include "../nvapi_private.h"
#include "../dxvk/dxvk_interfaces.h"
#include "../util/com_pointer.h"

namespace dxvk {

    class NvapiD3d11Device {

    public:

        static bool SetDepthBoundsTest(IUnknown* device, u_int enable, float minDepth, float maxDepth);
        static bool BeginUAVOverlap(IUnknown* device);
        static bool EndUAVOverlap(IUnknown* device);
        static bool MultiDrawInstancedIndirect(ID3D11DeviceContext* context, NvU32 drawCount, ID3D11Buffer* buffer, NvU32 alignedByteOffsetForArgs, NvU32 alignedByteStrideForArgs);
        static bool MultiDrawIndexedInstancedIndirect(ID3D11DeviceContext* context, NvU32 drawCount, ID3D11Buffer* buffer, NvU32 alignedByteOffsetForArgs, NvU32 alignedByteStrideForArgs);

    private:

        [[nodiscard]] static bool IsSupportedExtension(IUnknown* device, D3D11_VK_EXTENSION extension, bool& alreadyTested);
        [[nodiscard]] static Com<ID3D11VkExtContext> GetDxvkDeviceContext(IUnknown* device);

    };
}
