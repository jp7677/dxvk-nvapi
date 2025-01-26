#include "nvapi_adapter_registry.h"
#include "nvapi_d3d11_device.h"
#include "nvapi_d3d12_device.h"
#include "nvapi_d3d_low_latency_device.h"
#include "nvapi_vulkan_low_latency_device.h"
#include "../util/util_log.h"

namespace dxvk {

    NvapiAdapterRegistry::NvapiAdapterRegistry(NvapiResourceFactory& resourceFactory)
        : m_resourceFactory(resourceFactory) {}

    NvapiAdapterRegistry::~NvapiAdapterRegistry() {
        NvapiD3d11Device::Reset();
        NvapiD3d12Device::Reset();
        NvapiD3dLowLatencyDevice::Reset();
        NvapiVulkanLowLatencyDevice::Reset();

        for (const auto output : m_nvapiOutputs)
            delete output;

        for (const auto adapter : m_nvapiAdapters)
            delete adapter;

        m_nvapiOutputs.clear();
        m_nvapiAdapters.clear();
    }

    bool NvapiAdapterRegistry::Initialize() {
        m_dxgiFactory = m_resourceFactory.CreateDXGIFactory1();
        if (m_dxgiFactory == nullptr)
            return false;

        m_vk = m_resourceFactory.CreateVulkan(m_dxgiFactory);
        if (!m_vk || !m_vk->IsAvailable())
            return false;

        m_nvml = m_resourceFactory.CreateNvml();
        if (m_nvml->IsAvailable())
            log::info("NVML loaded and initialized successfully");

        Com<IDXGIVkInteropFactory1> dxgiVkInteropFactory;
        if (SUCCEEDED(m_dxgiFactory->QueryInterface(IID_PPV_ARGS(&dxgiVkInteropFactory))))
            m_dxgiVkInterop = dxgiVkInteropFactory;

        // Query all D3D11 adapter from DXVK to honor any DXVK device filtering
        Com<IDXGIAdapter1> dxgiAdapter;
        for (auto i = 0U; m_dxgiFactory->EnumAdapters1(i, &dxgiAdapter) != DXGI_ERROR_NOT_FOUND; i++) {
            Com<IDXGIAdapter3> dxgiAdapter3;
            if (FAILED(dxgiAdapter->QueryInterface(IID_PPV_ARGS(&dxgiAdapter3))))
                continue;

            auto nvapiAdapter = new NvapiAdapter(*m_vk, *m_nvml, dxgiAdapter3);
            if (nvapiAdapter->Initialize(i, m_nvapiOutputs))
                m_nvapiAdapters.push_back(nvapiAdapter);
            else
                delete nvapiAdapter;
        }

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
}
