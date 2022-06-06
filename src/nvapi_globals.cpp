#include "nvapi_globals.h"

uint64_t initializationCount = 0ULL;
std::unique_ptr<dxvk::ResourceFactory> resourceFactory;
std::unique_ptr<dxvk::NvapiAdapterRegistry> nvapiAdapterRegistry;
std::unique_ptr<dxvk::NvapiD3dInstance> nvapiD3dInstance;
