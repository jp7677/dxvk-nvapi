#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"

#include "../inc/catch.hpp"
#include "../inc/catch2/trompeloeil.hpp"
#include "../src/nvapi_d3d11.cpp"

using namespace trompeloeil;

class ID3D11DxvkDevice : public ID3D11Device, public ID3D11VkExtDevice {};

class D3D11DxvkDeviceMock : public mock_interface<ID3D11DxvkDevice> {
    MAKE_MOCK2(QueryInterface, HRESULT(REFIID, void**), override);
    MAKE_MOCK0(AddRef, ULONG(), override);
    MAKE_MOCK0(Release, ULONG(), override);
    IMPLEMENT_MOCK3(GetPrivateData);
    IMPLEMENT_MOCK3(SetPrivateData);
    IMPLEMENT_MOCK2(SetPrivateDataInterface);
    IMPLEMENT_MOCK3(CreateBuffer);
    IMPLEMENT_MOCK3(CreateTexture1D);
    IMPLEMENT_MOCK3(CreateTexture2D);
    IMPLEMENT_MOCK3(CreateTexture3D);
    IMPLEMENT_MOCK3(CreateShaderResourceView);
    IMPLEMENT_MOCK3(CreateUnorderedAccessView);
    IMPLEMENT_MOCK3(CreateRenderTargetView);
    IMPLEMENT_MOCK3(CreateDepthStencilView);
    IMPLEMENT_MOCK5(CreateInputLayout);
    IMPLEMENT_MOCK4(CreateVertexShader);
    IMPLEMENT_MOCK4(CreateGeometryShader);
    IMPLEMENT_MOCK9(CreateGeometryShaderWithStreamOutput);
    IMPLEMENT_MOCK4(CreatePixelShader);
    IMPLEMENT_MOCK4(CreateHullShader);
    IMPLEMENT_MOCK4(CreateDomainShader);
    IMPLEMENT_MOCK4(CreateComputeShader);
    IMPLEMENT_MOCK1(CreateClassLinkage);
    IMPLEMENT_MOCK2(CreateBlendState);
    IMPLEMENT_MOCK2(CreateDepthStencilState);
    IMPLEMENT_MOCK2(CreateRasterizerState);
    IMPLEMENT_MOCK2(CreateSamplerState);
    IMPLEMENT_MOCK2(CreateQuery);
    IMPLEMENT_MOCK2(CreatePredicate);
    IMPLEMENT_MOCK2(CreateCounter);
    IMPLEMENT_MOCK2(CreateDeferredContext);
    IMPLEMENT_MOCK3(OpenSharedResource);
    IMPLEMENT_MOCK2(CheckFormatSupport);
    IMPLEMENT_MOCK3(CheckMultisampleQualityLevels);
    IMPLEMENT_MOCK1(CheckCounterInfo);
    IMPLEMENT_MOCK9(CheckCounter);
    IMPLEMENT_MOCK3(CheckFeatureSupport);
    IMPLEMENT_MOCK0(GetFeatureLevel);
    IMPLEMENT_MOCK0(GetCreationFlags);
    IMPLEMENT_MOCK0(GetDeviceRemovedReason);
    IMPLEMENT_MOCK1(SetExceptionMode);
    IMPLEMENT_MOCK0(GetExceptionMode);
    IMPLEMENT_MOCK1(GetImmediateContext);
    IMPLEMENT_MOCK1(GetExtensionSupport);
};

class ID3D11DxvkDeviceContext : public ID3D11DeviceContext, public ID3D11VkExtContext {};

