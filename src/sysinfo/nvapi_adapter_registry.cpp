#include "nvapi_adapter_registry.h"

namespace dxvk {

    NvapiAdapterRegistry::NvapiAdapterRegistry() = default;

    NvapiAdapterRegistry::~NvapiAdapterRegistry() {
        for (const auto output : m_nvapiOutputs)
            delete output;

        for (const auto adapter : m_nvapiAdapters)
            delete adapter;

        m_nvapiOutputs.clear();
        m_nvapiAdapters.clear();
    }

    bool NvapiAdapterRegistry::Initialize() {
        Com<IDXGIFactory> dxgiFactory;
        if(FAILED(::CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&dxgiFactory)))
            return false;

        // Query all D3D11 adapter from DXVK to honor any DXVK device filtering
        Com<IDXGIAdapter> dxgiAdapter;
        for (auto i = 0U; dxgiFactory->EnumAdapters(i, &dxgiAdapter) != DXGI_ERROR_NOT_FOUND; i++) {
            auto nvapiAdapter = new NvapiAdapter();
            if (nvapiAdapter->Initialize(dxgiAdapter, m_nvapiOutputs))
                m_nvapiAdapters.push_back(nvapiAdapter);
            else
                delete nvapiAdapter;
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

    NvapiAdapter* NvapiAdapterRegistry::GetAdapter(NvPhysicalGpuHandle handle) const {
        auto it = std::find_if(m_nvapiAdapters.begin(), m_nvapiAdapters.end(),
            [&handle](const auto& adapter) {
                return (NvPhysicalGpuHandle) adapter == handle;
            });

        return it != m_nvapiAdapters.end() ? *it : nullptr;
    }

    NvapiAdapter* NvapiAdapterRegistry::GetAdapter(NvLogicalGpuHandle handle) const {
        auto it = std::find_if(m_nvapiAdapters.begin(), m_nvapiAdapters.end(),
            [&handle](const auto& adapter) {
                return (NvLogicalGpuHandle) adapter == handle;
            });

        return it != m_nvapiAdapters.end() ? *it : nullptr;
    }

    NvapiOutput* NvapiAdapterRegistry::GetOutput(const u_short index) const {
        if (index >= m_nvapiOutputs.size())
            return nullptr;

        return m_nvapiOutputs.at(index);
    }

    NvapiOutput* NvapiAdapterRegistry::GetOutput(NvDisplayHandle handle) const {
        auto it = std::find_if(m_nvapiOutputs.begin(), m_nvapiOutputs.end(),
            [&handle](const auto& output) {
                return (NvDisplayHandle) output == handle;
            });

        return it != m_nvapiOutputs.end() ? *it : nullptr;
    }

    short NvapiAdapterRegistry::GetPrimaryOutputId() const {
        auto it = std::find_if(m_nvapiOutputs.begin(), m_nvapiOutputs.end(),
            [](const auto& output) {
                return output->IsPrimary();
            });

        return static_cast<short>(it != m_nvapiOutputs.end() ? std::distance(m_nvapiOutputs.begin(), it) : -1);
    }

    short NvapiAdapterRegistry::GetOutputId(const std::string& displayName) const {
        auto it = std::find_if(m_nvapiOutputs.begin(), m_nvapiOutputs.end(),
            [&displayName](const auto& output) {
                return output->GetDeviceName() == displayName;
            });

        return static_cast<short>(it != m_nvapiOutputs.end() ? std::distance(m_nvapiOutputs.begin(), it) : -1);
    }
}
