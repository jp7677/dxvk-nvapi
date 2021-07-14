#include "nvapi_d3d11_device.h"

namespace dxvk {
    bool NvapiD3d11Device::SetDepthBoundsTest(IUnknown* deviceOrContext, const bool enable, const float minDepth, const float maxDepth) {
        auto it = m_depthBoundsDeviceOrContextMap.find(deviceOrContext);
        if (it != m_depthBoundsDeviceOrContextMap.end()) {
            it->second->SetDepthBoundsTest(enable, minDepth, maxDepth); // We completely ignore any reference counting here, not nice but works due to the nature of the context
            return true;
        }

        auto dxvkDeviceContext = GetDxvkDeviceContext(deviceOrContext, D3D11_VK_EXT_DEPTH_BOUNDS);
        if (dxvkDeviceContext == nullptr)
            return false;

        m_depthBoundsDeviceOrContextMap.emplace(deviceOrContext, dxvkDeviceContext.ptr());
        dxvkDeviceContext->SetDepthBoundsTest(enable, minDepth, maxDepth);
        return true;
    }

    bool NvapiD3d11Device::BeginUAVOverlap(IUnknown* deviceOrContext) {
        auto it = m_barrierControlDeviceOrContextMap.find(deviceOrContext);
        if (it != m_barrierControlDeviceOrContextMap.end()) {
            it->second->SetBarrierControl(D3D11_VK_BARRIER_CONTROL_IGNORE_WRITE_AFTER_WRITE); // See note above about reference counting
            return true;
        }

        auto dxvkDeviceContext = GetDxvkDeviceContext(deviceOrContext, D3D11_VK_EXT_BARRIER_CONTROL);
        if (dxvkDeviceContext == nullptr)
            return false;

        m_barrierControlDeviceOrContextMap.emplace(deviceOrContext, dxvkDeviceContext.ptr());
        dxvkDeviceContext->SetBarrierControl(D3D11_VK_BARRIER_CONTROL_IGNORE_WRITE_AFTER_WRITE);
        return true;
    }

    bool NvapiD3d11Device::EndUAVOverlap(IUnknown* deviceOrContext) {
        auto it = m_barrierControlDeviceOrContextMap.find(deviceOrContext);
        if (it != m_barrierControlDeviceOrContextMap.end()) {
            it->second->SetBarrierControl(0U); // See note above about reference counting
            return true;
        }

        auto dxvkDeviceContext = GetDxvkDeviceContext(deviceOrContext, D3D11_VK_EXT_BARRIER_CONTROL);
        if (dxvkDeviceContext == nullptr)
            return false;

        m_barrierControlDeviceOrContextMap.emplace(deviceOrContext, dxvkDeviceContext.ptr());
        dxvkDeviceContext->SetBarrierControl(0U);
        return true;
    }

    bool NvapiD3d11Device::MultiDrawInstancedIndirect(ID3D11DeviceContext* deviceContext, const NvU32 drawCount, ID3D11Buffer* buffer, const NvU32 alignedByteOffsetForArgs, const NvU32 alignedByteStrideForArgs) {
        auto it = m_multiDrawIndirectContextMap.find(deviceContext);
        if (it != m_multiDrawIndirectContextMap.end()) {
            it->second->MultiDrawIndirect(drawCount, buffer, alignedByteOffsetForArgs, alignedByteStrideForArgs); // See note above about reference counting
            return true;
        }

        auto dxvkDeviceContext = GetDxvkDeviceContext(deviceContext, D3D11_VK_EXT_MULTI_DRAW_INDIRECT);
        if (dxvkDeviceContext == nullptr)
            return false;

        m_multiDrawIndirectContextMap.emplace(deviceContext, dxvkDeviceContext.ptr());
        dxvkDeviceContext->MultiDrawIndirect(drawCount, buffer, alignedByteOffsetForArgs, alignedByteStrideForArgs);
        return true;
    }

    bool NvapiD3d11Device::MultiDrawIndexedInstancedIndirect(ID3D11DeviceContext* deviceContext, const NvU32 drawCount, ID3D11Buffer* buffer, const NvU32 alignedByteOffsetForArgs, const NvU32 alignedByteStrideForArgs) {
        auto it = m_multiDrawIndirectContextMap.find(deviceContext);
        if (it != m_multiDrawIndirectContextMap.end()) {
            it->second->MultiDrawIndexedIndirect(drawCount, buffer, alignedByteOffsetForArgs, alignedByteStrideForArgs); // See note above about reference counting
            return true;
        }

        auto dxvkDeviceContext = GetDxvkDeviceContext(deviceContext, D3D11_VK_EXT_MULTI_DRAW_INDIRECT);
        if (dxvkDeviceContext == nullptr)
            return false;

        m_multiDrawIndirectContextMap.emplace(deviceContext, dxvkDeviceContext.ptr());
        dxvkDeviceContext->MultiDrawIndexedIndirect(drawCount, buffer, alignedByteOffsetForArgs, alignedByteStrideForArgs);
        return true;
    }

    Com<ID3D11VkExtContext> NvapiD3d11Device::GetDxvkDeviceContext(IUnknown* deviceOrContext, D3D11_VK_EXTENSION extension) {
        Com<ID3D11Device> device;
        Com<ID3D11DeviceContext> deviceContext;
        if (SUCCEEDED(deviceOrContext->QueryInterface(IID_PPV_ARGS(&device))))
            device->GetImmediateContext(&deviceContext);
        else if (SUCCEEDED(deviceOrContext->QueryInterface(IID_PPV_ARGS(&deviceContext))))
            deviceContext->GetDevice(&device);
        else
            return nullptr;

        return GetDxvkDeviceContext(device.ptr(), deviceContext.ptr(), extension);
    }

    Com<ID3D11VkExtContext> NvapiD3d11Device::GetDxvkDeviceContext(ID3D11DeviceContext* deviceContext, D3D11_VK_EXTENSION extension) {
        Com<ID3D11Device> device;
        deviceContext->GetDevice(&device);

        return GetDxvkDeviceContext(device.ptr(), deviceContext, extension);
    }

    Com<ID3D11VkExtContext> NvapiD3d11Device::GetDxvkDeviceContext(ID3D11Device* device, ID3D11DeviceContext* deviceContext, D3D11_VK_EXTENSION extension) {
        Com<ID3D11VkExtDevice> dxvkDevice;
        if (FAILED(device->QueryInterface(IID_PPV_ARGS(&dxvkDevice))))
            return nullptr;

        if (!dxvkDevice->GetExtensionSupport(extension))
            return nullptr;

        Com<ID3D11VkExtContext> dxvkDeviceContext;
        if (FAILED(deviceContext->QueryInterface(IID_PPV_ARGS(&dxvkDeviceContext))))
            return nullptr;

        return dxvkDeviceContext;
    }
}
