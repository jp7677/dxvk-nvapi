#include "nvapi_adapter_registry.h"

namespace dxvk {

    NvapiAdapterRegistry::NvapiAdapterRegistry() {}
    NvapiAdapterRegistry::~NvapiAdapterRegistry() {}

    void NvapiAdapterRegistry::Initialize() {
        Com<IDXGIFactory> dxgiFactory; 
        if(FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&dxgiFactory)))
            return;

        // Query all D3D11 adapter from DXVK to honor any DXVK device filtering and
        // use the first NVIDIA card. Get the Vulkan handle  from that (DXVK) adapter
        // to get access to Vulkan device properties which has some information we want.
        Com<IDXGIAdapter> dxgiAdapter;
        for (u_short i = 0; dxgiFactory->EnumAdapters(i, &dxgiAdapter) != DXGI_ERROR_NOT_FOUND; ++i) {
            auto nvapiAdapter = new NvapiAdapter();
            auto success = nvapiAdapter->Initialize(dxgiAdapter);
            if (success)
                m_registry.push_back(nvapiAdapter);
        }
    }

    bool NvapiAdapterRegistry::Any() {
        return !m_registry.empty();
    }

    bool NvapiAdapterRegistry::Contains(u_short index) {
        return m_registry.size() > index;
    }

    u_short NvapiAdapterRegistry::Size() {
        return m_registry.size();
    }

    NvapiAdapter* NvapiAdapterRegistry::ByIndex(u_short index) {
        if (m_registry.size() > index)
            return m_registry.at(index);

        return nullptr;
    }

    NvapiAdapter* NvapiAdapterRegistry::First() {
        if (!m_registry.empty())
            return m_registry.at(0);

        return nullptr;
    }
}
