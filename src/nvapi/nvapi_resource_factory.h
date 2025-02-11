#pragma once

#include "../nvapi_private.h"
#include "../util/com_pointer.h"
#include "../shared/resource_factory.h"
#include "../shared/vk.h"
#include "nvml.h"

namespace dxvk {
    class NvapiResourceFactory : ResourceFactory {

      public:
        std::unique_ptr<Vk> CreateVulkan(const char* moduleName) override { return ResourceFactory::CreateVulkan(moduleName); }
        virtual std::unique_ptr<Vk> CreateVulkan(Com<IDXGIFactory1>& dxgiFactory);
        virtual Com<IDXGIFactory1> CreateDXGIFactory1();
        virtual std::unique_ptr<Nvml> CreateNvml();
    };
}
