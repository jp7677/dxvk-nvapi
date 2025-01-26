#pragma once

#include "../nvapi_private.h"
#include "nvapi_resource_factory.h"
#include "nvapi_adapter.h"
#include "nvapi_output.h"
#include "../interfaces/dxvk_interfaces.h"

namespace dxvk {
    class NvapiAdapterRegistry {

      public:
        explicit NvapiAdapterRegistry(NvapiResourceFactory& resourceFactory);
        ~NvapiAdapterRegistry();

        bool Initialize();

        [[nodiscard]] uint32_t GetAdapterCount() const;
        [[nodiscard]] NvapiAdapter* GetAdapter(uint32_t index) const;
        [[nodiscard]] NvapiAdapter* GetFirstAdapter() const;
        [[nodiscard]] NvapiAdapter* FindAdapter(const LUID& luid) const;
        [[nodiscard]] bool IsAdapter(NvapiAdapter* handle) const;

        [[nodiscard]] uint32_t GetOutputCount(NvapiAdapter* handle) const;
        [[nodiscard]] NvapiOutput* GetOutput(uint32_t index) const;
        [[nodiscard]] NvapiOutput* GetOutput(NvapiAdapter* handle, uint32_t index) const;
        [[nodiscard]] NvapiOutput* FindOutput(const std::string& displayName) const;
        [[nodiscard]] NvapiOutput* FindOutput(uint32_t id) const;
        [[nodiscard]] NvapiOutput* FindPrimaryOutput() const;
        [[nodiscard]] bool IsOutput(NvapiOutput* handle) const;

        [[nodiscard]] IDXGIVkInteropFactory1* GetInteropFactory() const { return m_dxgiVkInterop.ptr(); }

      private:
        NvapiResourceFactory& m_resourceFactory;
        Com<IDXGIFactory1> m_dxgiFactory;
        Com<IDXGIVkInteropFactory1> m_dxgiVkInterop;
        std::unique_ptr<Vk> m_vk;
        std::unique_ptr<Nvml> m_nvml;
        std::vector<NvapiAdapter*> m_nvapiAdapters;
        std::vector<NvapiOutput*> m_nvapiOutputs;
    };
}
