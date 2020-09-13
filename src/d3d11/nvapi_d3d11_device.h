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

    private:

        static bool IsSupportedExtension(IUnknown* device, D3D11_VK_EXTENSION extension, bool* alreadyTested);
        static Com<ID3D11VkExtContext> GetDxvkDeviceContext(IUnknown* device);

    };
}
