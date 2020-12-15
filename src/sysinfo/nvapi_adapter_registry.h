#pragma once

#include "../nvapi_private.h"
#include "../dxvk/dxvk_interfaces.h"
#include "../util/com_pointer.h"
#include "nvapi_adapter.h"

namespace dxvk {

    class NvapiAdapterRegistry {

    public:

        NvapiAdapterRegistry();
        ~NvapiAdapterRegistry();

        bool Initialize();

        u_short GetAdapterCount();
        NvapiAdapter* GetAdapter();
        NvapiAdapter* GetAdapter(const u_short index);
        NvapiAdapter* GetAdapter(const NvPhysicalGpuHandle handle);
        NvapiAdapter* GetAdapter(const NvLogicalGpuHandle handle);

        NvapiOutput* GetOutput(const u_short index);
        NvapiOutput* GetOutput(const NvDisplayHandle handle);
        short GetPrimaryOutputId();
        short GetOutputId(const std::string& displayName);

    private:

        std::vector<NvapiAdapter*> m_nvapiAdapters;
        std::vector<NvapiOutput*> m_nvapiOutputs;

    };
}
