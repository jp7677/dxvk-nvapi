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
            if (success) {
                m_nvapiAdapters.push_back(nvapiAdapter);
                for (u_short j = 0; j < nvapiAdapter->GetOutputs().size(); j++)
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
        if (!m_nvapiAdapters.empty())
            return m_nvapiAdapters.front();

        return nullptr;
    }

    NvapiAdapter* NvapiAdapterRegistry::At(u_short index) {
        return &(m_nvapiAdapters.at(index)->GetHandle());
    }


    bool NvapiAdapterRegistry::Contains(NvPhysicalGpuHandle handle) {
        for (auto const& adapter : m_nvapiAdapters)
            if (handle == (NvPhysicalGpuHandle) &(adapter->GetHandle()))
                return true;

        return false;
    }

    NvapiAdapter* NvapiAdapterRegistry::From(NvPhysicalGpuHandle handle) {
        return (NvapiAdapter*) handle;
    }


    bool NvapiAdapterRegistry::Contains(NvLogicalGpuHandle handle) {
        for (auto const& adapter : m_nvapiAdapters)
            if (handle == (NvLogicalGpuHandle) &(adapter->GetHandle()))
                return true;

        return false;
    }

    NvapiAdapter* NvapiAdapterRegistry::From(NvLogicalGpuHandle handle) {
        return (NvapiAdapter*) handle;
    }

    bool NvapiAdapterRegistry::HasOutput(u_short index) {
        return m_nvapiOutputs.size() > index;
    }

    NvapiOutput* NvapiAdapterRegistry::GetOutput(u_short index) {
        return m_nvapiOutputs.at(index);
    }

    short NvapiAdapterRegistry::GetOutput(std::string displayName) {
        for (u_short i = 0; i <= m_nvapiOutputs.size(); i++) {
            if (m_nvapiOutputs.at(i)->GetDeviceName() == displayName)
                return i;
        }

        return -1;
    }

    bool NvapiAdapterRegistry::Contains(NvDisplayHandle handle) {
        for (auto const& output : m_nvapiOutputs)
            if (handle == (NvDisplayHandle) &(output->GetHandle()))
                return true;

        return false;
    }

    NvapiOutput* NvapiAdapterRegistry::From(NvDisplayHandle handle) {
        return (NvapiOutput*) handle;
    }
}
