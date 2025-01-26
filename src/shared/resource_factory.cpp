#include "resource_factory.h"
#include "../interfaces/dxvk_interfaces.h"
#include "../util/util_string.h"
#include "../util/util_log.h"

namespace dxvk {
    ResourceFactory::ResourceFactory() = default;

    ResourceFactory::~ResourceFactory() = default;

    std::unique_ptr<Vk> ResourceFactory::CreateVulkan(const char* moduleName) {
        auto module = GetModuleHandleA(moduleName);
        if (!module) {
            log::info(str::format("Module ", moduleName, " not loaded into current process"));
            return nullptr;
        }

#define GET_PROC_ADDRESS(proc) auto proc = reinterpret_cast<PFN_##proc>(reinterpret_cast<void*>(GetProcAddress(module, #proc)));

        GET_PROC_ADDRESS(vkGetInstanceProcAddr);
        GET_PROC_ADDRESS(vkGetDeviceProcAddr);

        return std::make_unique<Vk>(vkGetInstanceProcAddr, vkGetDeviceProcAddr);
    }
}
