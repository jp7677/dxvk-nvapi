#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#endif // defined(__GNUC__) || defined(__clang__)

#include "../src/nvapi_private.h"
#include "../src/nvapi_globals.h"
#include "../src/d3d/nvapi_d3d_low_latency_device.h"
#include "../src/d3d11/nvapi_d3d11_device.h"
#include "../src/d3d12/nvapi_d3d12_device.h"
#include "../inc/catch_amalgamated.hpp"
#include "../inc/catch2/trompeloeil.hpp"
