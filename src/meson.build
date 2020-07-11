nvapi_src = files([
  'nvapi_interface.cpp',
  'dxvk/dxvk_interfaces.cpp',
])

nvapi_dll = shared_library('nvapi'+dll_suffix, nvapi_src,
  name_prefix         : '',
  dependencies        : [ lib_d3d11 ],
  install             : true)
