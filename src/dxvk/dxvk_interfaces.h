#pragma once

#define DXVK_DEFINE_GUID(iface) \
  template<> inline GUID const& __mingw_uuidof<iface> () { return iface::guid; }

enum D3D11_VK_EXTENSION : uint32_t {
  D3D11_VK_EXT_MULTI_DRAW_INDIRECT        = 0,
  D3D11_VK_EXT_MULTI_DRAW_INDIRECT_COUNT  = 1,
  D3D11_VK_EXT_DEPTH_BOUNDS               = 2,
  D3D11_VK_EXT_BARRIER_CONTROL            = 3,
};

enum D3D11_VK_BARRIER_CONTROL : uint32_t {
  D3D11_VK_BARRIER_CONTROL_IGNORE_WRITE_AFTER_WRITE   = 1 << 0,
};

MIDL_INTERFACE("8a6e3c42-f74c-45b7-8265-a231b677ca17")
ID3D11VkExtDevice : public IUnknown {
  static const GUID guid;
  
  /**
   * \brief Checks whether an extension is supported
   * 
   * \param [in] Extension The extension to check
   * \returns \c TRUE if the extension is supported
   */
  virtual BOOL STDMETHODCALLTYPE GetExtensionSupport(
          D3D11_VK_EXTENSION      Extension) = 0;
  
};

MIDL_INTERFACE("fd0bca13-5cb6-4c3a-987e-4750de2ca791")
ID3D11VkExtContext : public IUnknown {
  static const GUID guid;
  
  virtual void STDMETHODCALLTYPE MultiDrawIndirect(
          UINT                    DrawCount,
          ID3D11Buffer*           pBufferForArgs,
          UINT                    ByteOffsetForArgs,
          UINT                    ByteStrideForArgs) = 0;
  
  virtual void STDMETHODCALLTYPE MultiDrawIndexedIndirect(
          UINT                    DrawCount,
          ID3D11Buffer*           pBufferForArgs,
          UINT                    ByteOffsetForArgs,
          UINT                    ByteStrideForArgs) = 0;
  
  virtual void STDMETHODCALLTYPE MultiDrawIndirectCount(
          UINT                    MaxDrawCount,
          ID3D11Buffer*           pBufferForCount,
          UINT                    ByteOffsetForCount,
          ID3D11Buffer*           pBufferForArgs,
          UINT                    ByteOffsetForArgs,
          UINT                    ByteStrideForArgs) = 0;
  
  virtual void STDMETHODCALLTYPE MultiDrawIndexedIndirectCount(
          UINT                    MaxDrawCount,
          ID3D11Buffer*           pBufferForCount,
          UINT                    ByteOffsetForCount,
          ID3D11Buffer*           pBufferForArgs,
          UINT                    ByteOffsetForArgs,
          UINT                    ByteStrideForArgs) = 0;
  
  virtual void STDMETHODCALLTYPE SetDepthBoundsTest(
          BOOL                    Enable,
          FLOAT                   MinDepthBounds,
          FLOAT                   MaxDepthBounds) = 0;
  
  virtual void STDMETHODCALLTYPE SetBarrierControl(
          UINT                    ControlFlags) = 0;
  
};

DXVK_DEFINE_GUID(ID3D11VkExtDevice);
DXVK_DEFINE_GUID(ID3D11VkExtContext);
