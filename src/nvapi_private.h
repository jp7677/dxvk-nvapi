#pragma once

#include <cstddef>
#include <cstdint>

#include <dxgi.h>
#include <d3d11_1.h>
#include <vulkan/vulkan.h>

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <map>
#include <sstream>
#include <fstream>
#include <ctime>

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#pragma GCC diagnostic ignored "-Wattributes"
#endif // __GNUC__

#include "../inc/nvapi.h"

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif // __GNUC__
