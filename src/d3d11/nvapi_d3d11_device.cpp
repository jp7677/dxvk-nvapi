#include "nvapi_d3d11_device.h"

namespace dxvk {
    bool NvapiD3d11Device::SetDepthBoundsTest(IUnknown* deviceOrContext, const bool enable, const float minDepth, const float maxDepth) {
        ID3D11Device* d3d11Device;
        ID3D11DeviceContext* d3d11DeviceContext;
        if (m_alreadySetDepthBoundsTestTestedDevices.find(deviceOrContext) != m_alreadySetDepthBoundsTestTestedDevices.end()) {
            d3d11Device = dynamic_cast<ID3D11Device*>(deviceOrContext);
            d3d11Device->GetImmediateContext(&d3d11DeviceContext);
        } else if (SUCCEEDED(deviceOrContext->QueryInterface(IID_PPV_ARGS(&d3d11Device)))) {
            d3d11Device->Release();
            d3d11Device->GetImmediateContext(&d3d11DeviceContext);
            if (!IsSupportedExtension(d3d11DeviceContext, D3D11_VK_EXT_DEPTH_BOUNDS))
                return false;

            m_alreadySetDepthBoundsTestTestedDevices.emplace(d3d11Device);
        } else if (SUCCEEDED(deviceOrContext->QueryInterface(IID_PPV_ARGS(&d3d11DeviceContext)))) {
            d3d11DeviceContext->Release();
            if (!IsSupportedExtension(d3d11DeviceContext, D3D11_VK_EXT_DEPTH_BOUNDS))
                return false;
        } else
            return false;

        auto dxvkDeviceContext = GetDxvkDeviceContext(d3d11DeviceContext);
        if (dxvkDeviceContext == nullptr)
            return false;

        dxvkDeviceContext->SetDepthBoundsTest(enable, minDepth, maxDepth);
        return true;
    }

    bool NvapiD3d11Device::BeginUAVOverlap(IUnknown* deviceOrContext) {
        ID3D11Device* d3d11Device;
        ID3D11DeviceContext* d3d11DeviceContext;
        if (m_alreadyBarrierControlTestedDevices.find(deviceOrContext) != m_alreadyBarrierControlTestedDevices.end()) {
            d3d11Device = dynamic_cast<ID3D11Device*>(deviceOrContext);
            d3d11Device->GetImmediateContext(&d3d11DeviceContext);
        } else if (SUCCEEDED(deviceOrContext->QueryInterface(IID_PPV_ARGS(&d3d11Device)))) {
            d3d11Device->Release();
            d3d11Device->GetImmediateContext(&d3d11DeviceContext);
            if (!IsSupportedExtension(d3d11DeviceContext, D3D11_VK_EXT_BARRIER_CONTROL))
                return false;

            m_alreadyBarrierControlTestedDevices.emplace(d3d11Device);
        } else if (SUCCEEDED(deviceOrContext->QueryInterface(IID_PPV_ARGS(&d3d11DeviceContext)))) {
            d3d11DeviceContext->Release();
            if (!IsSupportedExtension(d3d11DeviceContext, D3D11_VK_EXT_BARRIER_CONTROL))
                return false;
        } else
            return false;

        auto dxvkDeviceContext = GetDxvkDeviceContext(d3d11DeviceContext);
        if (dxvkDeviceContext == nullptr)
            return false;

        dxvkDeviceContext->SetBarrierControl(D3D11_VK_BARRIER_CONTROL_IGNORE_WRITE_AFTER_WRITE);
        return true;
    }

    bool NvapiD3d11Device::EndUAVOverlap(IUnknown* deviceOrContext) {
        ID3D11Device* d3d11Device;
        ID3D11DeviceContext* d3d11DeviceContext;
        if (m_alreadyBarrierControlTestedDevices.find(deviceOrContext) != m_alreadyBarrierControlTestedDevices.end()) {
            d3d11Device = dynamic_cast<ID3D11Device*>(deviceOrContext);
            d3d11Device->GetImmediateContext(&d3d11DeviceContext);
        } else if (SUCCEEDED(deviceOrContext->QueryInterface(IID_PPV_ARGS(&d3d11Device)))) {
            d3d11Device->Release();
            d3d11Device->GetImmediateContext(&d3d11DeviceContext);
            if (!IsSupportedExtension(d3d11DeviceContext, D3D11_VK_EXT_BARRIER_CONTROL))
                return false;

            m_alreadyBarrierControlTestedDevices.emplace(d3d11Device);
        } else if (SUCCEEDED(deviceOrContext->QueryInterface(IID_PPV_ARGS(&d3d11DeviceContext)))) {
            d3d11DeviceContext->Release();
            if (!IsSupportedExtension(d3d11DeviceContext, D3D11_VK_EXT_BARRIER_CONTROL))
                return false;
        } else
            return false;

        auto dxvkDeviceContext = GetDxvkDeviceContext(d3d11DeviceContext);
        if (dxvkDeviceContext == nullptr)
            return false;

        dxvkDeviceContext->SetBarrierControl(0U);
        return true;
    }

    bool NvapiD3d11Device::MultiDrawInstancedIndirect(ID3D11DeviceContext* deviceContext, const NvU32 drawCount, ID3D11Buffer* buffer, const NvU32 alignedByteOffsetForArgs, const NvU32 alignedByteStrideForArgs) {
        if (!IsSupportedExtension(deviceContext, D3D11_VK_EXT_MULTI_DRAW_INDIRECT))
            return false;

        auto dxvkDeviceContext = GetDxvkDeviceContext(deviceContext);
        if (dxvkDeviceContext == nullptr)
            return false;

        dxvkDeviceContext->MultiDrawIndirect(drawCount, buffer, alignedByteOffsetForArgs, alignedByteStrideForArgs);
        return true;
    }

    bool NvapiD3d11Device::MultiDrawIndexedInstancedIndirect(ID3D11DeviceContext* deviceContext, const NvU32 drawCount, ID3D11Buffer* buffer, const NvU32 alignedByteOffsetForArgs, const NvU32 alignedByteStrideForArgs) {
        if (!IsSupportedExtension(deviceContext, D3D11_VK_EXT_MULTI_DRAW_INDIRECT))
            return false;

        auto dxvkDeviceContext = GetDxvkDeviceContext(deviceContext);
        if (dxvkDeviceContext == nullptr)
            return false;

        dxvkDeviceContext->MultiDrawIndexedIndirect(drawCount, buffer, alignedByteOffsetForArgs, alignedByteStrideForArgs);
        return true;
    }

    bool NvapiD3d11Device::IsSupportedExtension(ID3D11DeviceContext* deviceContext, const D3D11_VK_EXTENSION extension) {
        ID3D11Device* device;
        deviceContext->GetDevice(&device);

        Com<ID3D11VkExtDevice> dxvkDevice;
        if (FAILED(device->QueryInterface(IID_PPV_ARGS(&dxvkDevice))))
            return false;

        if (!dxvkDevice->GetExtensionSupport(extension))
            return false;

        return true;
    }

    Com<ID3D11VkExtContext> NvapiD3d11Device::GetDxvkDeviceContext(ID3D11DeviceContext* deviceContext) {
        Com<ID3D11VkExtContext> dxvkDeviceContext;
        if (FAILED(deviceContext->QueryInterface(IID_PPV_ARGS(&dxvkDeviceContext))))
            return nullptr;

        return dxvkDeviceContext;
    }
}
