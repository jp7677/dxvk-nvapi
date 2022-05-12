#pragma once

#include "../nvapi_private.h"
#include "../util/com_pointer.h"

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
