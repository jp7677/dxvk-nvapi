#pragma once

#include "nvofapi_tests_private.h"
#include "../../src/shared/resource_factory.h"
#include "../mocks/vulkan_mocks.h"

using namespace trompeloeil;

class MockFactory final : public dxvk::ResourceFactory {

  public:
    MockFactory(std::unique_ptr<VkMock> vkMock)
        : m_vkMock(std::move(vkMock)) {};

    std::unique_ptr<dxvk::Vk> CreateVulkan(const char*) override {
        return std::move(m_vkMock);
    }

  private:
    std::unique_ptr<VkMock> m_vkMock;
};
