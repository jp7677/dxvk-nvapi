catch2_src = files(['../inc/catch_amalgamated.cpp'])

nvapi_src = files([
  '../src/util/util_string.cpp',
  '../src/util/util_env.cpp',
  '../src/util/util_log.cpp',
  '../src/util/util_drs.cpp',
  '../src/shared/vk.cpp',
  '../src/shared/resource_factory.cpp',
  '../src/nvapi/nvml.cpp',
  '../src/nvapi/low_latency_frame_id_generator.cpp',
  '../src/nvapi/nvapi_resource_factory.cpp',
  '../src/nvapi/nvapi_d3d_low_latency_device.cpp',
  '../src/nvapi/nvapi_vulkan_low_latency_device.cpp',
  '../src/nvapi/nvapi_output.cpp',
  '../src/nvapi/nvapi_adapter.cpp',
  '../src/nvapi/nvapi_adapter_registry.cpp',
  '../src/nvapi/nvapi_d3d11_device.cpp',
  '../src/nvapi/nvapi_d3d12_device.cpp',
  '../src/nvapi/nvapi_d3d12_graphics_command_list.cpp',
  '../src/nvapi/nvapi_d3d12_command_queue.cpp',
  '../src/nvapi_globals.cpp',
  '../src/nvapi.cpp',
  '../src/nvapi_d3d.cpp',
  '../src/nvapi_d3d11.cpp',
  '../src/nvapi_d3d12.cpp',
  '../src/nvapi_disp.cpp',
  '../src/nvapi_drs.cpp',
  '../src/nvapi_gpu.cpp',
  '../src/nvapi_mosaic.cpp',
  '../src/nvapi_ngx.cpp',
  '../src/nvapi_sys.cpp',
  '../src/nvapi_vulkan.cpp',
])

nvapi_tests_src = files([
  'nvapi_main.cpp',
  'nvapi/resource_factory_util.cpp',
  'nvapi_d3d.cpp',
  'nvapi_d3d11.cpp',
  'nvapi_d3d12.cpp',
  'nvapi_drs.cpp',
  'nvapi_ngx.cpp',
  'nvapi_sysinfo.cpp',
  'nvapi_sysinfo_topo.cpp',
  'nvapi_sysinfo_nvml.cpp',
  'nvapi_sysinfo_hdr.cpp',
  'nvapi_system.cpp',
  'nvapi_vulkan.cpp',
  'util.cpp',
])

target_name = 'nvapi'+target_suffix+'-tests'
nvapi_exe = executable(target_name, [ nvapi_src, catch2_src, nvapi_tests_src, dxvk_nvapi_version ],
  cpp_args            : '-DDXVK_NVAPI_TARGET_NAME="'+target_name+'"',
  dependencies        : [ lib_dxgi, lib_d3d11, lib_version ],
  include_directories : [ nvapi_headers, vk_headers ],
  install             : true)

nvofapi_src = files([
  '../src/util/util_string.cpp',
  '../src/util/util_env.cpp',
  '../src/util/util_log.cpp',
  '../src/shared/resource_factory.cpp',
  '../src/shared/vk.cpp',
  '../src/nvofapi/nvofapi_image.cpp',
  '../src/nvofapi/nvofapi_instance.cpp',
  '../src/nvofapi/nvofapi_d3d12_instance.cpp',
  '../src/nvofapi/nvofapi_vk_instance.cpp',
  '../src/nvofapi_globals.cpp',
  '../src/nvofapi.cpp',
  '../src/nvofapi_d3d11.cpp',
  '../src/nvofapi_d3d12.cpp',
  '../src/nvofapi_vk.cpp',
  '../src/nvofapi_cuda.cpp',
])

nvofapi_tests_src = files([
  'nvofapi_main.cpp',
  'nvofapi_d3d11.cpp',
  'nvofapi_d3d12.cpp',
  'nvofapi_vk.cpp',
  'nvofapi_cuda.cpp',
])

target_name = 'nvofapi'+target_suffix+'-tests'
nvofapi_exe = executable(target_name, [ nvofapi_src, catch2_src, nvofapi_tests_src, dxvk_nvapi_version ],
  cpp_args            : '-DDXVK_NVAPI_TARGET_NAME="'+target_name+'"',
  dependencies        : [ lib_version ],
  include_directories : [ nvapi_headers, vk_headers ],
  install             : true)
