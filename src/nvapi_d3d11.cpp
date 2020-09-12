extern "C" {
    using namespace dxvk;

    NvAPI_Status __cdecl NvAPI_D3D11_SetDepthBoundsTest(IUnknown* pDeviceOrContext, NvU32 bEnable, float fMinDepth, float fMaxDepth) {
        static bool alreadyTested = false;
        if (!alreadyTested) {
            alreadyTested = true;
            Com<ID3D11VkExtDevice> dxvkDevice;
            if (FAILED(pDeviceOrContext->QueryInterface(IID_PPV_ARGS(&dxvkDevice))))
                return Error("NvAPI_D3D11_SetDepthBoundsTest");

            if (!dxvkDevice->GetExtensionSupport(D3D11_VK_EXT_DEPTH_BOUNDS))
                return Error("NvAPI_D3D11_SetDepthBoundsTest");
        }

        Com<ID3D11Device> d3d11Device;
        if (FAILED(pDeviceOrContext->QueryInterface(IID_PPV_ARGS(&d3d11Device))))
            return Error("NvAPI_D3D11_SetDepthBoundsTest");

        Com<ID3D11DeviceContext> d3d11DeviceContext;
        d3d11Device->GetImmediateContext(&d3d11DeviceContext);

        Com<ID3D11VkExtContext> dxvkDeviceContext;
        if (FAILED(d3d11DeviceContext->QueryInterface(IID_PPV_ARGS(&dxvkDeviceContext))))
            return Error("NvAPI_D3D11_SetDepthBoundsTest");

        dxvkDeviceContext->SetDepthBoundsTest(bEnable, fMinDepth, fMaxDepth);

        static bool alreadyLogged = false;
        if (!alreadyLogged) {
            alreadyLogged = true;
            return Ok("NvAPI_D3D11_SetDepthBoundsTest");
        }

        return NVAPI_OK;
    }

    NvAPI_Status __cdecl NvAPI_D3D11_IsNvShaderExtnOpCodeSupported(IUnknown* pDeviceOrContext, NvU32 code, bool* supported) {
        *supported = false;

        return Ok(str::format("NvAPI_D3D11_IsNvShaderExtnOpCodeSupported ", std::hex, code));
    }
}
