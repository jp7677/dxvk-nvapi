#pragma once

#include "nvapi_private.h"
#include "nvapi/nvapi_resource_factory.h"
#include "nvapi/nvapi_adapter_registry.h"

extern uint64_t initializationCount;
extern std::unique_ptr<dxvk::NvapiResourceFactory> resourceFactory;
extern std::unique_ptr<dxvk::NvapiAdapterRegistry> nvapiAdapterRegistry;
