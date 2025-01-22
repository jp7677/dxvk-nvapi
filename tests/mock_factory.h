#pragma once

#include "nvapi_tests_private.h"
#include "nvapi_d3d_mocks.h"
#include "nvapi_sysinfo_mocks.h"
#include "nvapi_vulkan_mocks.h"
#include "../src/nvapi/resource_factory.h"

using namespace trompeloeil;

class MockFactory : public dxvk::ResourceFactory {

  public:
    MockFactory(
        std::unique_ptr<DXGIDxvkFactoryMock> dxgiFactory1Mock,
        std::unique_ptr<VkMock> vkMock,
        std::unique_ptr<NvmlMock> nvmlMock,
        std::unique_ptr<LfxMock> lfxMock)
        : m_dxgiFactoryMock(std::move(dxgiFactory1Mock)),
          m_vkMock(std::move(vkMock)),
          m_nvmlMock(std::move(nvmlMock)),
          m_lfxMock(std::move(lfxMock)) {};

    dxvk::Com<IDXGIFactory1> CreateDXGIFactory1() override {
        return m_dxgiFactoryMock.get();
    };

    std::unique_ptr<dxvk::Vk> CreateVulkan(dxvk::Com<IDXGIFactory1>& dxgiFactory) override {
        return std::move(m_vkMock);
    }

    std::unique_ptr<dxvk::Vk> CreateVulkan(const char*) override {
        return std::move(m_vkMock);
    }

    std::unique_ptr<dxvk::Nvml> CreateNvml() override {
        return std::move(m_nvmlMock);
    }

    std::unique_ptr<dxvk::Lfx> CreateLfx() override {
        return std::move(m_lfxMock);
    }

    [[nodiscard]] DXGIDxvkFactoryMock* GetDXGIDxvkFactoryMock() {
        return m_dxgiFactoryMock.get();
    }

  private:
    std::unique_ptr<DXGIDxvkFactoryMock> m_dxgiFactoryMock;
    std::unique_ptr<VkMock> m_vkMock;
    std::unique_ptr<NvmlMock> m_nvmlMock;
    std::unique_ptr<LfxMock> m_lfxMock;
};
