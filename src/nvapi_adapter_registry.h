#pragma once

#include "nvapi_private.h"
#include "./dxvk/dxvk_interfaces.h"
#include "./dxvk/com_pointer.h"
#include "nvapi_adapter.h"

namespace dxvk {

    class NvapiAdapterRegistry {

    public:

        NvapiAdapterRegistry();
        ~NvapiAdapterRegistry();

        void Initialize();
        
        bool Any();
        u_short Size();
        NvapiAdapter* First();

        NvapiAdapter* At(u_short index);

        bool Contains(NvPhysicalGpuHandle handle);
        NvapiAdapter* From(NvPhysicalGpuHandle handle);

        bool Contains(NvLogicalGpuHandle handle);
        NvapiAdapter* From(NvLogicalGpuHandle handle);

        bool HasOutput(u_short index);
        NvapiOutput* GetOutput(u_short index);
        short GetOutput(std::string displayName);
        bool Contains(NvDisplayHandle handle);
        NvapiOutput* From(NvDisplayHandle handle);
    
    private:

        std::vector<NvapiAdapter*> m_nvapiAdapters;
        std::vector<NvapiOutput*> m_nvapiOutputs;

    };
}
