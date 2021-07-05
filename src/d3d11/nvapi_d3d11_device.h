#pragma once

#include "../nvapi_private.h"
#include "../dxvk/dxvk_interfaces.h"
#include "../util/com_pointer.h"

namespace dxvk {
    class NvapiD3d11Device {

    public:
        static bool SetDepthBoundsTest(IUnknown* deviceOrContext, bool enable, float minDepth, float maxDepth);
        static bool BeginUAVOverlap(IUnknown* deviceOrContext);
        static bool EndUAVOverlap(IUnknown* deviceOrContext);
        static bool MultiDrawInstancedIndirect(ID3D11DeviceContext* deviceContext, NvU32 drawCount, ID3D11Buffer* buffer, NvU32 alignedByteOffsetForArgs, NvU32 alignedByteStrideForArgs);
        static bool MultiDrawIndexedInstancedIndirect(ID3D11DeviceContext* deviceContext, NvU32 drawCount, ID3D11Buffer* buffer, NvU32 alignedByteOffsetForArgs, NvU32 alignedByteStrideForArgs);

    private:
        [[nodiscard]] static bool IsSupportedExtension(ID3D11DeviceContext* deviceContext, D3D11_VK_EXTENSION extension);
        [[nodiscard]] static Com<ID3D11VkExtContext> GetDxvkDeviceContext(ID3D11DeviceContext* deviceContext);
    };
}