class D3D11DxvkDeviceContextMock : public mock_interface<ID3D11DxvkDeviceContext> {
    MAKE_MOCK2(QueryInterface, HRESULT(REFIID, void**), override);
    MAKE_MOCK0(AddRef, ULONG(), override);
    MAKE_MOCK0(Release, ULONG(), override);
    IMPLEMENT_MOCK1(GetDevice);
    IMPLEMENT_MOCK3(GetPrivateData);
    IMPLEMENT_MOCK3(SetPrivateData);
    IMPLEMENT_MOCK2(SetPrivateDataInterface);
    IMPLEMENT_MOCK3(VSSetConstantBuffers);
    IMPLEMENT_MOCK3(PSSetShaderResources);
    IMPLEMENT_MOCK3(PSSetShader);
    IMPLEMENT_MOCK3(PSSetSamplers);
    IMPLEMENT_MOCK3(VSSetShader);
    IMPLEMENT_MOCK3(DrawIndexed);
    IMPLEMENT_MOCK2(Draw);
    IMPLEMENT_MOCK5(Map);
    IMPLEMENT_MOCK2(Unmap);
    IMPLEMENT_MOCK3(PSSetConstantBuffers);
    IMPLEMENT_MOCK1(IASetInputLayout);
    IMPLEMENT_MOCK5(IASetVertexBuffers);
    IMPLEMENT_MOCK3(IASetIndexBuffer);
    IMPLEMENT_MOCK5(DrawIndexedInstanced);
    IMPLEMENT_MOCK4(DrawInstanced);
    IMPLEMENT_MOCK3(GSSetConstantBuffers);
    IMPLEMENT_MOCK3(GSSetShader);
    IMPLEMENT_MOCK1(IASetPrimitiveTopology);
    IMPLEMENT_MOCK3(VSSetShaderResources);
    IMPLEMENT_MOCK3(VSSetSamplers);
    IMPLEMENT_MOCK1(Begin);
    IMPLEMENT_MOCK1(End);
    IMPLEMENT_MOCK4(GetData);
    IMPLEMENT_MOCK2(SetPredication);
    IMPLEMENT_MOCK3(GSSetShaderResources);
    IMPLEMENT_MOCK3(GSSetSamplers);
    IMPLEMENT_MOCK3(OMSetRenderTargets);
    IMPLEMENT_MOCK7(OMSetRenderTargetsAndUnorderedAccessViews);
    IMPLEMENT_MOCK3(OMSetBlendState);
    IMPLEMENT_MOCK2(OMSetDepthStencilState);
    IMPLEMENT_MOCK3(SOSetTargets);
    IMPLEMENT_MOCK0(DrawAuto);
    IMPLEMENT_MOCK2(DrawIndexedInstancedIndirect);
    IMPLEMENT_MOCK2(DrawInstancedIndirect);
    IMPLEMENT_MOCK3(Dispatch);
    IMPLEMENT_MOCK2(DispatchIndirect);
    IMPLEMENT_MOCK1(RSSetState);
    IMPLEMENT_MOCK2(RSSetViewports);
    IMPLEMENT_MOCK2(RSSetScissorRects);
    IMPLEMENT_MOCK8(CopySubresourceRegion);
    IMPLEMENT_MOCK2(CopyResource);
    IMPLEMENT_MOCK6(UpdateSubresource);
    IMPLEMENT_MOCK3(CopyStructureCount);
    IMPLEMENT_MOCK2(ClearRenderTargetView);
    IMPLEMENT_MOCK2(ClearUnorderedAccessViewUint);
    IMPLEMENT_MOCK2(ClearUnorderedAccessViewFloat);
    IMPLEMENT_MOCK4(ClearDepthStencilView);
    IMPLEMENT_MOCK1(GenerateMips);
    IMPLEMENT_MOCK2(SetResourceMinLOD);
    IMPLEMENT_MOCK1(GetResourceMinLOD);
    IMPLEMENT_MOCK5(ResolveSubresource);
    IMPLEMENT_MOCK2(ExecuteCommandList);
    IMPLEMENT_MOCK3(HSSetShaderResources);
    IMPLEMENT_MOCK3(HSSetShader);
    IMPLEMENT_MOCK3(HSSetSamplers);
    IMPLEMENT_MOCK3(HSSetConstantBuffers);
    IMPLEMENT_MOCK3(DSSetShaderResources);
    IMPLEMENT_MOCK3(DSSetShader);
    IMPLEMENT_MOCK3(DSSetSamplers);
    IMPLEMENT_MOCK3(DSSetConstantBuffers);
    IMPLEMENT_MOCK3(CSSetShaderResources);
    IMPLEMENT_MOCK4(CSSetUnorderedAccessViews);
    IMPLEMENT_MOCK3(CSSetShader);
    IMPLEMENT_MOCK3(CSSetSamplers);
    IMPLEMENT_MOCK3(CSSetConstantBuffers);
    IMPLEMENT_MOCK3(VSGetConstantBuffers);
    IMPLEMENT_MOCK3(PSGetShaderResources);
    IMPLEMENT_MOCK3(PSGetShader);
    IMPLEMENT_MOCK3(PSGetSamplers);
    IMPLEMENT_MOCK3(VSGetShader);
    IMPLEMENT_MOCK3(PSGetConstantBuffers);
    IMPLEMENT_MOCK1(IAGetInputLayout);
    IMPLEMENT_MOCK5(IAGetVertexBuffers);
    IMPLEMENT_MOCK3(IAGetIndexBuffer);
    IMPLEMENT_MOCK3(GSGetConstantBuffers);
    IMPLEMENT_MOCK3(GSGetShader);
    IMPLEMENT_MOCK1(IAGetPrimitiveTopology);
    IMPLEMENT_MOCK3(VSGetShaderResources);
    IMPLEMENT_MOCK3(VSGetSamplers);
    IMPLEMENT_MOCK2(GetPredication);
    IMPLEMENT_MOCK3(GSGetShaderResources);
    IMPLEMENT_MOCK3(GSGetSamplers);
    IMPLEMENT_MOCK3(OMGetRenderTargets);
    IMPLEMENT_MOCK6(OMGetRenderTargetsAndUnorderedAccessViews);
    IMPLEMENT_MOCK3(OMGetBlendState);
    IMPLEMENT_MOCK2(OMGetDepthStencilState);
    IMPLEMENT_MOCK2(SOGetTargets);
    IMPLEMENT_MOCK1(RSGetState);
    IMPLEMENT_MOCK2(RSGetViewports);
    IMPLEMENT_MOCK2(RSGetScissorRects);
    IMPLEMENT_MOCK3(HSGetShaderResources);
    IMPLEMENT_MOCK3(HSGetShader);
    IMPLEMENT_MOCK3(HSGetSamplers);
    IMPLEMENT_MOCK3(HSGetConstantBuffers);
    IMPLEMENT_MOCK3(DSGetShaderResources);
    IMPLEMENT_MOCK3(DSGetShader);
    IMPLEMENT_MOCK3(DSGetSamplers);
    IMPLEMENT_MOCK3(DSGetConstantBuffers);
    IMPLEMENT_MOCK3(CSGetShaderResources);
    IMPLEMENT_MOCK3(CSGetUnorderedAccessViews);
    IMPLEMENT_MOCK3(CSGetShader);
    IMPLEMENT_MOCK3(CSGetSamplers);
    IMPLEMENT_MOCK3(CSGetConstantBuffers);
    IMPLEMENT_MOCK0(ClearState);
    IMPLEMENT_MOCK0(Flush);
    IMPLEMENT_MOCK0(GetType);
    IMPLEMENT_MOCK0(GetContextFlags);
    IMPLEMENT_MOCK2(FinishCommandList);
    IMPLEMENT_MOCK4(MultiDrawIndirect);
    IMPLEMENT_MOCK4(MultiDrawIndexedIndirect);
    IMPLEMENT_MOCK6(MultiDrawIndirectCount);
    IMPLEMENT_MOCK6(MultiDrawIndexedIndirectCount);
    IMPLEMENT_MOCK3(SetDepthBoundsTest);
    IMPLEMENT_MOCK1(SetBarrierControl);
};

