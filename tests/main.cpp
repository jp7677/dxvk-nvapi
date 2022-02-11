#define CATCH_CONFIG_MAIN
#include "../inc/catch.hpp"

CATCH_REGISTER_TAG_ALIAS("[@unit-tests]", "[d3d],[d3d11],[d3d12],[drs],[sysinfo]")
CATCH_REGISTER_TAG_ALIAS("[@system]", "[system]")
CATCH_REGISTER_TAG_ALIAS("[@all]", "[d3d],[d3d11],[d3d12],[drs],[sysinfo],[system]")
