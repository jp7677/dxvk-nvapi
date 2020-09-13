nvapi_src = files([
  'dxvk/dxvk_interfaces.cpp',
  'util/util_string.cpp',
  'sysinfo/nvapi_output.cpp',
  'sysinfo/nvapi_adapter.cpp',
  'sysinfo/nvapi_adapter_registry.cpp',
  'nvapi_interface.cpp',
])

nvapi_dll = shared_library('nvapi'+dll_suffix, nvapi_src,
  name_prefix         : '',
  dependencies        : [ lib_d3d11, lib_dxgi, lib_vulkan ],
  install             : true)
