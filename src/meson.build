nvapi_src = files([
  'nvapi.cpp',
  
  'dxvk/dxvk_interfaces.cpp',
])

nvapi_dll = shared_library('nvapi'+dll_suffix, nvapi_src,
  name_prefix         : '',
  dependencies        : [ lib_dxgi, lib_d3d11 ],
  install             : true)