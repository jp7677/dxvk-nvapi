#define CATCH_CONFIG_MAIN
#include "../inc/catch_amalgamated.hpp"
#include "section_listener.h"

CATCH_REGISTER_TAG_ALIAS("[@unit-tests]", "[d3d],[d3d11],[d3d12],[drs],[sysinfo],[sysinfo-topo],[sysinfo-nvml],[sysinfo-hdr]")
CATCH_REGISTER_TAG_ALIAS("[@system]", "[system]")
CATCH_REGISTER_TAG_ALIAS("[@all]", "[d3d],[d3d11],[d3d12],[drs],[sysinfo],[sysinfo-topo],[sysinfo-nvml],[sysinfo-hdr],[system]")

CATCH_REGISTER_LISTENER(SectionListener)
