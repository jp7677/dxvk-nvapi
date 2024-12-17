project('dxvk-nvapi-vkreflex-layer', ['cpp'], default_options: ['cpp_std=c++20'])

vk_headers = include_directories('../external/Vulkan-Headers/include')
vkroots = include_directories('../external/vkroots')

vkreflex_layer = shared_library(
    'dxvk_nvapi_vkreflex_layer',
    sources: ['vulkan_reflex_layer.cpp'],
    include_directories: [vk_headers, vkroots],
    install: true,
)

fs = import('fs')

configure_file(
    configuration: {'vkreflex_library_name': fs.name(vkreflex_layer.full_path())},
    input: 'VkLayer_DXVK_NVAPI_reflex.json.in',
    output: 'VkLayer_DXVK_NVAPI_reflex.json',
    install: true,
    install_dir: 'share/vulkan/implicit_layer.d',
)