#pragma once

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <map>
#include <set>
#include <optional>
#include <sstream>
#include <fstream>
#include <ctime>
#include <mutex>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

#include <dxgi1_6.h>
#include <d3d11_1.h>
#ifdef _MSC_VER
#include <d3d12.h>
#else
#include "../inc/d3d12.h"
#endif
#include <vulkan/vulkan.h>
#include "../inc/nvml.h"

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#pragma GCC diagnostic ignored "-Wattributes"
#endif // __GNUC__

// Suppress deprecation warnings.
#define NVAPI_INTERNAL 1
#include "../inc/nvapi.h"

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif // __GNUC__

#if defined(__GNUC__)
#define _ReturnAddress() __builtin_return_address(0);
#elif defined(_MSC_VER)
#include <intrin.h>
#endif
