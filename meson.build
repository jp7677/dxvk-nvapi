project('dxvk-nvapi', ['cpp'], default_options: ['cpp_std=c++17'], version : 'v0.0', meson_version : '>= 0.43')

dxvk_compiler = meson.get_compiler('cpp')

lib_d3d11   = dxvk_compiler.find_library('d3d11')
lib_dxgi    = dxvk_compiler.find_library('dxgi')

subdir('src')