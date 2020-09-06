#include "nvapi_adapter_registry.h"

namespace dxvk {

    NvapiAdapterRegistry::NvapiAdapterRegistry() {}
    NvapiAdapterRegistry::~NvapiAdapterRegistry() {}

    void NvapiAdapterRegistry::Initialize() {
        Com<IDXGIFactory> dxgiFactory; 
        if(FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&dxgiFactory)))
            return;

        // Query all D3D11 adapter from DXVK to honor any DXVK device filtering 
        Com<IDXGIAdapter> dxgiAdapter;
        for (u_short i = 0; dxgiFactory->EnumAdapters(i, &dxgiAdapter) != DXGI_ERROR_NOT_FOUND; i++) {
            auto nvapiAdapter = new NvapiAdapter();
            auto success = nvapiAdapter->Initialize(dxgiAdapter);
            if (success)
                m_registry.push_back(nvapiAdapter);
            else
                delete (nvapiAdapter);
        }
    }

    NvPhysicalGpuHandle NvapiAdapterRegistry::GetHandle(u_short index) {
        return IndexToHandle(index);
    }

    bool NvapiAdapterRegistry::Any() {
        return !m_registry.empty();
    }

    bool NvapiAdapterRegistry::Contains(NvPhysicalGpuHandle handle) {
        auto index = HandleToIndex(handle);
        return m_registry.size() > index;
    }

    u_short NvapiAdapterRegistry::Size() {
        return m_registry.size();
    }

    NvapiAdapter* NvapiAdapterRegistry::From(NvPhysicalGpuHandle handle) {
        auto index = HandleToIndex(handle);
        if (m_registry.size() > index)
            return m_registry.at(index);

        return nullptr;
    }

    NvapiAdapter* NvapiAdapterRegistry::First() {
        if (!m_registry.empty())
            return m_registry.front();

        return nullptr;
    }

    NvPhysicalGpuHandle NvapiAdapterRegistry::IndexToHandle(u_short index) {
        // Beware, ugliness ahead!
        // The handles are just mean to be passed back to NvAPI for identifying
        // Instead of real handles we just pass the vector position as identifier.
        // Use offset by one in case somebody checks for zero.
        return (NvPhysicalGpuHandle)(uintptr_t)(index + 1);
    }

    u_short NvapiAdapterRegistry::HandleToIndex(NvPhysicalGpuHandle handle) {
        // Beware, ugliness ahead!
        // See `IndexToHandle`. Getting the index works only for single digits (1 up to 8).
        // Hence this really really ugly.
        return ((uintptr_t)handle) - 1;
    }
}
