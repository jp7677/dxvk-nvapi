#pragma once

#include "nvapi_tests_private.h"
#include "mock_factory.h"

using namespace trompeloeil;
using namespace dxvk;

class SectionListener : public Catch::EventListenerBase {

  public:
    using Catch::EventListenerBase::EventListenerBase;

    void sectionStarting(Catch::SectionInfo const& sectionInfo) override {
        ::SetEnvironmentVariableA("DXVK_ENABLE_NVAPI", "");
        ::SetEnvironmentVariableA("DXVK_NVAPI_ALLOW_OTHER_DRIVERS", "");
        ::SetEnvironmentVariableA("DXVK_NVAPI_DRIVER_VERSION", "");
        ::SetEnvironmentVariableA("DXVK_NVAPI_FAKE_VKREFLEX", "");
    }

    void sectionEnded(Catch::SectionStats const& sectionStats) override {
        NvapiD3d11Device::Reset();
        NvapiD3d12Device::Reset();
        NvapiD3d12GraphicsCommandList::Reset();
        NvapiD3d12CommandQueue::Reset();
        NvapiD3dLowLatencyDevice::Reset();
        NvapiVulkanLowLatencyDevice::Reset();

        if (!resourceFactory)
            return;

        auto mockFactory = reinterpret_cast<MockFactory*>(resourceFactory.get());

        // Ensure that Com<*> mocks can be deleted by destructors
        auto e = mockFactory->ConfigureRelease();

        nvapiAdapterRegistry.reset();
        initializationCount = 0ULL;
        resourceFactory.reset();
    }
};
