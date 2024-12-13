project('dxvk-nvapi',
  ['cpp'],
  default_options: [
    'cpp_std=c++20',
    'warning_level=2'
  ],
  version : 'v0.8.0',
  meson_version : '>= 0.58.0')

cpp = meson.get_compiler('cpp')

dxvk_is_msvc = cpp.get_argument_syntax() == 'msvc'

compiler_args = ['-DNOMINMAX', '-DWIN32_LEAN_AND_MEAN']
link_args = []
if not dxvk_is_msvc
  compiler_args += ['-Wno-unused-parameter']
  if get_option('buildtype') == 'debug' or get_option('buildtype') == 'debugoptimized'
    compiler_args += [
      '-gdwarf-4',
      '-Wa,-mbig-obj'
    ]
  endif

  link_args += [
    '-static',
    '-static-libgcc',
    '-static-libstdc++',
    '-Wl,--file-alignment=4096'
  ]
else
  compiler_args += ['-D_CRT_SECURE_NO_WARNINGS']
  link_args += ['/FILEALIGN:4096']
endif


add_project_arguments(cpp.get_supported_arguments(compiler_args), language: 'cpp')
add_project_link_arguments(cpp.get_supported_link_arguments(link_args), language: 'cpp')

dxvk_cpu_family = target_machine.cpu_family()
if dxvk_cpu_family == 'x86_64'
  target_suffix = '64'
else
  target_suffix = ''
endif

nvapi_headers = include_directories('./external/nvapi')
vk_headers = include_directories('./external/Vulkan-Headers/include')

lib_dxgi = cpp.find_library('dxgi')
lib_d3d11 = cpp.find_library('d3d11')
lib_version = cpp.find_library('version')

dxvk_nvapi_version = vcs_tag(
  command: ['git', 'describe', '--always', '--tags', '--dirty=+'],
  input:  'version.h.in',
  output: 'version.h')

conf_data = configuration_data()
conf_data.set('BUILD_COMPILER', cpp.get_id())
conf_data.set('BUILD_COMPILER_VERSION', cpp.version())
conf_data.set('BUILD_TARGET', dxvk_cpu_family)
conf_data.set('BUILD_TYPE', get_option('buildtype'))
configure_file(
  input : 'config.h.in',
  output : 'config.h',
  configuration : conf_data)

subdir('src')

enable_tests = get_option('enable_tests')
if enable_tests and dxvk_cpu_family == 'x86_64' # compiling tests for x86 fails, dunno why...
  subdir('tests')
endif
