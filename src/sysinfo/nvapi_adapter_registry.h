#pragma once

#include "../nvapi_private.h"
#include "nvapi_adapter.h"
#include "nvapi_output.h"

namespace dxvk {
    class NvapiAdapterRegistry {

    public:
        NvapiAdapterRegistry();
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
        std::unique_ptr<Vulkan> m_vulkan;
        std::vector<NvapiAdapter*> m_nvapiAdapters;
        std::vector<NvapiOutput*> m_nvapiOutputs;
    };
}
