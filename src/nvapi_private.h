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

#include <dxgi.h>
#include <d3d11_1.h>
#include <d3d12.h>
#include <vulkan/vulkan.h>
#include "../inc/nvml.h"

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#pragma GCC diagnostic ignored "-Wattributes"
#endif // __GNUC__

#include "../inc/nvapi.h"

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif // __GNUC__
