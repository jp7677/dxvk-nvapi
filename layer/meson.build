project(
    'dxvk-nvapi-vkreflex-layer',
    ['cpp'],
    default_options: ['cpp_std=c++20'],
    version: 'v0.8.0',
    meson_version: '>= 1.0',
)

version = vcs_tag(
    command: ['git', 'describe', '--always', '--tags', '--dirty=+'],
    input: '../version.h.in',
    output: 'version.h',
)

cpp = meson.get_compiler('cpp')

config = configure_file(
    configuration: {
        'BUILD_COMPILER': cpp.get_id(),
        'BUILD_COMPILER_VERSION': cpp.version(),
        'BUILD_TARGET': target_machine.cpu_family(),
        'BUILD_TYPE': get_option('buildtype'),
    },
    input: '../config.h.in',
    output: 'config.h',
)

vk_headers = include_directories('../external/Vulkan-Headers/include')
vkroots = include_directories('../external/vkroots')

vkreflex_layer = shared_library(
    'dxvk_nvapi_vkreflex_layer',
    sources: ['vulkan_reflex_layer.cpp', config, version],
    include_directories: [vk_headers, vkroots],
    install: true,
)

fs = import('fs')

configure_file(
    configuration: {
        'library_path_prefix': get_option('absolute_library_path') ? (fs.as_posix(get_option('prefix') / get_option('libdir')) + '/') : get_option('library_path_prefix'),
        'vkreflex_library_name': fs.name(vkreflex_layer.full_path()),
    },
    input: 'VkLayer_DXVK_NVAPI_reflex.json.in',
    output: 'VkLayer_DXVK_NVAPI_reflex.json',
    install: true,
    install_dir: get_option('manifest_install_dir'),
)
