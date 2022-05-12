#include "resource_factory.h"

namespace dxvk {
    ResourceFactory::ResourceFactory() = default;

    ResourceFactory::~ResourceFactory() = default;

    Com<IDXGIFactory1> ResourceFactory::CreateDXGIFactory1() {
        Com<IDXGIFactory1> dxgiFactory;
        if (FAILED(::CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&dxgiFactory)))
            return nullptr;

        return dxgiFactory;
    }

    std::unique_ptr<Vulkan> ResourceFactory::CreateVulkan() {
        return std::make_unique<Vulkan>();
    }

    std::unique_ptr<Nvml> ResourceFactory::CreateNvml() {
        return std::make_unique<Nvml>();
    }

    std::unique_ptr<Lfx> ResourceFactory::CreateLfx() {
        return std::make_unique<Lfx>();
    }
}
