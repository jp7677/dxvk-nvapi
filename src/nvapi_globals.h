#pragma once

#include "nvapi_private.h"
#include "resource_factory.h"
#include "nvapi/nvapi_adapter_registry.h"
#include "nvapi/nvapi_d3d_instance.h"

extern uint64_t initializationCount;
extern std::unique_ptr<dxvk::ResourceFactory> resourceFactory;
extern std::unique_ptr<dxvk::NvapiAdapterRegistry> nvapiAdapterRegistry;
extern std::unique_ptr<dxvk::NvapiD3dInstance> nvapiD3dInstance;
