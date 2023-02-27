#pragma once

#include "nvapi_tests_private.h"
#include "../src/resource_factory.h"

class MockFactory : public dxvk::ResourceFactory {

  public:
    MockFactory(std::unique_ptr<IDXGIFactory1> dxgiFactory1Mock, std::unique_ptr<dxvk::Vulkan> vulkanMock,
        std::unique_ptr<dxvk::Nvml> nvmlMock, std::unique_ptr<dxvk::Lfx> lfxMock)
        : m_dxgiFactoryMock(std::move(dxgiFactory1Mock)), m_vulkanMock(std::move(vulkanMock)),
          m_nvmlMock(std::move(nvmlMock)), m_lfxMock(std::move(lfxMock)){};

    dxvk::Com<IDXGIFactory1> CreateDXGIFactory1() override {
        dxvk::Com<IDXGIFactory1> dxgiFactory = m_dxgiFactoryMock.get();
        return dxgiFactory;
    };

    std::unique_ptr<dxvk::Vulkan> CreateVulkan(dxvk::Com<IDXGIFactory1>& dxgiFactory) override {
        return std::move(m_vulkanMock);
    }

    std::unique_ptr<dxvk::Nvml> CreateNvml() override {
        return std::move(m_nvmlMock);
    }

    std::unique_ptr<dxvk::Lfx> CreateLfx() override {
        return std::move(m_lfxMock);
    }

  private:
    std::unique_ptr<IDXGIFactory1> m_dxgiFactoryMock;
    std::unique_ptr<dxvk::Vulkan> m_vulkanMock;
    std::unique_ptr<dxvk::Nvml> m_nvmlMock;
    std::unique_ptr<dxvk::Lfx> m_lfxMock;
};
