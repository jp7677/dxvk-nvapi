/*
 * SPDX-FileCopyrightText: Copyright (c) 2024 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include "nvofapi_private.h"

// Common entrypoints
NVOFAPI_FUNCTION OFSessionInit(NvOFHandle hOf, const NV_OF_INIT_PARAMS* initParams);

NVOFAPI_FUNCTION OFSessionDestroy(NvOFHandle hOf);

NVOFAPI_FUNCTION OFSessionGetLastError(NvOFHandle hOf, char lastError[], uint32_t* size);

NVOFAPI_FUNCTION OFSessionGetCaps(NvOFHandle hOf, NV_OF_CAPS capsParam, uint32_t* capsVal, uint32_t* size);

// Overwrite CUDA entrypoint to avoid dependency to cuda.h
NVOFAPI_FUNCTION NvOFAPICreateInstanceCuda(uint32_t apiVer, void* functionList);
