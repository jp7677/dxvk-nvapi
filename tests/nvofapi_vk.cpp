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

        SECTION("CreateInstanceVk succeeds and supports GetCaps and InitSession") {
            auto vkPhysicalDevice = std::make_unique<VkPhysicalDeviceMock>();
            auto* vkPtr = vk.get();

            ALLOW_CALL(*vkPtr, IsAvailable()).RETURN(true);
            ALLOW_CALL(*vkPtr, GetInstanceProcAddr(_, eq(std::string_view("vkGetPhysicalDeviceQueueFamilyProperties"))))
                .RETURN(reinterpret_cast<PFN_vkVoidFunction>(VkPhysicalDeviceMock::GetPhysicalDeviceQueueFamilyProperties));
            ALLOW_CALL(*vkPtr, GetDeviceProcAddr(_, _))
                .RETURN(nullptr);
            ALLOW_CALL(*vkPtr, GetDeviceProcAddr(_, eq(std::string_view("vkGetDeviceQueue"))))
                .RETURN(reinterpret_cast<PFN_vkVoidFunction>(VkDeviceMock::GetDeviceQueue));
            ALLOW_CALL(*vkPtr, GetDeviceProcAddr(_, eq(std::string_view("vkCreateCommandPool"))))
                .RETURN(reinterpret_cast<PFN_vkVoidFunction>(VkDeviceMock::CreateCommandPool));
            ALLOW_CALL(*vkPtr, GetDeviceProcAddr(_, eq(std::string_view("vkDestroyCommandPool"))))
                .RETURN(reinterpret_cast<PFN_vkVoidFunction>(VkDeviceMock::DestroyCommandPool));
            ALLOW_CALL(*vkPtr, GetDeviceProcAddr(_, eq(std::string_view("vkAllocateCommandBuffers"))))
                .RETURN(reinterpret_cast<PFN_vkVoidFunction>(VkDeviceMock::AllocateCommandBuffers));
            ALLOW_CALL(*vkPtr, GetDeviceProcAddr(_, eq(std::string_view("vkCreateOpticalFlowSessionNV"))))
                .RETURN(reinterpret_cast<PFN_vkVoidFunction>(VkDeviceMock::CreateOpticalFlowSessionNV));
            ALLOW_CALL(*vkPtr, GetDeviceProcAddr(_, eq(std::string_view("vkDestroyOpticalFlowSessionNV"))))
                .RETURN(reinterpret_cast<PFN_vkVoidFunction>(VkDeviceMock::DestroyOpticalFlowSessionNV));
            ALLOW_CALL(*vkPtr, GetDeviceProcAddr(_, eq(std::string_view("vkBindOpticalFlowSessionImageNV"))))
                .RETURN(reinterpret_cast<PFN_vkVoidFunction>(VkDeviceMock::BindOpticalFlowSessionImageNV));
            ALLOW_CALL(*vkPtr, GetDeviceProcAddr(_, eq(std::string_view("vkCmdOpticalFlowExecuteNV"))))
                .RETURN(reinterpret_cast<PFN_vkVoidFunction>(VkDeviceMock::CmdOpticalFlowExecuteNV));

            ALLOW_CALL(*vkPhysicalDevice, vkGetPhysicalDeviceQueueFamilyProperties(_, _, nullptr))
                .SIDE_EFFECT(*_2 = 1);
            ALLOW_CALL(*vkPhysicalDevice, vkGetPhysicalDeviceQueueFamilyProperties(_, _, ne(nullptr)))
                .SIDE_EFFECT(_3[0].queueFlags = VK_QUEUE_OPTICAL_FLOW_BIT_NV);

            ALLOW_CALL(*vkDevice, vkGetDeviceQueue(_, 0U, 0U, _));
            ALLOW_CALL(*vkDevice, vkCreateCommandPool(_, _, _, _)).RETURN(VK_SUCCESS);
            ALLOW_CALL(*vkDevice, vkDestroyCommandPool(_, _, _));
            ALLOW_CALL(*vkDevice, vkAllocateCommandBuffers(_, _, _)).RETURN(VK_SUCCESS);
            ALLOW_CALL(*vkDevice, vkDestroyOpticalFlowSessionNV(_, _, _));

            resourceFactory = std::make_unique<MockFactory>(std::move(vk));

            VkInstance vkInstance{};
            NvOFHandle hOFInstance{};
            REQUIRE(functionList.nvCreateOpticalFlowVk(
                        vkInstance,
                        reinterpret_cast<VkPhysicalDevice>(vkPhysicalDevice.get()),
                        reinterpret_cast<VkDevice>(vkDevice.get()),
                        &hOFInstance)
                == NV_OF_SUCCESS);

            SECTION("GetCaps reports Vulkan optical flow properties") {
                ALLOW_CALL(*vkPtr, GetPhysicalDeviceProperties2(_, _, _))
                    .SIDE_EFFECT(
                        VkMock::ConfigureGetPhysicalDeviceProperties2(_3,
                            [](auto props) {
                                REQUIRE(props.opticalFlowProps != nullptr);
                                props.opticalFlowProps->supportedOutputGridSizes = VK_OPTICAL_FLOW_GRID_SIZE_1X1_BIT_NV | VK_OPTICAL_FLOW_GRID_SIZE_2X2_BIT_NV | VK_OPTICAL_FLOW_GRID_SIZE_4X4_BIT_NV;
                                props.opticalFlowProps->supportedHintGridSizes = VK_OPTICAL_FLOW_GRID_SIZE_4X4_BIT_NV | VK_OPTICAL_FLOW_GRID_SIZE_8X8_BIT_NV;
                                props.opticalFlowProps->hintSupported = VK_TRUE;
                                props.opticalFlowProps->minWidth = 32;
                                props.opticalFlowProps->minHeight = 32;
                                props.opticalFlowProps->maxWidth = 8192;
                                props.opticalFlowProps->maxHeight = 8192;
                                props.opticalFlowProps->maxNumRegionsOfInterest = 4;
                            }));

                uint32_t size = 0;
                REQUIRE(functionList.nvOFGetCaps(hOFInstance, NV_OF_CAPS_SUPPORTED_OUTPUT_GRID_SIZES, nullptr, &size) == NV_OF_SUCCESS);
                REQUIRE(size == 3);

                std::array<uint32_t, 3> outGrids{};
                REQUIRE(functionList.nvOFGetCaps(hOFInstance, NV_OF_CAPS_SUPPORTED_OUTPUT_GRID_SIZES, outGrids.data(), &size) == NV_OF_SUCCESS);
                REQUIRE(size == 3);
                REQUIRE(outGrids[0] == NV_OF_OUTPUT_VECTOR_GRID_SIZE_1);
                REQUIRE(outGrids[1] == NV_OF_OUTPUT_VECTOR_GRID_SIZE_2);
                REQUIRE(outGrids[2] == NV_OF_OUTPUT_VECTOR_GRID_SIZE_4);

                // Truncation when caller passes a smaller buffer
                uint32_t smallSize = 2;
                std::array<uint32_t, 2> smallOutGrids{};
                REQUIRE(functionList.nvOFGetCaps(hOFInstance, NV_OF_CAPS_SUPPORTED_OUTPUT_GRID_SIZES, smallOutGrids.data(), &smallSize) == NV_OF_SUCCESS);
                REQUIRE(smallSize == 2);
                REQUIRE(smallOutGrids[0] == NV_OF_OUTPUT_VECTOR_GRID_SIZE_1);
                REQUIRE(smallOutGrids[1] == NV_OF_OUTPUT_VECTOR_GRID_SIZE_2);

                // Hint grid sizes including 8x8
                size = 0;
                REQUIRE(functionList.nvOFGetCaps(hOFInstance, NV_OF_CAPS_SUPPORTED_HINT_GRID_SIZES, nullptr, &size) == NV_OF_SUCCESS);
                REQUIRE(size == 2);

                std::array<uint32_t, 2> hintGrids{};
                REQUIRE(functionList.nvOFGetCaps(hOFInstance, NV_OF_CAPS_SUPPORTED_HINT_GRID_SIZES, hintGrids.data(), &size) == NV_OF_SUCCESS);
                REQUIRE(size == 2);
                REQUIRE(hintGrids[0] == NV_OF_HINT_VECTOR_GRID_SIZE_4);
                REQUIRE(hintGrids[1] == NV_OF_HINT_VECTOR_GRID_SIZE_8);

                auto queryScalar = [&](NV_OF_CAPS cap, uint32_t expected) {
                    uint32_t count = 0;
                    REQUIRE(functionList.nvOFGetCaps(hOFInstance, cap, nullptr, &count) == NV_OF_SUCCESS);
                    REQUIRE(count == 1);

                    uint32_t val = 0xdeadbeef;
                    REQUIRE(functionList.nvOFGetCaps(hOFInstance, cap, &val, &count) == NV_OF_SUCCESS);
                    REQUIRE(count == 1);
                    REQUIRE(val == expected);
                };

                queryScalar(NV_OF_CAPS_SUPPORT_HINT_WITH_OF_MODE, 1);
                queryScalar(NV_OF_CAPS_SUPPORT_HINT_WITH_ST_MODE, 0);
                queryScalar(NV_OF_CAPS_WIDTH_MIN, 32);
                queryScalar(NV_OF_CAPS_HEIGHT_MIN, 32);
                queryScalar(NV_OF_CAPS_WIDTH_MAX, 8192);
                queryScalar(NV_OF_CAPS_HEIGHT_MAX, 8192);
                queryScalar(NV_OF_CAPS_SUPPORT_ROI, 1);
                queryScalar(NV_OF_CAPS_SUPPORT_ROI_MAX_NUM, 4);

                // Error handling
                uint32_t zeroSize = 0;
                uint32_t val = 0;
                REQUIRE(functionList.nvOFGetCaps(hOFInstance, NV_OF_CAPS_WIDTH_MIN, nullptr, nullptr) == NV_OF_ERR_INVALID_PTR);
                REQUIRE(functionList.nvOFGetCaps(hOFInstance, NV_OF_CAPS_WIDTH_MIN, &val, &zeroSize) == NV_OF_ERR_INVALID_PARAM);
                REQUIRE(functionList.nvOFGetCaps(hOFInstance, NV_OF_CAPS_SUPPORTED_OUTPUT_GRID_SIZES, &val, &zeroSize) == NV_OF_ERR_INVALID_PARAM);
                REQUIRE(functionList.nvOFGetCaps(hOFInstance, NV_OF_CAPS_SUPPORT_MAX, &val, &size) == NV_OF_ERR_GENERIC);
                REQUIRE(functionList.nvOFGetCaps(nullptr, NV_OF_CAPS_WIDTH_MIN, &val, &size) == NV_OF_ERR_GENERIC);
            }

            SECTION("GetCaps falls back to 4x4 output grid when driver reports empty masks") {
                ALLOW_CALL(*vkPtr, GetPhysicalDeviceProperties2(_, _, _))
                    .SIDE_EFFECT(
                        VkMock::ConfigureGetPhysicalDeviceProperties2(_3,
                            [](auto props) {
                                REQUIRE(props.opticalFlowProps != nullptr);
                                props.opticalFlowProps->supportedOutputGridSizes = 0;
                                props.opticalFlowProps->supportedHintGridSizes = 0;
                                props.opticalFlowProps->hintSupported = VK_FALSE;
                                props.opticalFlowProps->maxNumRegionsOfInterest = 0;
                            }));

                uint32_t size = 0;
                uint32_t grid = 0;
                REQUIRE(functionList.nvOFGetCaps(hOFInstance, NV_OF_CAPS_SUPPORTED_OUTPUT_GRID_SIZES, nullptr, &size) == NV_OF_SUCCESS);
                REQUIRE(size == 1);
                REQUIRE(functionList.nvOFGetCaps(hOFInstance, NV_OF_CAPS_SUPPORTED_OUTPUT_GRID_SIZES, &grid, &size) == NV_OF_SUCCESS);
                REQUIRE(size == 1);
                REQUIRE(grid == NV_OF_OUTPUT_VECTOR_GRID_SIZE_4);

                size = 99;
                REQUIRE(functionList.nvOFGetCaps(hOFInstance, NV_OF_CAPS_SUPPORTED_HINT_GRID_SIZES, nullptr, &size) == NV_OF_SUCCESS);
                REQUIRE(size == 0);

                uint32_t scalar = 99;
                size = 1;
                REQUIRE(functionList.nvOFGetCaps(hOFInstance, NV_OF_CAPS_SUPPORT_ROI, &scalar, &size) == NV_OF_SUCCESS);
                REQUIRE(scalar == 0);
            }

            SECTION("InitSession forwards grid sizes, formats, and session flags") {
                VkOpticalFlowSessionCreateInfoNV captured{};
                REQUIRE_CALL(*vkDevice, vkCreateOpticalFlowSessionNV(_, _, nullptr, _))
                    .LR_SIDE_EFFECT(captured = *_2)
                    .RETURN(VK_SUCCESS);

                NV_OF_INIT_PARAMS initParams{};
                initParams.width = 1920;
                initParams.height = 1080;
                initParams.outGridSize = NV_OF_OUTPUT_VECTOR_GRID_SIZE_2;
                initParams.hintGridSize = NV_OF_HINT_VECTOR_GRID_SIZE_8;
                initParams.mode = NV_OF_MODE_OPTICALFLOW;
                initParams.perfLevel = NV_OF_PERF_LEVEL_SLOW;
                initParams.enableExternalHints = NV_OF_TRUE;
                initParams.enableOutputCost = NV_OF_TRUE;
                initParams.enableRoi = NV_OF_TRUE;
                initParams.predDirection = NV_OF_PRED_DIRECTION_BOTH;
                initParams.enableGlobalFlow = NV_OF_TRUE;
                initParams.inputBufferFormat = NV_OF_BUFFER_FORMAT_GRAYSCALE8;

                REQUIRE(functionList.nvOFInit(hOFInstance, &initParams) == NV_OF_SUCCESS);
                REQUIRE(captured.width == 1920);
                REQUIRE(captured.height == 1080);
                REQUIRE(captured.outputGridSize == VK_OPTICAL_FLOW_GRID_SIZE_2X2_BIT_NV);
                REQUIRE(captured.hintGridSize == VK_OPTICAL_FLOW_GRID_SIZE_8X8_BIT_NV);
                REQUIRE(captured.flowVectorFormat == VK_FORMAT_R16G16_SINT);
                REQUIRE(captured.costFormat == VK_FORMAT_R8_UINT);
                REQUIRE(captured.imageFormat == VK_FORMAT_R8_UNORM);
                REQUIRE(captured.performanceLevel == VK_OPTICAL_FLOW_PERFORMANCE_LEVEL_SLOW_NV);
                REQUIRE(captured.flags == (VK_OPTICAL_FLOW_SESSION_CREATE_ENABLE_COST_BIT_NV | VK_OPTICAL_FLOW_SESSION_CREATE_BOTH_DIRECTIONS_BIT_NV | VK_OPTICAL_FLOW_SESSION_CREATE_ENABLE_GLOBAL_FLOW_BIT_NV | VK_OPTICAL_FLOW_SESSION_CREATE_ALLOW_REGIONS_BIT_NV | VK_OPTICAL_FLOW_SESSION_CREATE_ENABLE_HINT_BIT_NV));
                REQUIRE(captured.pNext == nullptr);
            }

            SECTION("InitSession preserves private data and undefined flow vector format") {
                struct TestPrivData {
                    uint32_t size;
                    uint32_t id;
                    void* data;
                };
                uint32_t payload = 0x12345678;
                TestPrivData priv{sizeof(payload), 7, &payload};

                VkOpticalFlowSessionCreateInfoNV captured{};
                VkOpticalFlowSessionCreatePrivateDataInfoNV capturedPriv{};
                REQUIRE_CALL(*vkDevice, vkCreateOpticalFlowSessionNV(_, _, nullptr, _))
                    .LR_SIDE_EFFECT(
                        captured = *_2;
                        if (_2->pNext)
                            capturedPriv = *reinterpret_cast<const VkOpticalFlowSessionCreatePrivateDataInfoNV*>(_2->pNext);)
                    .RETURN(VK_SUCCESS);

                NV_OF_INIT_PARAMS initParams{};
                initParams.width = 1280;
                initParams.height = 720;
                initParams.outGridSize = NV_OF_OUTPUT_VECTOR_GRID_SIZE_UNDEFINED;
                initParams.perfLevel = NV_OF_PERF_LEVEL_FAST;
                initParams.hPrivData = reinterpret_cast<NvOFPrivDataHandle>(&priv);
                initParams.inputBufferFormat = NV_OF_BUFFER_FORMAT_ABGR8;

                REQUIRE(functionList.nvOFInit(hOFInstance, &initParams) == NV_OF_SUCCESS);
                REQUIRE(captured.outputGridSize == VK_OPTICAL_FLOW_GRID_SIZE_4X4_BIT_NV);
                REQUIRE(captured.flowVectorFormat == VK_FORMAT_UNDEFINED);
                REQUIRE(captured.costFormat == VK_FORMAT_UNDEFINED);
                REQUIRE(captured.flags == 0);
                REQUIRE(captured.imageFormat == VK_FORMAT_B8G8R8A8_UNORM);
                REQUIRE(captured.performanceLevel == VK_OPTICAL_FLOW_PERFORMANCE_LEVEL_FAST_NV);
                REQUIRE(captured.pNext != nullptr);
                REQUIRE(capturedPriv.sType == VK_STRUCTURE_TYPE_OPTICAL_FLOW_SESSION_CREATE_PRIVATE_DATA_INFO_NV);
                REQUIRE(capturedPriv.size == sizeof(payload));
                REQUIRE(capturedPriv.id == 7);
                REQUIRE(capturedPriv.pPrivateData == &payload);
            }

            REQUIRE(functionList.nvOFDestroy(hOFInstance) == NV_OF_SUCCESS);
        }
    }
}
