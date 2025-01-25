#include "resource_factory.h"
#include "../interfaces/dxvk_interfaces.h"
#include "../util/util_string.h"
#include "../util/util_log.h"

namespace dxvk {
    ResourceFactory::ResourceFactory() = default;

    ResourceFactory::~ResourceFactory() = default;

    Com<IDXGIFactory1> ResourceFactory::CreateDXGIFactory1() {
        Com<IDXGIFactory1> dxgiFactory;
        if (FAILED(::CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)))) {
            log::info("Creating DXGI Factory (IDXGIFactory1) failed, please ensure that DXVK's dxgi.dll is present");
            return nullptr;
        }

        return dxgiFactory;
    }

    std::unique_ptr<Vk> ResourceFactory::CreateVulkan(Com<IDXGIFactory1>& dxgiFactory) {
        Com<IDXGIVkInteropFactory> dxgiVkFactory;
        if (FAILED(dxgiFactory->QueryInterface(IID_PPV_ARGS(&dxgiVkFactory)))) {
            log::info("Querying Vulkan entry point from DXGI factory failed, please ensure that DXVK's dxgi.dll (version 2.1 or newer) is present");
            return nullptr;
        }

        return std::make_unique<Vk>(std::move(dxgiVkFactory));
    }

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

    std::unique_ptr<Nvml> ResourceFactory::CreateNvml() {
        return std::make_unique<Nvml>();
    }

    std::unique_ptr<Lfx> ResourceFactory::CreateLfx() {
        return std::make_unique<Lfx>();
    }
}
