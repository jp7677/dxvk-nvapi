#pragma once

#include "nvapi_private.h"
#include "util/com_pointer.h"
#include "sysinfo/vulkan.h"
#include "sysinfo/nvml.h"
#include "d3d/lfx.h"

namespace dxvk {
    class ResourceFactory {

      public:
        ResourceFactory();
        virtual ~ResourceFactory();

        virtual Com<IDXGIFactory1> CreateDXGIFactory1();
        virtual std::unique_ptr<Vulkan> CreateVulkan();
        virtual std::unique_ptr<Nvml> CreateNvml();
        virtual std::unique_ptr<Lfx> CreateLfx();
    };
}
