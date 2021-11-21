using namespace trompeloeil;

TEST_CASE("D3D12 capability methods succeed", "[.d3d12]") {
    auto dxgiFactory = std::make_unique<DXGIFactory1Mock>();
    auto vulkan = std::make_unique<VulkanMock>();
    auto nvml = std::make_unique<NvmlMock>();
    DXGIDxvkAdapterMock adapter;
    DXGIOutputMock output;

    auto expectations = ConfigureDefaultTestEnvironment(*dxgiFactory, *vulkan, *nvml, adapter, output);

    SECTION("NvAPI_D3D12_GetGraphicsCapabilities returns OK") {
        D3D12Vkd3dDeviceMock device;
        auto deviceRefCount = 0;
        LUID* luid = new LUID{};

        ALLOW_CALL(device, QueryInterface(__uuidof(ID3D12Device), _))
            .LR_SIDE_EFFECT(*_2 = static_cast<ID3D12Device*>(&device))
            .LR_SIDE_EFFECT(deviceRefCount++)
            .RETURN(S_OK);
        ALLOW_CALL(device, AddRef())
            .LR_SIDE_EFFECT(deviceRefCount++)
            .RETURN(deviceRefCount);
        ALLOW_CALL(device, Release())
            .LR_SIDE_EFFECT(deviceRefCount--)
            .RETURN(deviceRefCount);

        ALLOW_CALL(device, GetAdapterLuid(_))
            .LR_RETURN(luid);

        SECTION("NvAPI_D3D12_GetGraphicsCapabilities without matching adapter returns OK with sm_0") {
            SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml));
            REQUIRE(NvAPI_Initialize() == NVAPI_OK);

            NV_D3D12_GRAPHICS_CAPS graphicsCaps{};
            REQUIRE(NvAPI_D3D12_GetGraphicsCapabilities(static_cast<ID3D12Device*>(&device), NV_D3D12_GRAPHICS_CAPS_VER1, &graphicsCaps) == NVAPI_OK);
            REQUIRE(graphicsCaps.bFastUAVClearSupported == true);
            REQUIRE(graphicsCaps.majorSMVersion == 0);
            REQUIRE(graphicsCaps.majorSMVersion == 0);
            REQUIRE(deviceRefCount == 0);
        }

        SECTION("NvAPI_D3D12_GetGraphicsCapabilities returns OK with valid SM") {
            struct Data {std::string extensionName; uint32_t expectedMajorSMVersion; uint32_t expectedMinorSMVersion;};
            auto args = GENERATE(
                Data{VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME, 8, 6},
                Data{VK_NV_SHADING_RATE_IMAGE_EXTENSION_NAME, 7, 5},
                Data{VK_NVX_IMAGE_VIEW_HANDLE_EXTENSION_NAME, 7, 0},
                Data{VK_NV_CLIP_SPACE_W_SCALING_EXTENSION_NAME, 6, 0},
                Data{VK_NV_VIEWPORT_ARRAY2_EXTENSION_NAME, 5, 0},
                Data{"ext", 0, 0});

            luid->HighPart = 0x08070605;
            luid->LowPart = 0x04030201;

            ALLOW_CALL(*vulkan, GetDeviceExtensions(_, _)) // NOLINT(bugprone-use-after-move)
                .RETURN(std::set<std::string>{
                    VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME,
                    args.extensionName});

            ALLOW_CALL(*vulkan, GetPhysicalDeviceProperties2(_, _, _)) // NOLINT(bugprone-use-after-move)
                .SIDE_EFFECT(
                    ConfigureGetPhysicalDeviceProperties2(_3,
                        [&args](auto props, auto idProps, auto pciBusInfoProps, auto driverProps, auto fragmentShadingRateProps) {
                            idProps->deviceLUIDValid = VK_TRUE;
                            for (auto i = 0U; i < VK_LUID_SIZE; i++)
                                idProps->deviceLUID[i] = i + 1;

                            driverProps->driverID = VK_DRIVER_ID_NVIDIA_PROPRIETARY;
                            if (args.extensionName == VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME)
                                fragmentShadingRateProps->primitiveFragmentShadingRateWithMultipleViewports = VK_TRUE;
                        })
                );

            SetupResourceFactory(std::move(dxgiFactory), std::move(vulkan), std::move(nvml));
            REQUIRE(NvAPI_Initialize() == NVAPI_OK);

            NV_D3D12_GRAPHICS_CAPS graphicsCaps;
            REQUIRE(NvAPI_D3D12_GetGraphicsCapabilities(static_cast<ID3D12Device*>(&device), NV_D3D12_GRAPHICS_CAPS_VER1, &graphicsCaps) == NVAPI_OK);
            REQUIRE(graphicsCaps.bFastUAVClearSupported == true);
            REQUIRE(graphicsCaps.majorSMVersion == args.expectedMajorSMVersion);
            REQUIRE(graphicsCaps.minorSMVersion == args.expectedMinorSMVersion);
            REQUIRE(deviceRefCount == 0);
        }
    }
}
