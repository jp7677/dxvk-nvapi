#include "nvapi_resource_factory.h"
#include "../interfaces/dxvk_interfaces.h"
#include "../util/util_string.h"
#include "../util/util_log.h"

namespace dxvk {
    std::unique_ptr<Vk> NvapiResourceFactory::CreateVulkan(Com<IDXGIFactory1>& dxgiFactory) {
        Com<IDXGIVkInteropFactory> dxgiVkFactory;
        if (FAILED(dxgiFactory->QueryInterface(IID_PPV_ARGS(&dxgiVkFactory)))) {
            log::info("Querying Vulkan entry point from DXGI factory failed, please ensure that DXVK's dxgi.dll (version 2.1 or newer) is present");
            return nullptr;
        }

        return std::make_unique<Vk>(std::move(dxgiVkFactory));
    }

    Com<IDXGIFactory1> NvapiResourceFactory::CreateDXGIFactory1() {
        Com<IDXGIFactory1> dxgiFactory;
        if (FAILED(::CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)))) {
            log::info("Creating DXGI Factory (IDXGIFactory1) failed, please ensure that DXVK's dxgi.dll is present");
            return nullptr;
        }

        return dxgiFactory;
    }

    std::unique_ptr<Nvml> NvapiResourceFactory::CreateNvml() {
        return std::make_unique<Nvml>();
    }
}
