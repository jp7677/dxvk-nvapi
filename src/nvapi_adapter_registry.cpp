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
        return (NvPhysicalGpuHandle) &(m_registry.at(index)->GetHandle());
    }

    bool NvapiAdapterRegistry::Any() {
        return !m_registry.empty();
    }

    bool NvapiAdapterRegistry::Contains(NvPhysicalGpuHandle handle) {
        for (auto const& adapter : m_registry)
            if (handle == (NvPhysicalGpuHandle) &(adapter->GetHandle()))
                return true;

        return false;
    }

    u_short NvapiAdapterRegistry::Size() {
        return m_registry.size();
    }

    NvapiAdapter* NvapiAdapterRegistry::From(NvPhysicalGpuHandle handle) {
        return (NvapiAdapter*) handle;
    }

    NvapiAdapter* NvapiAdapterRegistry::First() {
        if (!m_registry.empty())
            return m_registry.front();

        return nullptr;
    }
}
