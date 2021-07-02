#include "resource_factory.h"

namespace dxvk {
    ResourceFactory::ResourceFactory()  = default;

    ResourceFactory::~ResourceFactory()  = default;

    Com<IDXGIFactory> ResourceFactory::CreateDXGIFactory() {
        Com<IDXGIFactory> dxgiFactory;
        if(FAILED(::CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&dxgiFactory)))
            return nullptr;

        return dxgiFactory;
    }

    std::unique_ptr<Vulkan> ResourceFactory::CreateVulkan() {
        return std::make_unique<Vulkan>();
    }

    std::unique_ptr<Nvml> ResourceFactory::CreateNvml() {
        return std::make_unique<Nvml>();
    }
}
