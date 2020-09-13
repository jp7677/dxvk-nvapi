extern "C" {
    using namespace dxvk;

    NvAPI_Status __cdecl NvAPI_D3D11_SetDepthBoundsTest(IUnknown* pDeviceOrContext, NvU32 bEnable, float fMinDepth, float fMaxDepth) {
        constexpr auto n = "NvAPI_D3D11_SetDepthBoundsTest";
        static bool alreadyTested = false;
        if (!alreadyTested) {
            alreadyTested = true;

            if (pDeviceOrContext == nullptr)
                return InvalidArgument(n);

            Com<ID3D11VkExtDevice> dxvkDevice;
            if (FAILED(pDeviceOrContext->QueryInterface(IID_PPV_ARGS(&dxvkDevice))))
                return Error(n);

            if (!dxvkDevice->GetExtensionSupport(D3D11_VK_EXT_DEPTH_BOUNDS))
                return Error(n);
        }

        Com<ID3D11Device> d3d11Device;
        if (FAILED(pDeviceOrContext->QueryInterface(IID_PPV_ARGS(&d3d11Device))))
            return Error(n);

        Com<ID3D11DeviceContext> d3d11DeviceContext;
        d3d11Device->GetImmediateContext(&d3d11DeviceContext);

        Com<ID3D11VkExtContext> dxvkDeviceContext;
        if (FAILED(d3d11DeviceContext->QueryInterface(IID_PPV_ARGS(&dxvkDeviceContext))))
            return Error(n);

        dxvkDeviceContext->SetDepthBoundsTest(bEnable, fMinDepth, fMaxDepth);

        static bool alreadyLogged = false;
        if (!alreadyLogged) {
            alreadyLogged = true;
            return Ok(n);
        }

        return Ok();
    }

    NvAPI_Status __cdecl NvAPI_D3D11_IsNvShaderExtnOpCodeSupported(IUnknown* pDeviceOrContext, NvU32 code, bool* supported) {
        constexpr auto n = "NvAPI_D3D11_IsNvShaderExtnOpCodeSupported";
        if (pDeviceOrContext == nullptr || supported == nullptr)
                return InvalidArgument(n);

        *supported = false;

        return Ok(str::format(n, " ", code));
    }
}
