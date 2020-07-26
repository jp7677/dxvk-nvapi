project('dxvk-nvapi', ['cpp'], default_options: ['cpp_std=c++17'], version : 'v0.0', meson_version : '>= 0.43')

dxvk_compiler = meson.get_compiler('cpp')
dxvk_cpu_family = target_machine.cpu_family()
if (dxvk_cpu_family == 'x86_64')
  dll_suffix = '64'
else
  dll_suffix = ''
endif

add_project_arguments(dxvk_compiler.get_supported_arguments([
    '-Wno-attributes',
    '-Wno-non-virtual-dtor'
  ]),
  language : 'cpp')

lib_d3d11 = dxvk_compiler.find_library('d3d11')

dxvk_nvapi_version = vcs_tag(
  command: ['git', 'describe', '--always', '--tags', '--dirty=+'],
  input:  'version.h.in',
  output: 'version.h')

subdir('src')
