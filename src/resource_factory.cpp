#include "resource_factory.h"
#include "interfaces/dxvk_interfaces.h"
#include "util/util_string.h"
#include "util/util_log.h"

namespace dxvk {
    ResourceFactory::ResourceFactory() = default;

    ResourceFactory::~ResourceFactory() = default;

    Com<IDXGIFactory1> ResourceFactory::CreateDXGIFactory1() {
        Com<IDXGIFactory1> dxgiFactory;
        if (FAILED(::CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&dxgiFactory))) {
            log::info("Creating DXGI Factory (IDXGIFactory1) failed, please ensure that DXVK's dxgi.dll is present");
            return nullptr;
        }

        return dxgiFactory;
    }

    std::unique_ptr<Vulkan> ResourceFactory::CreateVulkan(Com<IDXGIFactory1>& dxgiFactory) {
        Com<IDXGIVkInteropFactory> dxgiVkFactory;
        if (FAILED(dxgiFactory->QueryInterface(IID_PPV_ARGS(&dxgiVkFactory)))) {
            log::info("Querying Vulkan entry point from DXGI factory failed, please ensure that DXVK's dxgi.dll (version 2.1 or newer) is present");
            return nullptr;
        }

        VkInstance vkInstance = VK_NULL_HANDLE;
        PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr{};
        dxgiVkFactory->GetVulkanInstance(&vkInstance, &vkGetInstanceProcAddr);

        log::info(str::format("Successfully acquired Vulkan vkGetInstanceProcAddr @ 0x", std::hex, reinterpret_cast<uintptr_t>(vkGetInstanceProcAddr)));
        return std::make_unique<Vulkan>(dxgiFactory, vkGetInstanceProcAddr);
    }

    std::unique_ptr<Nvml> ResourceFactory::CreateNvml() {
        return std::make_unique<Nvml>();
    }

    std::unique_ptr<Lfx> ResourceFactory::CreateLfx() {
        return std::make_unique<Lfx>();
    }
}
