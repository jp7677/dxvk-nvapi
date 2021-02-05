#pragma once

#include "../nvapi_private.h"
#include "../dxvk/dxvk_interfaces.h"
#include "../util/com_pointer.h"
#include "../util/util_string.h"

namespace dxvk {

    class NvapiOutput {

    public:

        NvapiOutput(uintptr_t parent);
        ~NvapiOutput();

        uintptr_t GetParent();
        void Initialize(Com<IDXGIOutput> dxgiOutput);

        std::string GetDeviceName();
        bool IsPrimary();

    private:

        uintptr_t m_parent;
        std::string m_deviceName;
        bool m_isPrimary{};

    };
}
