#include "nvapi_d3d11_device.h"

namespace dxvk {
    bool NvapiD3d11Device::SetDepthBoundsTest(IUnknown* deviceOrContext, const bool enable, const float minDepth, const float maxDepth) {
        auto it = m_depthBoundsDeviceOrContextMap.find(deviceOrContext);
        if (it != m_depthBoundsDeviceOrContextMap.end()) {
            it->second->SetDepthBoundsTest(enable, minDepth, maxDepth);
            return true;
        }

        Com<ID3D11Device> d3d11Device;
        Com<ID3D11DeviceContext> d3d11DeviceContext;
        if (SUCCEEDED(deviceOrContext->QueryInterface(IID_PPV_ARGS(&d3d11Device))))
            d3d11Device->GetImmediateContext(&d3d11DeviceContext);
        else if (FAILED(deviceOrContext->QueryInterface(IID_PPV_ARGS(&d3d11DeviceContext))))
            return false;

        if (!IsSupportedExtension(d3d11DeviceContext.ptr(), D3D11_VK_EXT_DEPTH_BOUNDS))
            return false;

        auto dxvkDeviceContext = GetDxvkDeviceContext(d3d11DeviceContext.ptr());
        if (dxvkDeviceContext == nullptr)
            return false;

        dxvkDeviceContext->SetDepthBoundsTest(enable, minDepth, maxDepth);
        m_depthBoundsDeviceOrContextMap.emplace(deviceOrContext, dxvkDeviceContext.ptr());
        return true;
    }

    bool NvapiD3d11Device::BeginUAVOverlap(IUnknown* deviceOrContext) {
        auto it = m_barrierControlDeviceOrContextMap.find(deviceOrContext);
        if (it != m_barrierControlDeviceOrContextMap.end()) {
            it->second->SetBarrierControl(D3D11_VK_BARRIER_CONTROL_IGNORE_WRITE_AFTER_WRITE);
            return true;
        }

        Com<ID3D11Device> d3d11Device;
        Com<ID3D11DeviceContext> d3d11DeviceContext;
        if (SUCCEEDED(deviceOrContext->QueryInterface(IID_PPV_ARGS(&d3d11Device))))
            d3d11Device->GetImmediateContext(&d3d11DeviceContext);
        else if (FAILED(deviceOrContext->QueryInterface(IID_PPV_ARGS(&d3d11DeviceContext))))
            return false;

        if (!IsSupportedExtension(d3d11DeviceContext.ptr(), D3D11_VK_EXT_BARRIER_CONTROL))
            return false;

        auto dxvkDeviceContext = GetDxvkDeviceContext(d3d11DeviceContext.ptr());
        if (dxvkDeviceContext == nullptr)
            return false;

        dxvkDeviceContext->SetBarrierControl(D3D11_VK_BARRIER_CONTROL_IGNORE_WRITE_AFTER_WRITE);
        m_barrierControlDeviceOrContextMap.emplace(deviceOrContext, dxvkDeviceContext.ptr());
        return true;
    }

    bool NvapiD3d11Device::EndUAVOverlap(IUnknown* deviceOrContext) {
        auto it = m_barrierControlDeviceOrContextMap.find(deviceOrContext);
        if (it != m_barrierControlDeviceOrContextMap.end()) {
            it->second->SetBarrierControl(0);
            return true;
        }

        Com<ID3D11Device> d3d11Device;
        Com<ID3D11DeviceContext> d3d11DeviceContext;
        if (SUCCEEDED(deviceOrContext->QueryInterface(IID_PPV_ARGS(&d3d11Device))))
            d3d11Device->GetImmediateContext(&d3d11DeviceContext);
        else if (FAILED(deviceOrContext->QueryInterface(IID_PPV_ARGS(&d3d11DeviceContext))))
            return false;

        if (!IsSupportedExtension(d3d11DeviceContext.ptr(), D3D11_VK_EXT_BARRIER_CONTROL))
            return false;

        auto dxvkDeviceContext = GetDxvkDeviceContext(d3d11DeviceContext.ptr());
        if (dxvkDeviceContext == nullptr)
            return false;

        dxvkDeviceContext->SetBarrierControl(0U);
        m_barrierControlDeviceOrContextMap.emplace(deviceOrContext, dxvkDeviceContext.ptr());
        return true;
    }

    bool NvapiD3d11Device::MultiDrawInstancedIndirect(ID3D11DeviceContext* deviceContext, const NvU32 drawCount, ID3D11Buffer* buffer, const NvU32 alignedByteOffsetForArgs, const NvU32 alignedByteStrideForArgs) {
        auto it = m_MultiDrawIndirectContextMap.find(deviceContext);
        if (it != m_MultiDrawIndirectContextMap.end()) {
            it->second->MultiDrawIndirect(drawCount, buffer, alignedByteOffsetForArgs, alignedByteStrideForArgs);
            return true;
        }

        if (!IsSupportedExtension(deviceContext, D3D11_VK_EXT_MULTI_DRAW_INDIRECT))
            return false;

        auto dxvkDeviceContext = GetDxvkDeviceContext(deviceContext);
        if (dxvkDeviceContext == nullptr)
            return false;

        dxvkDeviceContext->MultiDrawIndirect(drawCount, buffer, alignedByteOffsetForArgs, alignedByteStrideForArgs);
        m_MultiDrawIndirectContextMap.emplace(deviceContext, dxvkDeviceContext.ptr());
        return true;
    }

    bool NvapiD3d11Device::MultiDrawIndexedInstancedIndirect(ID3D11DeviceContext* deviceContext, const NvU32 drawCount, ID3D11Buffer* buffer, const NvU32 alignedByteOffsetForArgs, const NvU32 alignedByteStrideForArgs) {
        auto it = m_MultiDrawIndirectContextMap.find(deviceContext);
        if (it != m_MultiDrawIndirectContextMap.end()) {
            it->second->MultiDrawIndexedIndirect(drawCount, buffer, alignedByteOffsetForArgs, alignedByteStrideForArgs);
            return true;
        }

        if (!IsSupportedExtension(deviceContext, D3D11_VK_EXT_MULTI_DRAW_INDIRECT))
            return false;

        auto dxvkDeviceContext = GetDxvkDeviceContext(deviceContext);
        if (dxvkDeviceContext == nullptr)
            return false;

        dxvkDeviceContext->MultiDrawIndexedIndirect(drawCount, buffer, alignedByteOffsetForArgs, alignedByteStrideForArgs);
        m_MultiDrawIndirectContextMap.emplace(deviceContext, dxvkDeviceContext.ptr());
        return true;
    }

    bool NvapiD3d11Device::IsSupportedExtension(ID3D11DeviceContext* deviceContext, const D3D11_VK_EXTENSION extension) {
        Com<ID3D11Device> device;
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
