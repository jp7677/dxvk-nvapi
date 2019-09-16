project('dxvk-nvapi', ['cpp'], default_options: ['cpp_std=c++17'], version : 'v0.0', meson_version : '>= 0.43')

dxvk_compiler = meson.get_compiler('cpp')
dxvk_cpu_family = target_machine.cpu_family()
if (dxvk_cpu_family == 'x86_64')
  dll_suffix = '64'
else
  dll_suffix = ''
endif

lib_d3d11   = dxvk_compiler.find_library('d3d11')
lib_dxgi    = dxvk_compiler.find_library('dxgi')

dxvk_nvapi_version = vcs_tag(
  command: ['git', 'describe', '--always', '--dirty=+'],
  input:  'version.h.in',
  output: 'version.h')

subdir('src')