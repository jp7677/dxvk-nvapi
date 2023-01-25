#pragma once

#include "../nvapi_private.h"
#include "../util/com_pointer.h"

namespace dxvk {
    class NvapiAdapter;

    class NvapiOutput {

      public:
        explicit NvapiOutput(NvapiAdapter* parent, uint32_t adapterIndex, uint32_t outputIndex);
        ~NvapiOutput();

        struct ColorData {
            bool HasST2084Support;
            uint16_t RedPrimaryX;
            uint16_t RedPrimaryY;
            uint16_t GreenPrimaryX;
            uint16_t GreenPrimaryY;
            uint16_t BluePrimaryX;
            uint16_t BluePrimaryY;
            uint16_t WhitePointX;
            uint16_t WhitePointY;
            uint16_t MinLuminance;
            uint16_t MaxLuminance;
            uint16_t MaxFullFrameLuminance;
        };

        void Initialize(Com<IDXGIOutput>& dxgiOutput);
        [[nodiscard]] NvapiAdapter* GetParent() const;
        [[nodiscard]] uint32_t GetId() const;
        [[nodiscard]] std::string GetDeviceName() const;
        [[nodiscard]] bool IsPrimary() const;
        [[nodiscard]] ColorData GetColorData() const;

      private:
        NvapiAdapter* m_parent;
        uint32_t m_id;
        std::string m_deviceName;
        bool m_isPrimary{};
        ColorData m_colorData{};
    };
}
