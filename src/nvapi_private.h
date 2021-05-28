#pragma once

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <map>
#include <sstream>
#include <fstream>
#include <ctime>

#include <dxgi.h>
#include <d3d11_1.h>
#include <d3d12.h>
#include <vulkan/vulkan.h>

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#pragma GCC diagnostic ignored "-Wattributes"
#endif // __GNUC__

// Ugly hack due to outdated d3d12 headers but nvapi wanting ID3D12GraphicsCommandList1.
// This should be fine since we don't implement any method with that interface, but still... :(
#define ID3D12GraphicsCommandList1 ID3D12GraphicsCommandList

#include "../inc/nvapi.h"

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif // __GNUC__
