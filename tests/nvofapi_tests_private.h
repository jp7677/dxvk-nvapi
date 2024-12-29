#pragma once

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#endif // defined(__GNUC__) || defined(__clang__)

#include "../src/nvofapi_private.h"
#include "../src/nvofapi_entrypoints.h"
#include "../src/nvofapi_globals.h"

#include "../inc/catch_amalgamated.hpp"
#include "../inc/catch2/trompeloeil.hpp"
