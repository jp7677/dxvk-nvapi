#include "nvofapi_tests_private.h"
#include "nvofapi/mock_factory.h"

using namespace trompeloeil;

TEST_CASE("Vk methods succeed", "[.vk]") {

    SECTION("CreateInstanceVk fails to initialize with major version other than 5") {
        NV_OF_VK_API_FUNCTION_LIST functionList{};
        REQUIRE(NvOFAPICreateInstanceVk(0, &functionList) == NV_OF_ERR_INVALID_VERSION);
    }

    SECTION("CreateInstanceVk initializes") {
        auto vk = std::make_unique<VkMock>();
        auto vkDevice = std::make_unique<VkDeviceMock>();

        NV_OF_VK_API_FUNCTION_LIST functionList{};
        REQUIRE(NvOFAPICreateInstanceVk(80, &functionList) == NV_OF_SUCCESS);

        SECTION("CreateInstanceVk fails to initialize when Vulkan is not available") {
            ALLOW_CALL(*vk, IsAvailable()).RETURN(false);
            FORBID_CALL(*vk, GetInstanceProcAddr(_, _));
            FORBID_CALL(*vk, GetDeviceProcAddr(_, _));

            resourceFactory = std::make_unique<MockFactory>(std::move(vk));

            VkInstance vkInstance{};
            VkPhysicalDevice vkPhysicalDevice{};
            NvOFHandle hOFInstance;
            REQUIRE(functionList.nvCreateOpticalFlowVk(vkInstance, vkPhysicalDevice, reinterpret_cast<VkDevice>(vkDevice.get()), &hOFInstance) == NV_OF_ERR_GENERIC);
        }

        SECTION("CreateInstanceVk fails to initialize without VK_NV_optical_flow") {
            ALLOW_CALL(*vk, IsAvailable()).RETURN(false);
            ALLOW_CALL(*vk, GetInstanceProcAddr(_, eq(std::string_view("vkGetPhysicalDeviceQueueFamilyProperties"))))
                .RETURN(reinterpret_cast<PFN_vkVoidFunction>(VkPhysicalDeviceMock::GetPhysicalDeviceQueueFamilyProperties));
            ALLOW_CALL(*vk, GetDeviceProcAddr(_, _))
                .RETURN(nullptr);

            resourceFactory = std::make_unique<MockFactory>(std::move(vk));

            VkInstance vkInstance{};
            VkPhysicalDevice vkPhysicalDevice{};
            NvOFHandle hOFInstance;
            REQUIRE(functionList.nvCreateOpticalFlowVk(vkInstance, vkPhysicalDevice, reinterpret_cast<VkDevice>(vkDevice.get()), &hOFInstance) == NV_OF_ERR_GENERIC);
        }
    }
}
