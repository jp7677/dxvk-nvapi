#include "nvapi_d3d11_device.h"

namespace dxvk {
    bool NvapiD3d11Device::SetDepthBoundsTest(IUnknown* device, u_int enable, float minDepth, float maxDepth) {
        static bool alreadyTested = false;
        if (!alreadyTested) {
            alreadyTested = true;

            Com<ID3D11VkExtDevice> dxvkDevice;
            if (FAILED(device->QueryInterface(IID_PPV_ARGS(&dxvkDevice))))
                return false;

            if (!dxvkDevice->GetExtensionSupport(D3D11_VK_EXT_DEPTH_BOUNDS))
                return false;
        }

        Com<ID3D11Device> d3d11Device;
        if (FAILED(device->QueryInterface(IID_PPV_ARGS(&d3d11Device))))
            return false;

        Com<ID3D11DeviceContext> d3d11DeviceContext;
        d3d11Device->GetImmediateContext(&d3d11DeviceContext);

        Com<ID3D11VkExtContext> dxvkDeviceContext;
        if (FAILED(d3d11DeviceContext->QueryInterface(IID_PPV_ARGS(&dxvkDeviceContext))))
            return false;

        dxvkDeviceContext->SetDepthBoundsTest(enable, minDepth, maxDepth);
        return true;
    }
}
