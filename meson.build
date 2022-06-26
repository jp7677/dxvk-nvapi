project('dxvk-nvapi',
  ['cpp'],
  default_options: [
    'cpp_std=c++17',
    'warning_level=2'
  ],
  version : 'v0.0',
  meson_version : '>= 0.58.0')

dxvk_compiler = meson.get_compiler('cpp')
add_project_arguments(dxvk_compiler.get_supported_arguments('-Wno-unused-parameter'), language : 'cpp')
if get_option('buildtype') == 'debug' or get_option('buildtype') == 'debugoptimized'
  add_project_arguments(dxvk_compiler.get_supported_arguments('-gstrict-dwarf'), language : 'cpp')
  add_project_arguments(dxvk_compiler.get_supported_arguments('-gdwarf-2'), language : 'cpp')
  add_project_arguments(dxvk_compiler.get_supported_arguments('-Wa,-mbig-obj'), language : 'cpp')
endif

add_project_link_arguments(dxvk_compiler.get_supported_link_arguments('-Wl,--file-alignment=4096'), language: 'cpp')

dxvk_cpu_family = target_machine.cpu_family()
if dxvk_cpu_family == 'x86_64'
  target_suffix = '64'
else
  target_suffix = ''
endif

vk_headers = include_directories('./external/Vulkan-Headers/include')

lib_dxgi = dxvk_compiler.find_library('dxgi')

dxvk_nvapi_version = vcs_tag(
  command: ['git', 'describe', '--always', '--tags', '--dirty=+'],
  input:  'version.h.in',
  output: 'version.h')

subdir('src')

enable_tests = get_option('enable_tests')
if enable_tests and dxvk_cpu_family == 'x86_64' # compiling tests for x86 fails, dunno why...
  subdir('tests')
endif
