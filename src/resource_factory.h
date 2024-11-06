#pragma once

#include "nvapi_private.h"
#include "util/com_pointer.h"
#include "nvapi/vulkan.h"
#include "nvapi/nvml.h"
#include "nvapi/lfx.h"

namespace dxvk {
    class ResourceFactory {

      public:
        ResourceFactory();
        virtual ~ResourceFactory();

        virtual Com<IDXGIFactory1> CreateDXGIFactory1();
        virtual std::unique_ptr<Vulkan> CreateVulkan(Com<IDXGIFactory1>& dxgiFactory);
        virtual std::unique_ptr<Nvml> CreateNvml();
        virtual std::unique_ptr<Lfx> CreateLfx();
    };
}
