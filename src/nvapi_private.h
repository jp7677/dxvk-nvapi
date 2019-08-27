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

typedef unsigned char NvU8;
typedef unsigned int NvU32;

#define NvAPI_Status int

#define NVAPI_INTERFACE __declspec(dllexport) NvAPI_Status __stdcall

#define NVAPI_OK 0
#define NVAPI_ERROR -1
#define NVAPI_NO_IMPLEMENTATION -3
#define NVAPI_INVALID_ARGUMENT -5
#define NVAPI_NVIDIA_DEVICE_NOT_FOUND -6
#define NVAPI_END_ENUMERATION -7
#define NVAPI_INVALID_HANDLE -8
#define NVAPI_INCOMPATIBLE_STRUCT_VERSION -9
#define NVAPI_INVALID_POINTER -14
#define NVAPI_EXPECTED_LOGICAL_GPU_HANDLE -100
#define NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE -101
#define NVAPI_NO_ACTIVE_SLI_TOPOLOGY -113
#define NVAPI_STEREO_NOT_INITIALIZED -140
#define NVAPI_UNREGISTERED_RESOURCE -170

#define NVAPI_SHORT_STRING_MAX 64
typedef char NvAPI_ShortString[NVAPI_SHORT_STRING_MAX];

#define MAKE_NVAPI_VERSION(type,version) (NvU32)(sizeof(type) | ((version)<<16))

typedef struct
{
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

typedef enum
{
    NVAPI_D3D_SRH_CATEGORY_SLI = 1
} NVAPI_D3D_SETRESOURCEHINT_CATEGORY;

#define NV_DECLARE_HANDLE(name) struct name##__ { int unused; }; typedef struct name##__ *name
NV_DECLARE_HANDLE(NVDX_ObjectHandle);