#pragma once

#include "sysinfo/resource_factory.h"
#include "sysinfo/nvapi_adapter_registry.h"

static std::unique_ptr<dxvk::ResourceFactory> resourceFactory;
static std::unique_ptr<dxvk::NvapiAdapterRegistry> nvapiAdapterRegistry;

static auto initializationMutex = std::mutex{};
static auto initializationCount = 0ULL;
