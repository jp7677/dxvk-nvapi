#pragma once

#include "../nvapi_private.h"
#include "../util/com_pointer.h"

namespace dxvk {
    class NvapiOutput {

      public:
        explicit NvapiOutput(uintptr_t parent, uint32_t adapterIndex, uint32_t outputIndex);
        ~NvapiOutput();

        void Initialize(Com<IDXGIOutput>& dxgiOutput);
        [[nodiscard]] uintptr_t GetParent() const;
        [[nodiscard]] uint32_t GetId() const;
        [[nodiscard]] std::string GetDeviceName() const;
        [[nodiscard]] bool IsPrimary() const;

      private:
        uintptr_t m_parent;
        uint32_t m_id;
        std::string m_deviceName;
        bool m_isPrimary{};
    };
}
