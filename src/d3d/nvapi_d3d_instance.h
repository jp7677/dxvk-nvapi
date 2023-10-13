#pragma once

#include "../nvapi_private.h"
#include "../resource_factory.h"

namespace dxvk {
    class NvapiD3dInstance {
      public:
        explicit NvapiD3dInstance(ResourceFactory& resourceFactory);
        ~NvapiD3dInstance();

        void Initialize();
        [[nodiscard]] bool IsReflexAvailable(IUnknown* device);
        [[nodiscard]] bool IsLowLatencyEnabled() const;
        [[nodiscard]] bool IsUsingLfx() const;
        [[nodiscard]] bool SetReflexMode(IUnknown* device, bool enable, bool boost, uint32_t frameTimeUs);
        [[nodiscard]] bool Sleep(IUnknown* device);

      private:
        constexpr static uint64_t kNanoInMicro = 1000;

        ResourceFactory& m_resourceFactory;
        std::unique_ptr<Lfx> m_lfx;
        bool m_isLowLatencyEnabled = false;
    };
}