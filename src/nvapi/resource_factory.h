#pragma once

#include "../nvapi_private.h"
#include "../util/com_pointer.h"
#include "vk.h"
#include "nvml.h"
#include "lfx.h"

namespace dxvk {
    class ResourceFactory {

      public:
        ResourceFactory();
        virtual ~ResourceFactory();

        virtual Com<IDXGIFactory1> CreateDXGIFactory1();
        virtual std::unique_ptr<Vk> CreateVulkan(Com<IDXGIFactory1>& dxgiFactory);
        virtual std::unique_ptr<Vk> CreateVulkan(const char* moduleName);
        virtual std::unique_ptr<Nvml> CreateNvml();
        virtual std::unique_ptr<Lfx> CreateLfx();
    };
}
