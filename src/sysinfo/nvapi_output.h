#pragma once

#include "../nvapi_private.h"
#include "../dxvk/dxvk_interfaces.h"
#include "../util/com_pointer.h"
#include "../util/util_string.h"

namespace dxvk {

    class NvapiOutput {

    public:

        explicit NvapiOutput(uintptr_t parent);
        ~NvapiOutput();

        void Initialize(Com<IDXGIOutput>& dxgiOutput);
        [[nodiscard]] uintptr_t GetParent() const;
        [[nodiscard]] std::string GetDeviceName() const;
        [[nodiscard]] bool IsPrimary() const;

    private:

        uintptr_t m_parent;
        std::string m_deviceName;
        bool m_isPrimary{};

    };
}
