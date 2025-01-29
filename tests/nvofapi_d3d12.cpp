#include "nvofapi_tests_private.h"
#include "mocks/d3d12_mocks.h"
#include "nvofapi/mock_factory.h"

using namespace trompeloeil;

TEST_CASE("D3D12 methods succeed", "[.d3d12]") {
    SECTION("CreateInstanceD3D12 fails to initialize with major version other than 5") {
        NV_OF_D3D12_API_FUNCTION_LIST functionList{};
        REQUIRE(NvOFAPICreateInstanceD3D12(0, &functionList) == NV_OF_ERR_INVALID_VERSION);
    }

    SECTION("CreateInstanceD3D12 initializes") {
        auto vk = std::make_unique<VkMock>();
        auto vkDevice = std::make_unique<VkDeviceMock>();

        D3D12Vkd3dDeviceMock device;

        ALLOW_CALL(device, AddRef())
            .RETURN(1);
        ALLOW_CALL(device, Release())
            .RETURN(0);
        ALLOW_CALL(device, QueryInterface(__uuidof(ID3D12DXVKInteropDevice1), _))
            .LR_SIDE_EFFECT(*_2 = static_cast<ID3D12DXVKInteropDevice1*>(&device))
            .RETURN(S_OK);
        ALLOW_CALL(device, QueryInterface(__uuidof(ID3D12Device4), _))
            .LR_SIDE_EFFECT(*_2 = static_cast<ID3D12Device4*>(&device))
            .RETURN(S_OK);
        ALLOW_CALL(device, QueryInterface(__uuidof(ID3D12DeviceExt), _))
            .LR_SIDE_EFFECT(*_2 = static_cast<ID3D12DeviceExt*>(&device))
            .RETURN(S_OK);

        ALLOW_CALL(device, GetVulkanHandles(_, _, _))
            .LR_SIDE_EFFECT(*_3 = reinterpret_cast<VkDevice>(vkDevice.get()))
            .RETURN(S_OK);
        ALLOW_CALL(device, GetExtensionSupport(D3D12_VK_NV_OPTICAL_FLOW))
            .RETURN(TRUE);

        NV_OF_D3D12_API_FUNCTION_LIST functionList{};
        REQUIRE(NvOFAPICreateInstanceD3D12(80, &functionList) == NV_OF_SUCCESS);

        SECTION("CreateInstanceVk fails to initialize when Vulkan is not available") {
            ALLOW_CALL(*vk, IsAvailable()).RETURN(false);
            FORBID_CALL(*vk, GetInstanceProcAddr(_, _));
            FORBID_CALL(*vk, GetDeviceProcAddr(_, _));

            resourceFactory = std::make_unique<MockFactory>(std::move(vk));

            NvOFHandle hOFInstance;
            REQUIRE(functionList.nvCreateOpticalFlowD3D12(static_cast<ID3D12Device*>(&device), &hOFInstance) == NV_OF_ERR_GENERIC);
        }

        SECTION("CreateInstanceVk fails to initialize without VK_NV_optical_flow") {
            ALLOW_CALL(*vk, IsAvailable()).RETURN(false);
            ALLOW_CALL(device, GetExtensionSupport(D3D12_VK_NV_OPTICAL_FLOW)).RETURN(false);
            FORBID_CALL(*vk, GetInstanceProcAddr(_, _));
            FORBID_CALL(*vk, GetDeviceProcAddr(_, _));

            resourceFactory = std::make_unique<MockFactory>(std::move(vk));

            NvOFHandle hOFInstance;
            REQUIRE(functionList.nvCreateOpticalFlowD3D12(static_cast<ID3D12Device*>(&device), &hOFInstance) == NV_OF_ERR_GENERIC);
        }
    }
}
