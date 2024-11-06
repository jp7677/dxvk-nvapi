nvapi_src = files([
  'util/util_string.cpp',
  'util/util_env.cpp',
  'util/util_log.cpp',
  'nvapi/lfx.cpp',
  'nvapi/nvapi_d3d_instance.cpp',
  'nvapi/nvapi_d3d_low_latency_device.cpp',
  'nvapi/nvapi_d3d11_device.cpp',
  'nvapi/nvapi_d3d12_device.cpp',
  'nvapi/nvapi_adapter_registry.cpp',
  'nvapi/nvapi_adapter.cpp',
  'nvapi/nvapi_output.cpp',
  'nvapi/vulkan.cpp',
  'nvapi/nvml.cpp',
  'resource_factory.cpp',
  'nvapi_globals.cpp',
  'nvapi.cpp',
  'nvapi_d3d.cpp',
  'nvapi_d3d11.cpp',
  'nvapi_d3d12.cpp',
  'nvapi_disp.cpp',
  'nvapi_drs.cpp',
  'nvapi_gpu.cpp',
  'nvapi_mosaic.cpp',
  'nvapi_sys.cpp',
  'nvapi_interface.cpp',
])

nvapi_dll = shared_library('nvapi'+target_suffix, [ nvapi_src, dxvk_nvapi_version ],
  name_prefix         : '',
  dependencies        : [ lib_dxgi, lib_d3d11, lib_version ],
  include_directories : [ nvapi_headers, vk_headers ],
  install             : true)

nvofapi_src = files([
  'util/util_string.cpp',
  'util/util_env.cpp',
  'util/util_log.cpp',
  'nvofapi/nvofapi_image.cpp',
  'nvofapi/nvofapi_instance.cpp',
  'nvofapi/nvofapi_d3d12_instance.cpp',
  'nvofapi/nvofapi_vulkan_instance.cpp',
  'nvofapi.cpp',
  'nvofapi_d3d12.cpp',
  'nvofapi_vulkan.cpp',
])

# Only build 64-bit versions of nvofapi
if dxvk_cpu_family == 'x86_64'
  nvofapi_dll = shared_library('nvofapi'+target_suffix, [ nvofapi_src, dxvk_nvapi_version ],
    name_prefix         : '',
    dependencies        : [ lib_version ],
    include_directories : [ nvapi_headers, vk_headers ],
    install             : true)
endif
