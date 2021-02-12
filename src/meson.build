nvapi_src = files([
  'dxvk/dxvk_interfaces.cpp',
  'util/util_string.cpp',
  'util/util_env.cpp',
  'util/util_log.cpp',
  'sysinfo/nvapi_output.cpp',
  'sysinfo/nvapi_adapter.cpp',
  'sysinfo/nvapi_adapter_registry.cpp',
  'd3d11/nvapi_d3d11_device.cpp',
  'nvapi_interface.cpp',
])

nvapi_dll = shared_library('nvapi'+dll_suffix, [ nvapi_src, dxvk_nvapi_version ],
  name_prefix         : '',
  dependencies        : [ lib_d3d11, lib_dxgi, lib_vulkan ],
  install             : true)
