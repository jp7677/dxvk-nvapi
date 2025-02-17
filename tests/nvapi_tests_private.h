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

#include "../inc/catch_amalgamated.hpp"
#include "../inc/catch2/trompeloeil.hpp"
