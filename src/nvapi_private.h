#pragma once

#include <cstddef>
#include <cstdint>

#include <d3d9.h>
#include <d3d11_1.h>
#include <dxgi1_4.h>

#include <array>
#include <iostream>
#include <utility>
#include <vector>
#include <algorithm>

// GCC complains about the COM interfaces
// not having virtual destructors
#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#pragma GCC diagnostic ignored "-Wattributes"
#endif // __GNUC__

#include "../inc/nvapi.h"

#include "./dxvk/dxvk_interfaces.h"
#include "./dxvk/com_pointer.h"
#include "../version.h"

#define NVAPI_METHOD __declspec(dllexport) NvAPI_Status __cdecl
#define NVAPI_QUERYINTERFACE __declspec(dllexport) void* __cdecl
