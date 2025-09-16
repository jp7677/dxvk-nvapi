#pragma once

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#endif // defined(__GNUC__) || defined(__clang__)

#include "../src/nvapi_private.h"
#include "../src/nvapi_globals.h"
#include "../src/nvapi/nvapi_vulkan_low_latency_device.h"
#include "../src/nvapi/nvapi_d3d_low_latency_device.h"
#include "../src/nvapi/nvapi_d3d11_device.h"
#include "../src/nvapi/nvapi_d3d12_device.h"
#include "../src/nvapi/nvapi_d3d12_graphics_command_list.h"
#include "../src/nvapi/nvapi_d3d12_command_queue.h"

#if defined(DEPRECATED)
#undef DEPRECATED
#endif // defined(DEPRECATED)

#include <catch_amalgamated.hpp>
#include <catch2/trompeloeil.hpp>
