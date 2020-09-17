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
        NvapiAdapter* GetAdapter(u_short index);
        NvapiAdapter* GetAdapter(NvPhysicalGpuHandle handle);
        NvapiAdapter* GetAdapter(NvLogicalGpuHandle handle);

        NvapiOutput* GetOutput(u_short index);
        NvapiOutput* GetOutput(NvDisplayHandle handle);
        short GetOutputId(std::string displayName);

    private:

        std::vector<NvapiAdapter*> m_nvapiAdapters;
        std::vector<NvapiOutput*> m_nvapiOutputs;

    };
}
