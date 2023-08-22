#pragma once

#include "nvapi_tests_private.h"
#include "resource_factory_util.h"

class SectionEndedListener : public Catch::EventListenerBase {

  public:
    using Catch::EventListenerBase::EventListenerBase;

    void sectionEnded(Catch::SectionStats const&) override {
        ResetGlobals();
    }
};
