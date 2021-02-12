#pragma once

#include "../nvapi_private.h"
#include "../util/com_pointer.h"
#include "nvapi_adapter.h"

namespace dxvk {
    class NvapiAdapterRegistry {

    public:
        NvapiAdapterRegistry();
        ~NvapiAdapterRegistry();

        bool Initialize();

        [[nodiscard]] u_short GetAdapterCount() const;
        [[nodiscard]] NvapiAdapter* GetAdapter() const;
        [[nodiscard]] NvapiAdapter* GetAdapter(u_short index) const;
        [[nodiscard]] NvapiAdapter* GetAdapter(NvPhysicalGpuHandle handle) const;
        [[nodiscard]] NvapiAdapter* GetAdapter(NvLogicalGpuHandle handle) const;

        [[nodiscard]] NvapiOutput* GetOutput(u_short index) const;
        [[nodiscard]] NvapiOutput* GetOutput(NvDisplayHandle handle) const;
        [[nodiscard]] short GetPrimaryOutputId() const;
        [[nodiscard]] short GetOutputId(const std::string& displayName) const;

    private:
        std::vector<NvapiAdapter*> m_nvapiAdapters;
        std::vector<NvapiOutput*> m_nvapiOutputs;
    };
}
