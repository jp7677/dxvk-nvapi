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
        bool Contains(u_short index);
        u_short Size();
        NvapiAdapter* ByIndex(u_short index);
        NvapiAdapter* First();
    
    private:

        std::vector<NvapiAdapter*> m_registry;

    };
}
