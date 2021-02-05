#include "nvapi_adapter_registry.h"

namespace dxvk {

    NvapiAdapterRegistry::NvapiAdapterRegistry() = default;

    NvapiAdapterRegistry::~NvapiAdapterRegistry() {
        for (const auto& output : m_nvapiOutputs)
            delete (output);

        for (const auto& adapter : m_nvapiAdapters)
            delete (adapter);
    }

    bool NvapiAdapterRegistry::Initialize() {
        Com<IDXGIFactory> dxgiFactory;
        if(FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&dxgiFactory)))
            return false;

        // Query all D3D11 adapter from DXVK to honor any DXVK device filtering
        Com<IDXGIAdapter> dxgiAdapter;
        for (auto i = 0U; dxgiFactory->EnumAdapters(i, &dxgiAdapter) != DXGI_ERROR_NOT_FOUND; i++) {
            auto nvapiAdapter = new NvapiAdapter();
            if (nvapiAdapter->Initialize(dxgiAdapter, &m_nvapiOutputs))
                m_nvapiAdapters.push_back(nvapiAdapter);
            else
                delete (nvapiAdapter);
        }

        return !m_nvapiAdapters.empty();
    }

    u_short NvapiAdapterRegistry::GetAdapterCount() const {
        return m_nvapiAdapters.size();
    }

    NvapiAdapter* NvapiAdapterRegistry::GetAdapter() const {
        return m_nvapiAdapters.front();
    }

    NvapiAdapter* NvapiAdapterRegistry::GetAdapter(const u_short index) const {
        if (index >= m_nvapiAdapters.size())
            return nullptr;

        return m_nvapiAdapters.at(index);
    }

    NvapiAdapter* NvapiAdapterRegistry::GetAdapter(const NvPhysicalGpuHandle handle) const {
        for (const auto& adapter : m_nvapiAdapters)
            if (handle == (NvPhysicalGpuHandle) adapter)
                return adapter;

        return nullptr;
    }

    NvapiAdapter* NvapiAdapterRegistry::GetAdapter(const NvLogicalGpuHandle handle) const {
        for (const auto& adapter : m_nvapiAdapters)
            if (handle == (NvLogicalGpuHandle) adapter)
                return adapter;

        return nullptr;
    }

    NvapiOutput* NvapiAdapterRegistry::GetOutput(const u_short index) const {
        if (index >= m_nvapiOutputs.size())
            return nullptr;

        return m_nvapiOutputs.at(index);
    }

    NvapiOutput* NvapiAdapterRegistry::GetOutput(const NvDisplayHandle handle) const {
        for (const auto& output : m_nvapiOutputs)
            if (handle == (NvDisplayHandle) output)
                return output;

        return nullptr;
    }

    short NvapiAdapterRegistry::GetPrimaryOutputId() const {
        for (auto i = 0U; i <= m_nvapiOutputs.size(); i++)
            if (m_nvapiOutputs.at(i)->IsPrimary())
                return i;

        return -1;
    }

    short NvapiAdapterRegistry::GetOutputId(const std::string& displayName) const {
        for (auto i = 0U; i <= m_nvapiOutputs.size(); i++)
            if (m_nvapiOutputs.at(i)->GetDeviceName() == displayName)
                return i;

        return -1;
    }
}
