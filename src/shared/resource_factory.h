#pragma once

#include "../nvapi_private.h"
#include "vk.h"

namespace dxvk {
    class ResourceFactory {

      public:
        ResourceFactory();
        virtual ~ResourceFactory();

        virtual std::unique_ptr<Vk> CreateVulkan(const char* moduleName);
    };
}
