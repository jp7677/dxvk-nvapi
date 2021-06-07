#include "nvapi_adapter_registry.h"
#include "vulkan.h"
#include "../util/util_string.h"
#include "../util/util_log.h"

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
        m_vulkan = std::make_unique<Vulkan>();
        if (!m_vulkan->IsLoaded())
            return false;

        Com<IDXGIFactory> dxgiFactory;
        if(FAILED(::CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&dxgiFactory)))
            return false;

        // Query all D3D11 adapter from DXVK to honor any DXVK device filtering
        Com<IDXGIAdapter> dxgiAdapter;
        for (auto i = 0U; dxgiFactory->EnumAdapters(i, &dxgiAdapter) != DXGI_ERROR_NOT_FOUND; i++) {
            auto nvapiAdapter = new NvapiAdapter(*m_vulkan);
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
        return index < m_nvapiAdapters.size() ? m_nvapiAdapters[index] : nullptr;
    }

    bool NvapiAdapterRegistry::IsAdapter(NvapiAdapter* handle) const {
        return std::find(m_nvapiAdapters.begin(), m_nvapiAdapters.end(), handle) != m_nvapiAdapters.end();
    }

    NvapiOutput* NvapiAdapterRegistry::GetOutput(const u_short index) const {
        return index < m_nvapiOutputs.size() ? m_nvapiOutputs[index] : nullptr;
    }

    bool NvapiAdapterRegistry::IsOutput(NvapiOutput* handle) const {
        return std::find(m_nvapiOutputs.begin(), m_nvapiOutputs.end(), handle) != m_nvapiOutputs.end();
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
