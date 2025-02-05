#define CATCH_CONFIG_MAIN
#include "../inc/catch_amalgamated.hpp"
#include "nvapi/section_listener.h"

CATCH_REGISTER_TAG_ALIAS("[@unit-tests]", "[d3d],[d3d11],[d3d12],[drs],[ngx],[sysinfo],[sysinfo-topo],[sysinfo-nvml],[sysinfo-hdr],[util],[vulkan]")
CATCH_REGISTER_TAG_ALIAS("[@system]", "[system]")
CATCH_REGISTER_TAG_ALIAS("[@all]", "[d3d],[d3d11],[d3d12],[drs],[ngx],[sysinfo],[sysinfo-topo],[sysinfo-nvml],[sysinfo-hdr],[util],[system],[vulkan]")

CATCH_REGISTER_LISTENER(SectionListener)
