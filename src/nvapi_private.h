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

#include "./dxvk/dxvk_interfaces.h"
#include "./dxvk/com_pointer.h"
#include "../version.h"

typedef unsigned char NvU8;
typedef unsigned int NvU32;

#define NvAPI_Status int
#define NVAPI_OK 0
#define NVAPI_ERROR -1
#define NVAPI_NO_IMPLEMENTATION -3

#define NVAPI_SHORT_STRING_MAX 64
typedef char NvAPI_ShortString[NVAPI_SHORT_STRING_MAX];

typedef struct {
    NvU32 version;
    NvU32 maxNumAFRGroups;
    NvU32 numAFRGroups;
    NvU32 currentAFRIndex;
    NvU32 nextFrameAFRIndex;
    NvU32 previousFrameAFRIndex;
    NvU32 bIsCurAFRGroupNew;
    NvU32 numVRSLIGpus;
} NV_GET_CURRENT_SLI_STATE_V2;

#define NV_GET_CURRENT_SLI_STATE NV_GET_CURRENT_SLI_STATE_V2

typedef enum {
    NVAPI_D3D_SRH_CATEGORY_SLI = 1
} NVAPI_D3D_SETRESOURCEHINT_CATEGORY;

#define NV_DECLARE_HANDLE(name) struct name##__ { int unused; }; typedef struct name##__ *name
NV_DECLARE_HANDLE(NVDX_ObjectHandle);

#define NVAPI_INTERFACE __declspec(dllexport) NvAPI_Status __cdecl
#define NVAPI_QUERYINTERFACE __declspec(dllexport) void* __cdecl 
#define NVAPI_CAST reinterpret_cast<void*>
