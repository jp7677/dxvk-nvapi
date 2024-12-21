nvapi_src = files([
  '../src/util/util_string.cpp',
  '../src/util/util_env.cpp',
  '../src/util/util_log.cpp',
  '../src/nvapi/resource_factory.cpp',
  '../src/nvapi/vk.cpp',
  '../src/nvapi/nvml.cpp',
  '../src/nvapi/lfx.cpp',
  '../src/nvapi/nvapi_d3d_instance.cpp',
  '../src/nvapi/nvapi_d3d_low_latency_device.cpp',
  '../src/nvapi/nvapi_output.cpp',
  '../src/nvapi/nvapi_adapter.cpp',
  '../src/nvapi/nvapi_adapter_registry.cpp',
  '../src/nvapi/nvapi_d3d11_device.cpp',
  '../src/nvapi/nvapi_d3d12_device.cpp',
  '../src/nvapi_globals.cpp',
  '../src/nvapi.cpp',
  '../src/nvapi_d3d.cpp',
  '../src/nvapi_d3d11.cpp',
  '../src/nvapi_d3d12.cpp',
  '../src/nvapi_disp.cpp',
  '../src/nvapi_drs.cpp',
  '../src/nvapi_gpu.cpp',
  '../src/nvapi_mosaic.cpp',
  '../src/nvapi_sys.cpp',
])

catch2_src = files(['../inc/catch_amalgamated.cpp'])

nvapi_tests_src = files([
  'main.cpp',
  'resource_factory_util.cpp',
  'nvapi_d3d.cpp',
  'nvapi_d3d11.cpp',
  'nvapi_d3d12.cpp',
  'nvapi_drs.cpp',
  'nvapi_sysinfo.cpp',
  'nvapi_sysinfo_topo.cpp',
  'nvapi_sysinfo_nvml.cpp',
  'nvapi_sysinfo_hdr.cpp',
  'nvapi_system.cpp',
  'util.cpp',
])

target_name = 'nvapi'+target_suffix+'-tests'
nvapi_exe = executable(target_name, [ nvapi_src, catch2_src, nvapi_tests_src, dxvk_nvapi_version ],
  cpp_args            : '-DDXVK_NVAPI_TARGET_NAME="'+target_name+'"',
  dependencies        : [ lib_dxgi, lib_d3d11, lib_version ],
  include_directories : [ nvapi_headers, vk_headers ],
  install             : true)
