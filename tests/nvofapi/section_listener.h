#pragma once

#include "nvofapi_tests_private.h"

class SectionListener : public Catch::EventListenerBase {

  public:
    using Catch::EventListenerBase::EventListenerBase;

    void sectionEnded(Catch::SectionStats const&) override {
        resourceFactory.reset();
    }
};
