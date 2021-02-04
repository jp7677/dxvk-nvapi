#include "nvapi_d3d11_device.h"

namespace dxvk {
    bool NvapiD3d11Device::SetDepthBoundsTest(IUnknown* device, const u_int enable, const float minDepth, const float maxDepth) {
        static bool alreadyTested = false;
        if (!IsSupportedExtension(device, D3D11_VK_EXT_DEPTH_BOUNDS, alreadyTested))
            return false;

        Com<ID3D11VkExtContext> dxvkDeviceContext = GetDxvkDeviceContext(device);
        if (dxvkDeviceContext == nullptr)
            return false;

        dxvkDeviceContext->SetDepthBoundsTest(enable, minDepth, maxDepth);
        return true;
    }

    bool NvapiD3d11Device::BeginUAVOverlap(IUnknown* device) {
        static bool alreadyTested = false;
        if (!IsSupportedExtension(device, D3D11_VK_EXT_BARRIER_CONTROL, alreadyTested))
            return false;

        Com<ID3D11VkExtContext> dxvkDeviceContext = GetDxvkDeviceContext(device);
        if (dxvkDeviceContext == nullptr)
            return false;

        dxvkDeviceContext->SetBarrierControl(D3D11_VK_BARRIER_CONTROL_IGNORE_WRITE_AFTER_WRITE);
        return true;
    }

    bool NvapiD3d11Device::EndUAVOverlap(IUnknown* device) {
        static bool alreadyTested = false;
        if (!IsSupportedExtension(device, D3D11_VK_EXT_BARRIER_CONTROL, alreadyTested))
            return false;

        Com<ID3D11VkExtContext> dxvkDeviceContext = GetDxvkDeviceContext(device);
        if (dxvkDeviceContext == nullptr)
            return false;

        dxvkDeviceContext->SetBarrierControl(0);
        return true;
    }

    bool NvapiD3d11Device::MultiDrawInstancedIndirect(ID3D11DeviceContext* context, const NvU32 drawCount, ID3D11Buffer* buffer, const NvU32 alignedByteOffsetForArgs, const NvU32 alignedByteStrideForArgs) {
        static bool alreadyTested = false;
        if (!IsSupportedExtension(context, D3D11_VK_EXT_MULTI_DRAW_INDIRECT, alreadyTested))
            return false;

        Com<ID3D11VkExtContext> dxvkDeviceContext = GetDxvkDeviceContext(context);
        if (dxvkDeviceContext == nullptr)
            return false;

        dxvkDeviceContext->MultiDrawIndirect(drawCount, buffer, alignedByteOffsetForArgs, alignedByteStrideForArgs);
        return true;
    }

    bool NvapiD3d11Device::MultiDrawIndexedInstancedIndirect(ID3D11DeviceContext* context, const NvU32 drawCount, ID3D11Buffer* buffer, const NvU32 alignedByteOffsetForArgs, const NvU32 alignedByteStrideForArgs) {
        static bool alreadyTested = false;
        if (!IsSupportedExtension(context, D3D11_VK_EXT_MULTI_DRAW_INDIRECT, alreadyTested))
            return false;

        Com<ID3D11VkExtContext> dxvkDeviceContext = GetDxvkDeviceContext(context);
        if (dxvkDeviceContext == nullptr)
            return false;

        dxvkDeviceContext->MultiDrawIndexedIndirect(drawCount, buffer, alignedByteOffsetForArgs, alignedByteStrideForArgs);
        return true;
    }

    bool NvapiD3d11Device::IsSupportedExtension(IUnknown* device, const D3D11_VK_EXTENSION extension, bool& alreadyTested) {
        if (alreadyTested)
            return true;

        alreadyTested = true;

        Com<ID3D11VkExtDevice> dxvkDevice;
        if (FAILED(device->QueryInterface(IID_PPV_ARGS(&dxvkDevice))))
            return false;

        if (!dxvkDevice->GetExtensionSupport(extension))
            return false;

        return true;
    }

    Com<ID3D11VkExtContext> NvapiD3d11Device::GetDxvkDeviceContext(IUnknown* device) {
        Com<ID3D11Device> d3d11Device;
        if (FAILED(device->QueryInterface(IID_PPV_ARGS(&d3d11Device))))
            return nullptr;

        Com<ID3D11DeviceContext> d3d11DeviceContext;
        d3d11Device->GetImmediateContext(&d3d11DeviceContext);

        Com<ID3D11VkExtContext> dxvkDeviceContext;
        if (FAILED(d3d11DeviceContext->QueryInterface(IID_PPV_ARGS(&dxvkDeviceContext))))
            return nullptr;

        return dxvkDeviceContext;
    }
}
