#pragma once

#include "nvapi_private.h"
#include "./dxvk/dxvk_interfaces.h"
#include "./dxvk/com_pointer.h"

namespace dxvk {

    class NvapiOutput {

    public:

        NvapiOutput(uintptr_t parent);
        ~NvapiOutput();

        uintptr_t GetParent();
        void Initialize(Com<IDXGIOutput> dxgiOutput);

        std::string GetDeviceName();

    private:

        uintptr_t m_parent;
        std::string m_deviceName;
        std::string FromWideString(const WCHAR *ws);

    };
}