TEST_CASE("SetDepthBoundsTests succeeds", "[d3d11]") {
    D3D11DxvkDeviceMock device;
    D3D11DxvkDeviceContextMock context;

    ALLOW_CALL(device, QueryInterface(_, _))
        .LR_SIDE_EFFECT({
            if (_1 == ID3D11VkExtDevice::guid)
                *_2 = static_cast<ID3D11VkExtDevice*>(&device);
            else if (_1 == IID_ID3D11Device)
                *_2 = static_cast<ID3D11Device*>(&device);
        })
        .RETURN(S_OK);
    ALLOW_CALL(device, Release())
        .RETURN(0);
    ALLOW_CALL(device, GetExtensionSupport(_))
        .RETURN(true);
    ALLOW_CALL(device, GetImmediateContext(_))
        .LR_SIDE_EFFECT(*_1 = &context);

    ALLOW_CALL(context, QueryInterface(_, _))
        .LR_SIDE_EFFECT({
            if (_1 == ID3D11VkExtContext::guid)
                *_2 = static_cast<ID3D11VkExtContext*>(&context);
            else if (_1 == IID_ID3D11DeviceContext)
                *_2 = static_cast<ID3D11DeviceContext*>(&context);
        })
        .RETURN(S_OK);
    ALLOW_CALL(context, Release())
        .RETURN(0);

    auto enable = true;
    auto min = 0.4f;
    auto max = 0.7f;
    REQUIRE_CALL(context, SetDepthBoundsTest(enable, min, max));
    REQUIRE(NvAPI_D3D11_SetDepthBoundsTest(static_cast<ID3D11Device*>(&device), enable, min, max) == NVAPI_OK);
}
