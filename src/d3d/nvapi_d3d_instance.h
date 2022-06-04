#pragma once

#include "../nvapi_private.h"
#include "../resource_factory.h"

namespace dxvk {
    class NvapiD3dInstance {
      public:
        explicit NvapiD3dInstance(ResourceFactory& resourceFactory);
        ~NvapiD3dInstance();

        void Initialize();
        [[nodiscard]] bool IsReflexAvailable();
        [[nodiscard]] bool IsReflexEnabled() const;
        void SetReflexEnabled(bool value);
        void Sleep();
        void SetTargetFrameTime(uint64_t frameTimeUs);

      private:
        ResourceFactory& m_resourceFactory;
        std::unique_ptr<Lfx> m_lfx;
        bool m_isLfxEnabled = false;
    };
}