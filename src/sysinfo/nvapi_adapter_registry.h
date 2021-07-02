#pragma once

#include "../nvapi_private.h"
#include "resource_factory.h"
#include "nvapi_adapter.h"
#include "nvapi_output.h"
#include "vulkan.h"
#include "nvml.h"

namespace dxvk {
    class NvapiAdapterRegistry {

    public:
        explicit NvapiAdapterRegistry(ResourceFactory& resourceFactory);
        ~NvapiAdapterRegistry();

        bool Initialize();

        [[nodiscard]] u_short GetAdapterCount() const;
        [[nodiscard]] NvapiAdapter* GetAdapter() const;
        [[nodiscard]] NvapiAdapter* GetAdapter(u_short index) const;
        [[nodiscard]] bool IsAdapter(NvapiAdapter* handle) const;

        [[nodiscard]] NvapiOutput* GetOutput(u_short index) const;
        [[nodiscard]] bool IsOutput(NvapiOutput* handle) const;
        [[nodiscard]] short GetPrimaryOutputId() const;
        [[nodiscard]] short GetOutputId(const std::string& displayName) const;

    private:
        ResourceFactory& m_resourceFactory;
        std::unique_ptr<Vulkan> m_vulkan;
        std::unique_ptr<Nvml> m_nvml;
        std::vector<NvapiAdapter*> m_nvapiAdapters;
        std::vector<NvapiOutput*> m_nvapiOutputs;
    };
}
