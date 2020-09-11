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
        for (auto i = 0U; dxgiFactory->EnumAdapters(i, &dxgiAdapter) != DXGI_ERROR_NOT_FOUND; i++) {
            auto nvapiAdapter = new NvapiAdapter();
            auto success = nvapiAdapter->Initialize(dxgiAdapter);
            if (success) {
                m_nvapiAdapters.push_back(nvapiAdapter);
                for (auto j = 0U; j < nvapiAdapter->GetOutputs().size(); j++)
                    m_nvapiOutputs.push_back(nvapiAdapter->GetOutputs().at(j));
            } else
                delete (nvapiAdapter);
        }
    }

    bool NvapiAdapterRegistry::Any() {
        return !m_nvapiAdapters.empty();
    }

    u_short NvapiAdapterRegistry::Size() {
        return m_nvapiAdapters.size();
    }

    NvapiAdapter* NvapiAdapterRegistry::First() {
        return m_nvapiAdapters.front();
    }

    NvapiAdapter* NvapiAdapterRegistry::GetAdapter(u_short index) {
        if (index > m_nvapiAdapters.size())
            return nullptr;

        return m_nvapiAdapters.at(index);
    }

    NvapiAdapter* NvapiAdapterRegistry::GetAdapter(NvPhysicalGpuHandle handle) {
        for (auto const& adapter : m_nvapiAdapters)
            if (handle == (NvPhysicalGpuHandle) adapter)
                return adapter;

        return nullptr;
    }

    NvapiAdapter* NvapiAdapterRegistry::GetAdapter(NvLogicalGpuHandle handle) {
        for (auto const& adapter : m_nvapiAdapters)
            if (handle == (NvLogicalGpuHandle) adapter)
                return adapter;

        return nullptr;
    }

    NvapiOutput* NvapiAdapterRegistry::GetOutput(u_short index) {
        if (index > m_nvapiOutputs.size())
            return nullptr;

        return m_nvapiOutputs.at(index);
    }

    NvapiOutput* NvapiAdapterRegistry::GetOutput(NvDisplayHandle handle) {
        for (auto const& output : m_nvapiOutputs)
            if (handle == (NvDisplayHandle) output)
                return output;

        return nullptr;
    }

    short NvapiAdapterRegistry::GetOutputId(std::string displayName) {
        for (auto i = 0U; i <= m_nvapiOutputs.size(); i++) {
            if (m_nvapiOutputs.at(i)->GetDeviceName() == displayName)
                return i;
        }

        return -1;
    }
}
