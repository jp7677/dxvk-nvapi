nvapi_src = files([
  'util/util_string.cpp',
  'util/util_env.cpp',
  'util/util_log.cpp',
  'util/util_drs.cpp',
  'shared/vk.cpp',
  'shared/resource_factory.cpp',
  'nvapi/nvml.cpp',
  'nvapi/low_latency_frame_id_generator.cpp',
  'nvapi/nvapi_resource_factory.cpp',
  'nvapi/nvapi_d3d_low_latency_device.cpp',
  'nvapi/nvapi_vulkan_low_latency_device.cpp',
  'nvapi/nvapi_output.cpp',
  'nvapi/nvapi_adapter.cpp',
  'nvapi/nvapi_adapter_registry.cpp',
  'nvapi/nvapi_d3d11_device.cpp',
  'nvapi/nvapi_d3d12_device.cpp',
  'nvapi/nvapi_d3d12_graphics_command_list.cpp',
  'nvapi/nvapi_d3d12_command_queue.cpp',
  'nvapi_globals.cpp',
  'nvapi.cpp',
  'nvapi_d3d.cpp',
  'nvapi_d3d11.cpp',
  'nvapi_d3d12.cpp',
  'nvapi_disp.cpp',
  'nvapi_drs.cpp',
  'nvapi_gpu.cpp',
  'nvapi_mosaic.cpp',
  'nvapi_ngx.cpp',
  'nvapi_sys.cpp',
  'nvapi_vulkan.cpp',
  'nvapi_interface.cpp',
])

target_name = 'nvapi'+target_suffix
nvapi_dll = shared_library(target_name, [ nvapi_src, dxvk_nvapi_version ],
  cpp_args            : '-DDXVK_NVAPI_TARGET_NAME="'+target_name+'"',
  name_prefix         : '',
  dependencies        : [ lib_dxgi, lib_d3d11, lib_version ],
  include_directories : [ nvapi_headers, vk_headers ],
  vs_module_defs      : target_name+'.def',
  install             : true)

nvofapi_src = files([
  'util/util_string.cpp',
  'util/util_env.cpp',
  'util/util_log.cpp',
  'shared/resource_factory.cpp',
  'shared/vk.cpp',
  'nvofapi/nvofapi_image.cpp',
  'nvofapi/nvofapi_instance.cpp',
  'nvofapi/nvofapi_d3d12_instance.cpp',
  'nvofapi/nvofapi_vk_instance.cpp',
  'nvofapi_globals.cpp',
  'nvofapi.cpp',
  'nvofapi_d3d11.cpp',
  'nvofapi_d3d12.cpp',
  'nvofapi_vk.cpp',
  'nvofapi_cuda.cpp',
])

# Only build 64-bit versions of nvofapi
if dxvk_cpu_family == 'x86_64'
  target_name = 'nvofapi'+target_suffix
  nvofapi_dll = shared_library(target_name, [ nvofapi_src, dxvk_nvapi_version ],
    cpp_args            : '-DDXVK_NVAPI_TARGET_NAME="'+target_name+'"',
    name_prefix         : '',
    dependencies        : [ lib_version ],
    include_directories : [ nvapi_headers, vk_headers ],
    vs_module_defs      : target_name+'.def',
    install             : true)
endif
