#pragma once

#include <cstddef>
#include <cstdint>

#include <d3d11_1.h>

#include <iostream>
#include <algorithm>

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#pragma GCC diagnostic ignored "-Wattributes"
#endif // __GNUC__

#include "../inc/nvapi.h"

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif // __GNUC__

#include "./dxvk/dxvk_interfaces.h"
#include "./dxvk/com_pointer.h"
#include "../version.h"
