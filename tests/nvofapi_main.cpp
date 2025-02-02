#define CATCH_CONFIG_MAIN
#include "../inc/catch_amalgamated.hpp"
#include "nvofapi/section_listener.h"

CATCH_REGISTER_TAG_ALIAS("[@unit-tests]", "[d3d11],[d3d12],[vk],[cuda]")
CATCH_REGISTER_TAG_ALIAS("[@all]", "[d3d11],[d3d12],[vk],[cuda]")

CATCH_REGISTER_LISTENER(SectionListener)
