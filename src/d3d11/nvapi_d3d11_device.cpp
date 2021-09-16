#include "nvapi_d3d11_device.h"

namespace dxvk {
    bool NvapiD3d11Device::SetDepthBoundsTest(IUnknown* deviceOrContext, const bool enable, const float minDepth, const float maxDepth) {
        auto sdbtDeviceContext = GetSdbtDeviceContext(deviceOrContext);
        if (sdbtDeviceContext == nullptr)
            return false;

        sdbtDeviceContext->SetDepthBoundsTest(enable, minDepth, maxDepth);
        return true;
    }

    bool NvapiD3d11Device::BeginUAVOverlap(IUnknown* deviceOrContext) {
        auto barrierControlDeviceContext = GetBarrierControlDeviceContext(deviceOrContext);
        if (barrierControlDeviceContext == nullptr)
            return false;

        barrierControlDeviceContext->SetBarrierControl(D3D11_VK_BARRIER_CONTROL_IGNORE_WRITE_AFTER_WRITE);
        return true;
    }

    bool NvapiD3d11Device::EndUAVOverlap(IUnknown* deviceOrContext) {
        auto barrierControlDeviceContext = GetBarrierControlDeviceContext(deviceOrContext);
        if (barrierControlDeviceContext == nullptr)
            return false;

        barrierControlDeviceContext->SetBarrierControl(0U);
        return true;
    }

    bool NvapiD3d11Device::MultiDrawInstancedIndirect(ID3D11DeviceContext* deviceContext, const NvU32 drawCount, ID3D11Buffer* buffer, const NvU32 alignedByteOffsetForArgs, const NvU32 alignedByteStrideForArgs) {
        auto multiDrawDeviceContext = GetMultiDrawDeviceContext(deviceContext);
        if (multiDrawDeviceContext == nullptr)
            return false;

        multiDrawDeviceContext->MultiDrawIndirect(drawCount, buffer, alignedByteOffsetForArgs, alignedByteStrideForArgs);
        return true;
    }

    bool NvapiD3d11Device::MultiDrawIndexedInstancedIndirect(ID3D11DeviceContext* deviceContext, const NvU32 drawCount, ID3D11Buffer* buffer, const NvU32 alignedByteOffsetForArgs, const NvU32 alignedByteStrideForArgs) {
        auto multiDrawDeviceContext = GetMultiDrawDeviceContext(deviceContext);
        if (multiDrawDeviceContext == nullptr)
            return false;

        multiDrawDeviceContext->MultiDrawIndexedIndirect(drawCount, buffer, alignedByteOffsetForArgs, alignedByteStrideForArgs);
        return true;
    }

    Com<ID3D11VkExtContext> NvapiD3d11Device::GetSdbtDeviceContext(IUnknown* deviceOrContext) {
        auto it = m_depthBoundsDeviceOrContextMap.find(deviceOrContext);
        if (it != m_depthBoundsDeviceOrContextMap.end())
            return it->second;

        auto deviceContextExt = GetDeviceContextExt(deviceOrContext, D3D11_VK_EXT_DEPTH_BOUNDS);
        if (deviceContextExt != nullptr)
            m_depthBoundsDeviceOrContextMap.emplace(deviceOrContext, deviceContextExt.ptr());

        return deviceContextExt;
    }

    Com<ID3D11VkExtContext> NvapiD3d11Device::GetBarrierControlDeviceContext(IUnknown* deviceOrContext) {
        auto it = m_barrierControlDeviceOrContextMap.find(deviceOrContext);
        if (it != m_barrierControlDeviceOrContextMap.end())
            return it->second;

        auto deviceContextExt = GetDeviceContextExt(deviceOrContext, D3D11_VK_EXT_BARRIER_CONTROL);
        if (deviceContextExt != nullptr)
            m_barrierControlDeviceOrContextMap.emplace(deviceOrContext, deviceContextExt.ptr());

        return deviceContextExt;
    }

    Com<ID3D11VkExtContext> NvapiD3d11Device::GetMultiDrawDeviceContext(ID3D11DeviceContext* deviceContext) {
        auto it = m_multiDrawIndirectContextMap.find(deviceContext);
        if (it != m_multiDrawIndirectContextMap.end())
            return it->second;

        auto deviceContextExt = GetDeviceContextExt(deviceContext, D3D11_VK_EXT_MULTI_DRAW_INDIRECT);
        if (deviceContextExt != nullptr)
            m_multiDrawIndirectContextMap.emplace(deviceContext, deviceContextExt.ptr());

        return deviceContextExt;
    }

    Com<ID3D11VkExtContext> NvapiD3d11Device::GetDeviceContextExt(IUnknown* deviceOrContext, D3D11_VK_EXTENSION extension) {
        Com<ID3D11Device> device;
        Com<ID3D11DeviceContext> deviceContext;
        if (SUCCEEDED(deviceOrContext->QueryInterface(IID_PPV_ARGS(&device))))
            device->GetImmediateContext(&deviceContext);
        else if (SUCCEEDED(deviceOrContext->QueryInterface(IID_PPV_ARGS(&deviceContext))))
            deviceContext->GetDevice(&device);
        else
            return nullptr;

        return GetDeviceContextExt(device.ptr(), deviceContext.ptr(), extension);
    }

    Com<ID3D11VkExtContext> NvapiD3d11Device::GetDeviceContextExt(ID3D11DeviceContext* deviceContext, D3D11_VK_EXTENSION extension) {
        Com<ID3D11Device> device;
        deviceContext->GetDevice(&device);

        return GetDeviceContextExt(device.ptr(), deviceContext, extension);
    }

    Com<ID3D11VkExtContext> NvapiD3d11Device::GetDeviceContextExt(ID3D11Device* device, ID3D11DeviceContext* deviceContext, D3D11_VK_EXTENSION extension) {
        Com<ID3D11VkExtDevice> deviceExt;
        if (FAILED(device->QueryInterface(IID_PPV_ARGS(&deviceExt))))
            return nullptr;

        if (!deviceExt->GetExtensionSupport(extension))
            return nullptr;

        Com<ID3D11VkExtContext> deviceContextExt;
        if (FAILED(deviceContext->QueryInterface(IID_PPV_ARGS(&deviceContextExt))))
            return nullptr;

        return deviceContextExt;
    }
}
