#pragma once

#include "./nvapi_private.h"

#include <nvOpticalFlowD3D11.h>
#include <nvOpticalFlowD3D12.h>
#include <nvOpticalFlowVulkan.h>

#define NVOFAPI_CREATE_INSTANCE extern "C" NV_OF_STATUS __cdecl
#define NVOFAPI_FUNCTION extern "C" NV_OF_STATUS NVOFAPI