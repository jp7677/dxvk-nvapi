# [DXVK-NVAPI](https://github.com/jp7677/dxvk-nvapi)

## Alternative NVAPI implementation on top of [DXVK](https://github.com/doitsujin/dxvk)

This [repository](https://github.com/jp7677/dxvk-nvapi) provides an alternative implementation of NVIDIA's NVAPI/NVOFAPI library for usage with DXVK and [VKD3D-Proton](https://github.com/HansKristian-Work/vkd3d-proton). Its way of working is similar to [DXVK-AGS](https://github.com/doitsujin/dxvk-ags), but adjusted and enhanced for NVAPI.

This implementation currently offers entry points for supporting the following features in applications:

- NVIDIA DLSS for Vulkan, by supporting the relevant adapter information by querying from [Vulkan](https://www.vulkan.org/).
- NVIDIA DLSS for D3D11 and D3D12, by querying from Vulkan and forwarding the relevant calls into DXVK / VKD3D-Proton.
- NVIDIA Reflex, by forwarding the relevant calls into either DXVK / VKD3D-Proton or [LatencyFleX](https://github.com/ishitatsuyuki/LatencyFleX).
- Several NVAPI D3D11 extensions, among others `SetDepthBoundsTest` and `UAVOverlap`, by forwarding the relevant calls into DXVK.
- NVIDIA PhysX, by supporting entry points for querying PhysX capabilities.
- Several GPU topology related methods for adapter and display information, by querying from DXVK and Vulkan.

Note that DXVK-NVAPI does not implement DLSS, Reflex or PhysX. It mostly forwards the relevant calls.

While originally being developed for usage with Unreal Engine 4, most notably for `Assetto Corsa Competizione`, more entry points have been added over time for enabling other NVIDIA related technologies. DXVK-NVAPI does not and will never cover the complete surface of NVIDIA's NVAPI. It is not meant as a full replacement, but rather as an addition to DXVK and VKD3D-Proton to enable several GPU features.

## Requirements

This implementation is supposed to be used on Linux using Wine or derivatives like Proton. Usage on Windows is discouraged. Please do not replace `nvapi.dll`/`nvapi64.dll`/`nvofapi64.dll` on Windows from NVIDIA's driver package with this version. DXVK-NVAPI uses several DXVK and VKD3D-Proton extension points, thus using DXVK (D3D11 and DXGI) is a requirement. Using Wine's D3D11 or DXGI implementation will fail. Usage of DXVK-NVAPI is not restricted to NVIDIA GPUs, though the default behavior is to skip GPUs not running the NVIDIA proprietary driver or Mesa NVK. Some entry points offer no functionality or make no sense when a different GPU vendor is detected. DLSS requires an NVIDIA GPU, Turing or newer, running the proprietary driver.

When available, DXVK-NVAPI uses NVIDIA's NVML management library to query temperature, utilization and others for NVIDIA GPUs. See [wine-nvml](https://github.com/Saancreed/wine-nvml) how to add NVML support to Wine/Proton.

## How to build

Like DXVK, this library is being built as a Windows DLL using MinGW. DXVK-NVAPI requires MinGW-w64 compiler and headers version 9 or newer, the Meson build system at least version 0.58 and Python 3 (needed for a prebuild validation script). This project uses git submodules. Ensure to fetch the submodules while or after cloning the repository, e.g. with `git clone --recurse-submodules git@github.com:jp7677/dxvk-nvapi.git`.

Run:

```bash
./package-release.sh master /your/path [--enable-tests]
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

Using DXVK-NVAPI with other GPU vendors / drivers has very limited use. Outside of testing, only Reflex (LatencyFlex) or HDR entry points provide benefits. This requires DXVK to see the GPU as an NVIDIA GPU. Use `DXVK_CONFIG="dxgi.hideAmdGpu = True"` to spoof an AMD GPU as NVIDIA GPU. Use `DXVK_CONFIG="dxgi.customVendorId = 10de"` for generally spoofing an NVIDIA GPU. 

Setting `DXVK_NVAPI_ALLOW_OTHER_DRIVERS=1` is needed for successful DXVK-NVAPI initialization when using a driver other than the NVIDIA proprietary driver or Mesa NVK. The reported driver version on drivers other than the NVIDIA proprietary driver will be 999.99. Overriding the reported driver version is still recommended. The reported GPU arrchitecture for other vendors is always Pascal to prevent attempts to initialize DLSS. This behavior cannot be changed without modifying the source code.

## Tweaks, debugging and troubleshooting

See the [DXVK-NVAPI Wiki](https://github.com/jp7677/dxvk-nvapi/wiki) for common issues and workarounds.

The following environment variables tweak DXVK-NVAPI's runtime behavior:

- `DXVK_NVAPI_DRIVER_VERSION` lets you override the reported driver version. Valid values are numbers between 100 and 99999. Use e.g. `DXVK_NVAPI_DRIVER_VERSION=47141` to report driver version `471.41`.
- `DXVK_NVAPI_ALLOW_OTHER_DRIVERS`, when set to `1`, allows DXVK-NVAPI to initialize for a non-NVIDIA GPU.
- `DXVK_NVAPI_DISABLE_ENTRYPOINTS`, when set to a comma-separated list of NVAPI entrypoint names, will hide their implementation from the application. For example, `DXVK_NVAPI_DISABLE_ENTRYPOINTS=NvAPI_D3D11_BeginUAVOverlap,NvAPI_D3D11_EndUAVOverlap` will report D3D11 barrier control extensions as not available.
- `DXVK_NVAPI_GPU_ARCH`, when set to one of supported NVIDIA GPU architecture IDs will override reported GPU architecture. Currently supported values are:
  - `GK100` (Kepler)
  - `GM000` (Maxwell Gen1)
  - `GM200` (Maxwell Gen2)
  - `GP100` (Pascal)
  - `GV100` (Volta)
  - `TU100` (Turing)
  - `GA100` (Ampere)
  - `AD100` (Ada)
- `DXVK_NVAPI_LOG_LEVEL` set to `info` prints log statements. The default behavior omits any logging. Please fill an issue if using log servery `info` creates log spam. Setting severity to `trace` logs all entry points enter and exits, this has a severe effect on performance. All other log levels will be interpreted as `none`.
- `DXVK_NVAPI_LOG_PATH` enables file logging additionally to console output and sets the path where the log file `nvapi.log`/`nvapi64.log`/`nvofapi64.log` should be written to. Log statements are appended to an existing file. Please remove this file once in a while to prevent excessive grow. This requires `DXVK_NVAPI_LOG_LEVEL` set to `info` or `trace`.

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
