#include "nvapi_adapter_registry.h"
#include "../util/util_log.h"

namespace dxvk {

    NvapiAdapterRegistry::NvapiAdapterRegistry(ResourceFactory& resourceFactory)
        : m_resourceFactory(resourceFactory) {}

    NvapiAdapterRegistry::~NvapiAdapterRegistry() {
        for (const auto output : m_nvapiOutputs)
            delete output;

        for (const auto adapter : m_nvapiAdapters)
            delete adapter;

        m_nvapiOutputs.clear();
        m_nvapiAdapters.clear();
    }

    bool NvapiAdapterRegistry::Initialize() {
        auto dxgiFactory = m_resourceFactory.CreateDXGIFactory1();
        if (dxgiFactory == nullptr)
            return false;

        m_vulkan = m_resourceFactory.CreateVulkan();
        if (!m_vulkan->IsAvailable())
            return false;

        m_nvml = m_resourceFactory.CreateNvml();
        if (m_nvml->IsAvailable())
            log::write("NVML loaded and initialized successfully");

        // Query all D3D11 adapter from DXVK to honor any DXVK device filtering
        Com<IDXGIAdapter1> dxgiAdapter;
        for (auto i = 0U; dxgiFactory->EnumAdapters1(i, &dxgiAdapter) != DXGI_ERROR_NOT_FOUND; i++) {
            auto nvapiAdapter = new NvapiAdapter(*m_vulkan, *m_nvml);
            if (nvapiAdapter->Initialize(dxgiAdapter, i, m_nvapiOutputs))
                m_nvapiAdapters.push_back(nvapiAdapter);
            else
                delete nvapiAdapter;
        }

        sanitizeOutputs();

        return !m_nvapiAdapters.empty();
    }

    uint32_t NvapiAdapterRegistry::GetAdapterCount() const {
        return m_nvapiAdapters.size();
    }

    NvapiAdapter* NvapiAdapterRegistry::GetAdapter(const uint32_t index) const {
        return index < m_nvapiAdapters.size() ? m_nvapiAdapters[index] : nullptr;
    }

    NvapiAdapter* NvapiAdapterRegistry::GetFirstAdapter() const {
        return m_nvapiAdapters.front();
    }

    NvapiAdapter* NvapiAdapterRegistry::FindAdapter(const LUID& luid) const {
        auto it = std::find_if(m_nvapiAdapters.begin(), m_nvapiAdapters.end(),
            [luid](const auto& adapter) {
                auto adapterLuid = adapter->GetLuid();
                return adapterLuid.has_value()
                    && adapterLuid.value().HighPart == luid.HighPart
                    && adapterLuid.value().LowPart == luid.LowPart;
            });

        return it != m_nvapiAdapters.end() ? *it : nullptr;
    }

    bool NvapiAdapterRegistry::IsAdapter(NvapiAdapter* handle) const {
        return std::find(m_nvapiAdapters.begin(), m_nvapiAdapters.end(), handle) != m_nvapiAdapters.end();
    }

    uint32_t NvapiAdapterRegistry::GetOutputCount(NvapiAdapter* handle) const {
        return std::count_if(m_nvapiOutputs.begin(), m_nvapiOutputs.end(),
            [&handle](const auto& output) {
                return output->GetParent() == handle;
            });
    }

    NvapiOutput* NvapiAdapterRegistry::GetOutput(const uint32_t index) const {
        return index < m_nvapiOutputs.size() ? m_nvapiOutputs[index] : nullptr;
    }

    NvapiOutput* NvapiAdapterRegistry::GetOutput(NvapiAdapter* handle, const uint32_t index) const {
        std::vector<NvapiOutput*> filtered;
        std::copy_if(m_nvapiOutputs.begin(), m_nvapiOutputs.end(), std::back_inserter(filtered),
            [&handle](const auto& output) {
                return output->GetParent() == handle;
            });

        return index < filtered.size() ? filtered[index] : nullptr;
    }

    NvapiOutput* NvapiAdapterRegistry::FindOutput(const std::string& displayName) const {
        auto it = std::find_if(m_nvapiOutputs.begin(), m_nvapiOutputs.end(),
            [&displayName](const auto& output) {
                return output->GetDeviceName() == displayName;
            });

        return it != m_nvapiOutputs.end() ? *it : nullptr;
    }

    NvapiOutput* NvapiAdapterRegistry::FindOutput(uint32_t id) const {
        auto it = std::find_if(m_nvapiOutputs.begin(), m_nvapiOutputs.end(),
            [id](const auto& output) {
                return output->GetId() == id;
            });

        return it != m_nvapiOutputs.end() ? *it : nullptr;
    }

    NvapiOutput* NvapiAdapterRegistry::FindPrimaryOutput() const {
        auto it = std::find_if(m_nvapiOutputs.begin(), m_nvapiOutputs.end(),
            [](const auto& output) {
                return output->IsPrimary();
            });

        return it != m_nvapiOutputs.end() ? *it : nullptr;
    }

    bool NvapiAdapterRegistry::IsOutput(NvapiOutput* handle) const {
        return std::find(m_nvapiOutputs.begin(), m_nvapiOutputs.end(), handle) != m_nvapiOutputs.end();
    }

    void NvapiAdapterRegistry::sanitizeOutputs() {
        auto end = std::remove_if(m_nvapiOutputs.begin(), m_nvapiOutputs.end(),
            [this](const auto output) {
                auto it = std::find_if(m_nvapiOutputs.begin(), m_nvapiOutputs.end(),
                    [&output](const auto& other) {
                        return other != output && other->GetDeviceName() == output->GetDeviceName();
                    });

                // Remove output when it belongs to an integrated adapter and
                // another output with the same name belongs to a discrete adapter
                return it != m_nvapiOutputs.end()
                    && output->GetParent()->GetGpuType() == NV_SYSTEM_TYPE_IGPU
                    && (*it)->GetParent()->GetGpuType() == NV_SYSTEM_TYPE_DGPU;
            });

        std::for_each(end, m_nvapiOutputs.end(),
            [](const auto& duplicate) {
                delete duplicate;
            });

        m_nvapiOutputs.erase(end, m_nvapiOutputs.end());
    }
}
