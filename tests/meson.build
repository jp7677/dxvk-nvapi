nvapi_src = files([
  '../src/dxvk/dxvk_interfaces.cpp',
  '../src/vkd3d-proton/vkd3d-proton_interfaces.cpp',
  '../src/util/util_string.cpp',
  '../src/util/util_env.cpp',
  '../src/util/util_log.cpp',
  '../src/sysinfo/vulkan.cpp',
  '../src/sysinfo/nvml.cpp',
  '../src/sysinfo/nvapi_output.cpp',
  '../src/sysinfo/nvapi_adapter.cpp',
  '../src/sysinfo/nvapi_adapter_registry.cpp',
  '../src/sysinfo/resource_factory.cpp',
  '../src/d3d11/nvapi_d3d11_device.cpp',
  '../src/d3d12/nvapi_d3d12_device.cpp',
])

nvapi_tests_src = files([
  'main.cpp',
  'nvapi_tests.cpp',
  'nvapi_system.cpp',
])

nvapi_exe = executable('nvapi'+target_suffix+'-tests', [ nvapi_src, nvapi_tests_src, dxvk_nvapi_version ],
  dependencies        : [ lib_dxgi ],
  include_directories : [ vk_headers ],
  install             : true)
