# [DXVK-NVAPI](https://github.com/jp7677/dxvk-nvapi)

## Alternative NVAPI implementation on top of [DXVK](https://github.com/doitsujin/dxvk)

This [repository](https://github.com/jp7677/dxvk-nvapi) provides an alternative implementation of NVIDIA's NVAPI/NVOFAPI library for usage with DXVK and [VKD3D-Proton](https://github.com/HansKristian-Work/vkd3d-proton). Its way of working is similar to [DXVK-AGS](https://github.com/doitsujin/dxvk-ags), but adjusted and enhanced for NVAPI.

This implementation currently offers entry points for supporting the following features in applications:

- NVIDIA DLSS for Vulkan, by supporting the relevant adapter information by querying from [Vulkan](https://www.vulkan.org/).
- NVIDIA DLSS for D3D11 and D3D12, by querying from Vulkan and forwarding the relevant calls into DXVK / VKD3D-Proton.
- NVIDIA Reflex, by forwarding the relevant calls into either DXVK / VKD3D-Proton or its custom Vulkan layer.
- Several NVAPI D3D11 extensions, among others `SetDepthBoundsTest` and `UAVOverlap`, by forwarding the relevant calls into DXVK.
- NVIDIA PhysX, by supporting entry points for querying PhysX capabilities.
- Several GPU topology related methods for adapter and display information, by querying from DXVK and Vulkan.

Note that DXVK-NVAPI does not implement DLSS, Reflex or PhysX. It mostly forwards the relevant calls.

While originally being developed for usage with Unreal Engine 4, most notably for `Assetto Corsa Competizione`, more entry points have been added over time for enabling other NVIDIA related technologies. DXVK-NVAPI does not and will never cover the complete surface of NVIDIA's NVAPI. It is not meant as a full replacement, but rather as an addition to DXVK and VKD3D-Proton to enable several GPU features.

## Requirements

This implementation is supposed to be used on Linux using Wine or derivatives like Proton. Usage on Windows is discouraged. Please do not replace `nvapi.dll`/`nvapi64.dll`/`nvofapi64.dll` on Windows from NVIDIA's driver package with this version. DXVK-NVAPI uses several DXVK and VKD3D-Proton extension points, thus using DXVK (D3D11 and DXGI) is a requirement. Using Wine's D3D11 or DXGI implementation will fail. Usage of DXVK-NVAPI is not restricted to NVIDIA GPUs, though the default behavior is to skip GPUs not running the NVIDIA proprietary driver or Mesa NVK. Some entry points offer no functionality or make no sense when a different GPU vendor is detected. DLSS requires an NVIDIA GPU, Turing or newer, running the proprietary driver.

In order to support Vulkan flavor of NVIDIA Reflex, an [additional Vulkan layer](./layer) is required which intercepts (technically invalid) Vulkan calls made by DXVK-NVAPI, enriches them with extra data, and forwards to a Vulkan driver that supports [`VK_NV_low_latency2`](https://registry.khronos.org/vulkan/specs/latest/man/html/VK_NV_low_latency2.html) device extension.

When available, DXVK-NVAPI uses NVIDIA's NVML management library to query temperature, utilization and others for NVIDIA GPUs. See [wine-nvml](https://github.com/Saancreed/wine-nvml) how to add NVML support to Wine/Proton.

## How to build

Like DXVK, this library is being built as a Windows DLL using MinGW. DXVK-NVAPI requires MinGW-w64 compiler and headers version 9 or newer, the Meson build system at least version 0.58 and Python 3 (needed for a prebuild validation script). This project uses git submodules. Ensure to fetch the submodules while or after cloning the repository, e.g. with `git clone --recurse-submodules git@github.com:jp7677/dxvk-nvapi.git`.

The Vulkan Reflex layer has higher requirements: Meson 1.0 and a C++ compiler that supports C++20 standard. It is recommended (and assumed) that the layer is built as Linux-side library, and as such this compiler should target Linux and not be a cross-compiler like MinGW.

Run:

```bash
./package-release.sh master /your/path [--disable-layer] [--enable-tests]
```

Alternatively [DXVK-Docker](https://github.com/jp7677/dxvk-docker) provides a way for a build setup using docker/podman.
Prebuilt binaries of release versions are available at <https://github.com/jp7677/dxvk-nvapi/releases>. GitHub Actions is set up for building development snapshots.

## How to use

Support for DXVK-NVAPI has been added to popular game launchers. See their respective documentation for usage and updates of DXVK-NVAPI.

### Steam Play / Proton

Proton 9.0 includes DXVK-NVAPI and enables it by default for all titles with a few exceptions. Proton 9.0 can force-enable DXVK-NVAPI for titles that have DXVK-NVAPI disabled by default and also for non-NVIDIA GPUs when setting `PROTON_FORCE_NVAPI=1`. Contrary, `PROTON_DISABLE_NVAPI` disables DXVK-NVAPI.

- Copy and replace `nvapi.dll`/`nvapi64.dll`/`nvofapi64.dll` into the `dist/lib/wine/nvapi`/`dist/lib64/wine/nvapi` folder of your Proton installation, e.g. in `~/.steam/steam/steamapps/common/Proton 9.0/` for manually updating the included version.

Proton Experimental Bleeding Edge additionally always contains the latest DXVK-NVAPI development (master) version.

Proton 8.0 includes DXVK-NVAPI and enables it by default for a lot of titles. Use `PROTON_ENABLE_NVAPI=1` for that version as game launch argument in Steam to enable DXVK-NVAPI for other titles.

### Wine / Wine-Staging

Wine does not includes DXVK-NVAPI.

- Copy `nvapi.dll`/`nvapi64.dll`/`nvofapi64.dll` into the `syswow64`/`system32` folder of your x86/x64 Wine prefix.
- Ensure that Wine uses the native version of `nvapi`/`nvapi64`, e.g. with `WINEDLLOVERRIDES=nvapi,nvapi64=n`. This applies only to Wine versions that ship their own NVAPI implementation, e.g. Wine-Staging older than 7.22. Generally this is no longer needed nowadays.
- Ensure that DXVK is installed in your x86/x64 Wine prefix.
- Ensure that Wine uses DXVK's `dxgi.dll`, e.g. with `WINEDLLOVERRIDES=dxgi=n`.
- Set `DXVK_ENABLE_NVAPI=1` to disable DXVK's `nvapiHack` in DXVK.

DXVK 1.10 and older does not support `DXVK_ENABLE_NVAPI`. Disable the `nvapiHack` in DXVK 1.10 and older with `dxgi.nvapiHack = False` set in a DXVK configuration file, see [dxvk.conf](https://github.com/doitsujin/dxvk/blob/master/dxvk.conf).

### Non-NVIDIA GPU

Using DXVK-NVAPI with other GPU vendors / drivers has very limited use. Outside of testing, only HDR entry points provide benefits. This requires DXVK to see the GPU as an NVIDIA GPU. Use `DXVK_CONFIG="dxgi.hideAmdGpu = True"` to spoof an AMD GPU as NVIDIA GPU. Use `DXVK_CONFIG="dxgi.customVendorId = 10de"` for generally spoofing an NVIDIA GPU.

Setting `DXVK_NVAPI_ALLOW_OTHER_DRIVERS=1` is needed for successful DXVK-NVAPI initialization when using a driver other than the NVIDIA proprietary driver or Mesa NVK. The reported driver version on drivers other than the NVIDIA proprietary driver will be 999.99. Overriding the reported driver version is still recommended. The reported GPU architecture for other vendors is, unless overriden, always Pascal to prevent attempts to initialize DLSS.

### Vulkan Reflex layer

The Vulkan Reflex layer, when built as a Linux-side layer, is not installed into a Wine prefix. Instead it's installed like any other Vulkan implicit layer. It is distributed as a pair of files:

- `VkLayer_DXVK_NVAPI_reflex.json` (the layer manifest)
- `libdxvk_nvapi_vkreflex_layer.so` (the layer library)

To make it discoverable by your Vulkan loader:

1. Do one of the following:
    - Export full path to the directory containing the layer manifest in `VK_ADD_IMPLICIT_LAYER_PATH` environment variable (e.g. `VK_ADD_IMPLICIT_LAYER_PATH="${HOME}/dxvk-nvapi/layer"`)
        - Note that `VK_ADD_IMPLICIT_LAYER_PATH` works only with Vulkan loader v1.3.296 or newer
        - If you already have `VK_ADD_IMPLICIT_LAYER_PATH` set and pointing to a directory with some other implicit layer manifest, it is possible to pass multiple paths by separating them with `:`
    - Install the layer manifest into one of default implicit layer search paths (e.g. `${XDG_DATA_HOME}/vulkan/implicit_layer.d` where Steam usually installs its layers)
2. If needed, edit the layer manifest to ensure that its `library_path` points to the layer library
     - For example, if the layer library is in the same directory as the layer manifest, the path should be `./libdxvk_nvapi_vkreflex_layer.so`
         - Note that the distributed layer manifest by default assumes this scenario; therefore there is no need to edit the manifest if the layer library is kept in the same directory as the manifest file
     - For other scenarios where the layer library is nearby, relative paths work as well (e.g. `../../../lib/libdxvk_nvapi_vkreflex_layer.so`)
     - It's also possible to provide full path to the layer library (e.g. `/home/user/dxvk-nvapi/layer/libdxvk_nvapi_vkreflex_layer.so`)
     - Finally, if the layer library is in one of the paths searched by dynamic linker (like `/usr/lib`, this depends on the distro), it's possible to set the library path to just the name of the layer library: `"library_path": "libdxvk_nvapi_vkreflex_layer.so"`

Because the layer could interfere with other Vulkan applications that don't use Reflex, it is currently _disabled by default_ even when installed. To enable it, export `DXVK_NVAPI_VKREFLEX=1` as environment variable. When an application attempts to use Vulkan Reflex and the layer isn't installed and enabled, the following message should be logged at `info` log level: `info:nvapi64:<-NvAPI_Vulkan_InitLowLatencyDevice: Not supported`, with an explanation directing the user to verify the layer's setup.

Running `env DXVK_NVAPI_VKREFLEX=1 vulkaninfo` can serve as a quick setup check to verify that both files are in correct locations. If the layer manifest was found, there will be an entry like `VK_LAYER_DXVK_NVAPI_reflex (DXVK-NVAPI Vulkan Reflex compatibility layer)` on the list of discovered Vulkan layers. Additionally, if the layer manifest was found but the layer library couldn't be loaded (perhaps due to incorrect `library_path` in the manifest), an error similar to `ERROR: [Loader Message] Code 0 : /home/user/.local/share/vulkan/implicit_layer.d/./libdxvk_nvapi_vkreflex_layer.so: cannot open shared object file: No such file or directory` will be logged on stderr before any standard output.

Like any other (glibc-based) Linux shared library, layer compiled on a system with newer version of glibc will _not_ run on systems with older version of glibc. If you are running a distro with older glibc than the ones prebuilt binaries are distributed for, it is likely that building your own library from source will be required.

## Tweaks, debugging and troubleshooting

See the [DXVK-NVAPI Wiki](https://github.com/jp7677/dxvk-nvapi/wiki) for common issues and workarounds.

The following environment variables tweak DXVK-NVAPI's runtime behavior:

- `DXVK_NVAPI_DRIVER_VERSION` lets you override the reported driver version. Valid values are numbers between 100 and 99999. Use e.g. `DXVK_NVAPI_DRIVER_VERSION=47141` to report driver version `471.41`.
- `DXVK_NVAPI_ALLOW_OTHER_DRIVERS`, when set to `1`, allows DXVK-NVAPI to initialize for a non-NVIDIA GPU.
- `DXVK_NVAPI_DISABLE_ENTRYPOINTS`, when set to a comma-separated list of NVAPI entrypoint names, will hide their implementation from the application. For example, `DXVK_NVAPI_DISABLE_ENTRYPOINTS=NvAPI_D3D11_BeginUAVOverlap,NvAPI_D3D11_EndUAVOverlap` will report D3D11 barrier control extensions as not available.
- `DXVK_NVAPI_DRS_SETTINGS` allows providing custom values for arbitrary driver settings. Currently, only values of DWORD (u32) type are supported. Format is `setting1=value1,setting2=value2,…` where both `settingN` and `valueN` are unsigned 32-bit integers and can be prefixed with `0x` or `0X` to parse them as hexadecimal numbers. For example, `DXVK_NVAPI_DRS_SETTINGS=0x10E41DF3=0xffffff,0x10E41DF7=0xffffff` overrides presets for both DLSS Super Resolution and DLSS Ray Reconstruction to latest.
  - Additionally, the following settings are supported in their named/text form:
    - `NGX_DLAA_OVERRIDE`
    - `NGX_DLSSG_MULTI_FRAME_COUNT`
    - `NGX_DLSS_FG_OVERRIDE`
    - `NGX_DLSS_RR_MODE`
    - `NGX_DLSS_OVERRIDE_OPTIMAL_SETTINGS`
    - `NGX_DLSS_RR_OVERRIDE`
    - `NGX_DLSS_RR_OVERRIDE_RENDER_PRESET_SELECTION`
    - `NGX_DLSS_SR_MODE`
    - `NGX_DLSS_SR_OVERRIDE`
    - `NGX_DLSS_SR_OVERRIDE_RENDER_PRESET_SELECTION`
  - The corresponding values can be deduced from the [NVAPI `NvApiDriverSettings.h` header](https://github.com/NVIDIA/nvapi/blob/d08488fcc82eef313b0464db37d2955709691e94/NvApiDriverSettings.h#L700) with the setting type prefix omitted, for example `DXVK_NVAPI_DRS_SETTINGS=NGX_DLSS_FG_OVERRIDE=on`. Both setting name (`NGX_DLSS_FG_OVERRIDE`) and value (`on`) are case-insensitive.
  - Furthermore, each supported named setting can also be configured with a dedicated environment variable, the name of which is constructed by prefixing setting name with `DXVK_NVAPI_DRS_`, for example `DXVK_NVAPI_DRS_NGX_DLSS_FG_OVERRIDE=on`. The name of environment variable is case-sensitive, the value is not. If a setting is set via both `DXVK_NVAPI_DRS_SETTINGS` and its own dedicated environment variable, the latter takes precedence.
  - See the [Passing driver settings](https://github.com/jp7677/dxvk-nvapi/wiki/Passing-driver-settings) wiki page for more information and examples.
- `DXVK_NVAPI_GPU_ARCH`, when set to one of supported NVIDIA GPU architecture IDs will override reported GPU architecture. Currently supported values are:
  - `GK100` (Kepler)
  - `GM000` (Maxwell Gen1)
  - `GM200` (Maxwell Gen2)
  - `GP100` (Pascal)
  - `GV100` (Volta)
  - `TU100` (Turing)
  - `GA100` (Ampere)
  - `AD100` (Ada)
  - `GB200` (Blackwell)
- `DXVK_NVAPI_LOG_LEVEL` set to `info` prints log statements. The default behavior omits any logging. Please fill an issue if using log servery `info` creates log spam. Setting severity to `trace` logs all entry points enter and exits, this has a severe effect on performance. All other log levels will be interpreted as `none`.
- `DXVK_NVAPI_LOG_PATH` enables file logging additionally to console output and sets the path where the log file `nvapi.log`/`nvapi64.log`/`nvofapi64.log` should be written to. Log statements are appended to an existing file. Please remove this file once in a while to prevent excessive grow. This requires `DXVK_NVAPI_LOG_LEVEL` set to `info` or `trace`.
- `DXVK_NVAPI_SET_NGX_DEBUG_OPTIONS` allows to set various NGX debug registry keys with the format `setting1=value1,setting2=value2,…`, whereas values are of type DWORD (u32). Setting the registry keys for enabling DLSS indicators corresponds to `DXVK_NVAPI_SET_NGX_DEBUG_OPTIONS=DLSSIndicator=1024,DLSSGIndicator=2`, hiding the indicators to `DXVK_NVAPI_SET_NGX_DEBUG_OPTIONS=DLSSIndicator=0,DLSSGIndicator=0`. Be aware, this tweak permanently modifies the registry.

The following environment variables tweak DXVK-NVAPI's Vulkan Reflex layer's runtime behavior:

- `DXVK_NVAPI_VKREFLEX=1` enables the layer; the layer is disabled by default when this isn't set
- `DISABLE_DXVK_NVAPI_VKREFLEX` disables the layer (overriding `DXVK_NVAPI_VKREFLEX=1`) when set to any nonempty value
- `DXVK_NVAPI_VKREFLEX_LAYER_LOG_LEVEL` configures logging verbosity, supported values are `none` (default), `error`, `warn`, `info`, `trace`, `debug` and integer values from `0` to `5` inclusive that correspond to named log levels
- `DXVK_NVAPI_VKREFLEX_EXPOSE_EXTENSION=1` causes the layer to report support for `VK_NV_low_latency` device extension, this is usually unnecessary as modern NVIDIA drivers already claim support for it and exposing it from the layer can cause issues in some games (notably Indiana Jones and the Great Circle)
- `DXVK_NVAPI_VKREFLEX_INJECT_SUBMIT_FRAME_IDS=1` and `DXVK_NVAPI_VKREFLEX_INJECT_PRESENT_FRAME_IDS=1` cause the layer to inject frame IDs into `vkQueueSubmit*` and `vkQueuePresentKHR` Vulkan commands respectively based on the latency markers set by the application with `NvAPI_Vulkan_SetLatencyMarker`, possibly helping the driver correlate the calls but could interfere with application's own present IDs; enabling either enables `VK_KHR_present_id` device extension and related `presentID` feature
  - `DXVK_NVAPI_VKREFLEX_ALLOW_FALLBACK_TO_OOB_FRAME_ID=0` disables fallback to out-of-band frame IDs in submit and present calls
  - `DXVK_NVAPI_VKREFLEX_ALLOW_FALLBACK_TO_PRESENT_FRAME_ID=0` disables fallback to present frame IDs in submit calls
  - `DXVK_NVAPI_VKREFLEX_ALLOW_FALLBACK_TO_SIMULATION_FRAME_ID=0` disables fallback to simulation frame IDs in submit calls

Additionally, GitHub Actions provide build artifacts from different toolchains. In very rare situations a non-gcc build might provide better results.

Using those tweaks is exclusively meant for troubleshooting. Regular usage is discouraged and may result in unwanted side effects. Please open an issue if using one of the tweaks is mandatory for a specific title/scenario.

This project provides a test suite. Run the package script with `--enable-tests` (see above) to build `nvapi64-tests.exe`. Running the tests executable without arguments queries the local system and provides system information about visible GPUs:

```bash
DXVK_LOG_LEVEL=none WINEDEBUG=-all WINEDLLOVERRIDES=dxgi,nvapi64=n wine nvapi64-tests.exe
```

The test executable also runs on Windows against NVIDIA's `nvapi64.dll`. Ensure that DXVK-NVAPI's `nvapi64.dll`is not present in the current `PATH` for this scenario.

The actual unit tests can be run with `nvapi64-tests.exe [@unit-tests]` to validate DXVK-NVAPI's internal implementation.

Producing a debug build and starting a debugging session with the test suite can be achieved with the following snippet:

```bash
meson setup --reconfigure --cross-file "./build-win64.txt" --buildtype "debugoptimized" -Denable_tests=True build
meson compile -C build

DXVK_LOG_LEVEL=none DXVK_NVAPI_LOG_LEVEL=none WINEDEBUG=-all WINEDLLOVERRIDES=nvapi64=n WINEPATH=build/src winedbg --gdb build/tests/nvapi64-tests.exe [@all]
```

Once the debug session has started, use `c` to start/continue execution and a.o. `bt` to show a proper stacktrace after a segmentation fault. Ensure to have no other native `nvapi64.dll` in the Wine prefix, otherwise this one gets precedence over the one found in `WINEPATH`.

## References and inspirations

- [DXVK](https://github.com/doitsujin/dxvk)
- [DXVK-AGS](https://github.com/doitsujin/dxvk-ags)
- [VKD3D-Proton](https://github.com/HansKristian-Work/vkd3d-proton)
- [NVAPI](https://docs.nvidia.com/gameworks/content/gameworkslibrary/coresdk/nvapi/index.html)
- [NVAPI Open Source SDK](https://download.nvidia.com/XFree86/nvapi-open-source-sdk/)
- [NVAPI GitHub Project](https://github.com/NVIDIA/nvapi)
- [NVAPI (wine-staging) standalone](https://github.com/SveSop/nvapi_standalone)

Many thanks to the corresponding authors!
