#pragma once

#include "../nvapi_private.h"
#include "../dxvk/dxvk_interfaces.h"
#include "../util/com_pointer.h"

namespace dxvk {

    class NvapiD3d11Device {

    public:

        static bool SetDepthBoundsTest(IUnknown* device, u_int enable, float minDepth, float maxDepth);

    };
}
