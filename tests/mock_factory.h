#pragma once

#include "nvapi_tests_private.h"
#include "../src/resource_factory.h"

using namespace dxvk;

class MockFactory : public ResourceFactory {

  public:
    MockFactory(std::unique_ptr<IDXGIFactory1> dxgiFactory1Mock, std::unique_ptr<Vulkan> vulkanMock,
        std::unique_ptr<Nvml> nvmlMock, std::unique_ptr<Lfx> lfxMock)
        : m_dxgiFactoryMock(std::move(dxgiFactory1Mock)), m_vulkanMock(std::move(vulkanMock)),
          m_nvmlMock(std::move(nvmlMock)), m_lfxMock(std::move(lfxMock)){};

    Com<IDXGIFactory1> CreateDXGIFactory1() override {
        Com<IDXGIFactory1> dxgiFactory = m_dxgiFactoryMock.get();
        return dxgiFactory;
    };

    std::unique_ptr<Vulkan> CreateVulkan() override {
        return std::move(m_vulkanMock);
    }

    std::unique_ptr<Nvml> CreateNvml() override {
        return std::move(m_nvmlMock);
    }

    std::unique_ptr<Lfx> CreateLfx() override {
        return std::move(m_lfxMock);
    }

  private:
    std::unique_ptr<IDXGIFactory1> m_dxgiFactoryMock;
    std::unique_ptr<Vulkan> m_vulkanMock;
    std::unique_ptr<Nvml> m_nvmlMock;
    std::unique_ptr<Lfx> m_lfxMock;
};
