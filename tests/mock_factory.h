#pragma once

#include "nvapi_tests_private.h"
#include "../src/resource_factory.h"

using namespace trompeloeil;

class MockFactory : public dxvk::ResourceFactory {

  public:
    MockFactory(
        std::unique_ptr<DXGIDxvkFactoryMock> dxgiFactory1Mock,
        std::unique_ptr<VulkanMock> vulkanMock,
        std::unique_ptr<NvmlMock> nvmlMock,
        std::unique_ptr<LfxMock> lfxMock)
        : m_dxgiFactoryMock(std::move(dxgiFactory1Mock)),
          m_vulkanMock(std::move(vulkanMock)),
          m_nvmlMock(std::move(nvmlMock)),
          m_lfxMock(std::move(lfxMock)) {};

    dxvk::Com<IDXGIFactory1> CreateDXGIFactory1() override {
        return m_dxgiFactoryMock.get();
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

    [[nodiscard]] std::array<std::unique_ptr<expectation>, 1> ConfigureAllowRelease() {
        return {
            NAMED_ALLOW_CALL(*m_dxgiFactoryMock, Release())
                .RETURN(0)};
    }

  private:
    std::unique_ptr<DXGIDxvkFactoryMock> m_dxgiFactoryMock;
    std::unique_ptr<VulkanMock> m_vulkanMock;
    std::unique_ptr<NvmlMock> m_nvmlMock;
    std::unique_ptr<LfxMock> m_lfxMock;
};
