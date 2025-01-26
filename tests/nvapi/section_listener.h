#pragma once

#include "nvapi_tests_private.h"
#include "resource_factory_util.h"

class SectionListener : public Catch::EventListenerBase {

  public:
    using Catch::EventListenerBase::EventListenerBase;

    void sectionStarting(Catch::SectionInfo const& sectionInfo) override {
        ::SetEnvironmentVariableA("DXVK_ENABLE_NVAPI", "");
        ::SetEnvironmentVariableA("DXVK_NVAPI_ALLOW_OTHER_DRIVERS", "");
        ::SetEnvironmentVariableA("DXVK_NVAPI_DRIVER_VERSION", "");
    }

    void sectionEnded(Catch::SectionStats const&) override {
        ResetGlobals();
    }
};
