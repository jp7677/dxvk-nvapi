#include "nvapi_globals.h"

uint64_t initializationCount = 0ULL;
std::unique_ptr<dxvk::NvapiResourceFactory> resourceFactory;
std::unique_ptr<dxvk::NvapiAdapterRegistry> nvapiAdapterRegistry;
