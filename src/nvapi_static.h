#pragma once

#include "resource_factory.h"
#include "sysinfo/nvapi_adapter_registry.h"
#include "d3d/nvapi_d3d_instance.h"

static std::unique_ptr<dxvk::ResourceFactory> resourceFactory;
static std::unique_ptr<dxvk::NvapiAdapterRegistry> nvapiAdapterRegistry;
static std::unique_ptr<dxvk::NvapiD3dInstance> nvapiD3dInstance;

static uint32_t drs = 1;
static NvDRSSessionHandle nvapiDrsSession = reinterpret_cast<NvDRSSessionHandle>(&drs);
static NvDRSProfileHandle nvapiDrsProfile = reinterpret_cast<NvDRSProfileHandle>(&drs);

static auto initializationMutex = std::mutex{};
static auto initializationCount = 0ULL;
