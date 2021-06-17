#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"

using namespace trompeloeil;

class D3D12DeviceMock : public mock_interface<ID3D12Device>
{
    MAKE_MOCK2(QueryInterface, HRESULT(REFIID, void **), override);
    MAKE_MOCK0(AddRef, ULONG(), override);
    MAKE_MOCK0(Release, ULONG(), override);
    IMPLEMENT_MOCK3(GetPrivateData);
    IMPLEMENT_MOCK3(SetPrivateData);
    IMPLEMENT_MOCK2(SetPrivateDataInterface);
    IMPLEMENT_MOCK1(SetName);
    IMPLEMENT_MOCK0(GetNodeCount);
    IMPLEMENT_MOCK3(CreateCommandQueue);
    IMPLEMENT_MOCK3(CreateCommandAllocator);
    IMPLEMENT_MOCK3(CreateGraphicsPipelineState);
    IMPLEMENT_MOCK3(CreateComputePipelineState);
    IMPLEMENT_MOCK6(CreateCommandList);
    IMPLEMENT_MOCK3(CheckFeatureSupport);
    IMPLEMENT_MOCK3(CreateDescriptorHeap);
    IMPLEMENT_MOCK1(GetDescriptorHandleIncrementSize);
    IMPLEMENT_MOCK5(CreateRootSignature);
    IMPLEMENT_MOCK2(CreateConstantBufferView);
    IMPLEMENT_MOCK3(CreateShaderResourceView);
    IMPLEMENT_MOCK4(CreateUnorderedAccessView);
    IMPLEMENT_MOCK3(CreateRenderTargetView);
    IMPLEMENT_MOCK3(CreateDepthStencilView);
    IMPLEMENT_MOCK2(CreateSampler);
    IMPLEMENT_MOCK7(CopyDescriptors);
    IMPLEMENT_MOCK4(CopyDescriptorsSimple);
    MAKE_MOCK4(GetResourceAllocationInfo, D3D12_RESOURCE_ALLOCATION_INFO*(D3D12_RESOURCE_ALLOCATION_INFO*, UINT, UINT, const D3D12_RESOURCE_DESC*), override);
    MAKE_MOCK3(GetCustomHeapProperties, D3D12_HEAP_PROPERTIES*(D3D12_HEAP_PROPERTIES*, UINT, D3D12_HEAP_TYPE), override);
    IMPLEMENT_MOCK7(CreateCommittedResource);
    IMPLEMENT_MOCK3(CreateHeap);
    IMPLEMENT_MOCK7(CreatePlacedResource);
    IMPLEMENT_MOCK5(CreateReservedResource);
    IMPLEMENT_MOCK5(CreateSharedHandle);
    IMPLEMENT_MOCK3(OpenSharedHandle);
    IMPLEMENT_MOCK3(OpenSharedHandleByName);
    IMPLEMENT_MOCK2(MakeResident);
    IMPLEMENT_MOCK2(Evict);
    IMPLEMENT_MOCK4(CreateFence);
    IMPLEMENT_MOCK0(GetDeviceRemovedReason);
    IMPLEMENT_MOCK8(GetCopyableFootprints);
    IMPLEMENT_MOCK3(CreateQueryHeap);
    IMPLEMENT_MOCK1(SetStablePowerState);
    IMPLEMENT_MOCK4(CreateCommandSignature);
    IMPLEMENT_MOCK7(GetResourceTiling);
    MAKE_MOCK1(GetAdapterLuid, LUID*(LUID*), override);
};

class D3D12DeviceExtMock : public mock_interface<ID3D12DeviceExt>
{
    MAKE_MOCK2(QueryInterface, HRESULT(REFIID, void **), override);
    MAKE_MOCK0(AddRef, ULONG(), override);
    MAKE_MOCK0(Release, ULONG(), override);
    IMPLEMENT_MOCK3(GetVulkanHandles);
    IMPLEMENT_MOCK1(GetExtensionSupport);
    IMPLEMENT_MOCK7(CreateCubinComputeShaderWithName);
    IMPLEMENT_MOCK1(DestroyCubinComputeShader);
    IMPLEMENT_MOCK3(GetCudaTextureObject);
    IMPLEMENT_MOCK2(GetCudaSurfaceObject);
    IMPLEMENT_MOCK1(CaptureUAVInfo);
};

