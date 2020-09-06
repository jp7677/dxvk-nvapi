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
        NvPhysicalGpuHandle GetHandle(u_short index);
        bool Any();
        bool Contains(NvPhysicalGpuHandle handle);
        u_short Size();
        NvapiAdapter* From(NvPhysicalGpuHandle handle);
        NvapiAdapter* First();
    
    private:

        std::vector<NvapiAdapter*> m_registry;

    };
}