class D3D12GraphicsCommandListMock : public mock_interface<ID3D12GraphicsCommandList>
{
    MAKE_MOCK2(QueryInterface, HRESULT(REFIID, void **), override);
    MAKE_MOCK0(AddRef, ULONG(), override);
    MAKE_MOCK0(Release, ULONG(), override);
    IMPLEMENT_MOCK3(GetPrivateData);
    IMPLEMENT_MOCK3(SetPrivateData);
    IMPLEMENT_MOCK2(SetPrivateDataInterface);
    IMPLEMENT_MOCK1(SetName);
    IMPLEMENT_MOCK2(GetDevice);
    IMPLEMENT_MOCK0(GetType);
    IMPLEMENT_MOCK6(ExecuteIndirect);
    IMPLEMENT_MOCK0(EndEvent);
    IMPLEMENT_MOCK3(BeginEvent);
    IMPLEMENT_MOCK3(SetMarker);
    IMPLEMENT_MOCK6(ResolveQueryData);
    IMPLEMENT_MOCK3(EndQuery);
    IMPLEMENT_MOCK3(BeginQuery);
    IMPLEMENT_MOCK2(DiscardResource);
    IMPLEMENT_MOCK6(ClearUnorderedAccessViewFloat);
    IMPLEMENT_MOCK6(ClearUnorderedAccessViewUint);
    IMPLEMENT_MOCK4(ClearRenderTargetView);
    IMPLEMENT_MOCK6(ClearDepthStencilView);
    IMPLEMENT_MOCK4(OMSetRenderTargets);
    IMPLEMENT_MOCK3(SOSetTargets);
    IMPLEMENT_MOCK3(IASetVertexBuffers);
    IMPLEMENT_MOCK1(IASetIndexBuffer);
    IMPLEMENT_MOCK2(SetGraphicsRootUnorderedAccessView);
    IMPLEMENT_MOCK2(SetComputeRootUnorderedAccessView);
    IMPLEMENT_MOCK2(SetGraphicsRootShaderResourceView);
    IMPLEMENT_MOCK2(SetComputeRootShaderResourceView);
    IMPLEMENT_MOCK2(SetGraphicsRootConstantBufferView);
    IMPLEMENT_MOCK2(SetComputeRootConstantBufferView);
    IMPLEMENT_MOCK4(SetGraphicsRoot32BitConstants);
    IMPLEMENT_MOCK4(SetComputeRoot32BitConstants);
    IMPLEMENT_MOCK3(SetGraphicsRoot32BitConstant);
    IMPLEMENT_MOCK3(SetComputeRoot32BitConstant);
    IMPLEMENT_MOCK2(SetGraphicsRootDescriptorTable);
    IMPLEMENT_MOCK2(SetComputeRootDescriptorTable);
    IMPLEMENT_MOCK1(SetGraphicsRootSignature);
    IMPLEMENT_MOCK1(SetComputeRootSignature);
    IMPLEMENT_MOCK2(SetDescriptorHeaps);
    IMPLEMENT_MOCK1(ExecuteBundle);
    IMPLEMENT_MOCK2(ResourceBarrier);
    IMPLEMENT_MOCK1(SetPipelineState);
    IMPLEMENT_MOCK1(OMSetStencilRef);
    IMPLEMENT_MOCK1(OMSetBlendFactor);
    IMPLEMENT_MOCK2(RSSetScissorRects);
    IMPLEMENT_MOCK2(RSSetViewports);
    IMPLEMENT_MOCK1(IASetPrimitiveTopology);
    IMPLEMENT_MOCK5(ResolveSubresource);
    IMPLEMENT_MOCK6(CopyTiles);
    IMPLEMENT_MOCK2(CopyResource);
    IMPLEMENT_MOCK6(CopyTextureRegion);
    IMPLEMENT_MOCK5(CopyBufferRegion);
    IMPLEMENT_MOCK3(Dispatch);
    IMPLEMENT_MOCK5(DrawIndexedInstanced);
    IMPLEMENT_MOCK4(DrawInstanced);
    IMPLEMENT_MOCK1(ClearState);
    IMPLEMENT_MOCK2(Reset);
    IMPLEMENT_MOCK0(Close);
    IMPLEMENT_MOCK3(SetPredication);
};

class D3D12GraphicsCommandListExtMock : public mock_interface<ID3D12GraphicsCommandListExt>
{
    MAKE_MOCK2(QueryInterface, HRESULT(REFIID, void **), override);
    MAKE_MOCK0(AddRef, ULONG(), override);
    MAKE_MOCK0(Release, ULONG(), override);
    IMPLEMENT_MOCK1(GetVulkanHandle);
    IMPLEMENT_MOCK6(LaunchCubinShader);
};
