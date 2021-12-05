# [DXVK-NVAPI](https://github.com/jp7677/dxvk-nvapi)

## Alternative NVAPI implementation on top of [DXVK](https://github.com/doitsujin/dxvk)

This [repository](https://github.com/jp7677/dxvk-nvapi) provides an alternative implementation of NVIDIA's NVAPI library for usage with DXVK and [VKD3D-Proton](https://github.com/HansKristian-Work/vkd3d-proton). Its way of working is similar to [DXVK-AGS](https://github.com/doitsujin/dxvk-ags), but adjusted and enhanced for NVAPI.

This implementation currently offers entry points for supporting the following features in applications:

- NVIDIA DLSS for Vulkan, by supporting the relevant adapter information by querying from [Vulkan](https://www.vulkan.org/).
- NVIDIA DLSS for D3D11 and D3D12, by querying from Vulkan and forwarding the relevant calls into DXVK / VKD3D-Proton.
- Several NVAPI D3D11 extensions, among others `SetDepthBoundsTest` and `UAVOverlap`, by forwarding the relevant calls into DXVK.
- Several GPU topology related methods for adapter and display information, by querying from DXVK and Vulkan.

While originally being developed for usage with Unreal Engine 4, most notably for `Assetto Corsa Competizione`, more entry points have been added for enabling other NVIDIA related technologies. DXVK-NVAPI does not and will never cover the complete surface of NVIDIA's NVAPI. It is not meant as a full replacement, but rather as an addition to DXVK and VKD3D-Proton to enable several GPU features.

## Requirements

This implementation is supposed to be used on Linux using Wine or derivatives like Proton. Usage on Windows is discouraged. Please do not replace `nvapi.dll`/`nvapi64.dll` on Windows from NVIDIA's driver package with this version. DXVK-NVAPI uses several DXVK and VKD3D-Proton extension points, thus using DXVK (D3D11 and DXGI) is a requirement. Using Wine's D3D11 or DXGI will fail. Usage of DXVK-NVAPI is not restricted to NVIDIA GPUs, though the default behavior is to skip GPUs not running the NVIDIA proprietary driver. Some entry points offer no functionality or make no sense when a different GPU vendor is detected.

When available, DXVK-NVAPI uses NVIDIA's NVML management library to query temperature, utilization and others for NVIDIA GPUs. See [wine-nvml](https://github.com/Saancreed/wine-nvml) how to add NVML support to Wine/Proton.

## How to build

Like DXVK, this library is being built as a Windows DLL using MinGW, DXVK-NVAPI requires MinGW 9, MinGW 8 unfortunately has an outdated D3D12 header. Python 3 is needed for a prebuild validation script. This project uses git submodules, ensure to fetch them while or after cloning the repository.

Run:

```bash
./package-release.sh master /your/path [--enable-tests]
```

Alternatively [DXVK-Docker](https://github.com/jp7677/dxvk-docker) provides a way for a build setup using docker/podman.
Prebuilt binaries of release versions are available at <https://github.com/jp7677/dxvk-nvapi/releases>.

## How to use

Using DXVK-NVAPI in Proton, Lutris or Wine requires DXVK to see the GPU as an NVIDIA GPU.

- Spoof an NVIDIA GPU when running a non-NVIDIA GPU with `dxgi.customVendorId = 10de` set in a DXVK configuration file, see [dxvk.conf](https://github.com/doitsujin/dxvk/blob/master/dxvk.conf#L31).

### Proton

Proton 6.3-6 and newer includes DXVK-NVAPI but it is disabled by default.

- Use `PROTON_ENABLE_NVAPI=1` as game launch argument in Steam to enable DXVK-NVAPI. Use additionally `PROTON_HIDE_NVIDIA_GPU=0` for certain titles that don't rely on DXGI for obtaining the current GPU type.
- Disable the `nvapiHack` in DXVK with `dxgi.nvapiHack = False` set in a DXVK configuration file, see [dxvk.conf](https://github.com/doitsujin/dxvk/blob/master/dxvk.conf#L51).
- Copy and replace  `nvapi.dll`/`nvapi64.dll` into the `dist/lib/wine/nvapi`/`dist/lib64/wine/nvapi` folder of your Proton installation, e.g. in `~/.steam/steam/steamapps/common/Proton 6.3/` if you want to manually update the included version.

### Lutris

DXVK-NVAPI is enabled by default since Lutris 0.5.9. It's updated automatically.

- To use a custom version, copy a folder with that version containing `x32`/`x64` with `nvapi.dll`/`nvapi64.dll` of corresponding architecture inside them, into `~/.local/share/lutris/runtime/dxvk-nvapi/`. Edit the "DXVK-NVAPI version" field in the Lutris settings to the name of that folder.

### Wine

Wine does not includes DXVK-NVAPI.

- Copy `nvapi.dll`/`nvapi64.dll` into the `syswow64`/`system32` folder of your x86/x64 Wine prefix.
- Ensure that Wine uses the native version of `nvapi`/`nvapi64`, e.g. with `WINEDLLOVERRIDES=nvapi64,nvapi=n`.
- Ensure that DXVK is installed in your x86/x64 Wine prefix.
- Ensure that Wine uses DXVK's `dxgi.dll`, e.g. with `WINEDLLOVERRIDES=dxgi=n`.
- Disable the `nvapiHack` in DXVK with `dxgi.nvapiHack = False` set in a DXVK configuration file, see [dxvk.conf](https://github.com/doitsujin/dxvk/blob/master/dxvk.conf#L51).

## Tweaks, debugging and troubleshooting

See [Quirks-for-usage-with-DXVK-NVAPI](https://github.com/jp7677/dxvk-nvapi/wiki/Quirks-for-usage-with-DXVK-NVAPI) for any known quirks or work arounds.

The following environment variables tweak DXVK-NVAPI's runtime behavior:

- `DXVK_NVAPI_DRIVER_VERSION` lets you override the reported driver version. Valid values are numbers between 100 and 99999. Use e.g. `DXVK_NVAPI_DRIVER_VERSION=47141` to report driver version `471.41`.
- `DXVK_NVAPI_ALLOW_OTHER_DRIVERS` - when set, lets you use DXVK-NVAPI without running an NVIDIA card on NVIDIA's proprietary driver. Useful for using D3D11 extensions on a non-NVIDIA GPU.
- `DXVK_NVAPI_LOG_LEVEL` set to `info` prints log statements. The default behavior omits any logging. Please fill an issue if using log servery `info` creates log spam. There are no other log levels.
- `DXVK_NVAPI_LOG_PATH` enables file logging additionally to console output and sets the path where the log file `dxvk-nvapi.log` should be written to. Log statements are appended to an existing file. Please remove this file once in a while to prevent excessive grow. This requires `DXVK_NVAPI_LOG_LEVEL` set to `info`.

This project provides a test suite. Run the package script with `--enable-tests` (see above) to build `nvapi64-tests.exe`. Running the tests executable without arguments queries the local system and provides system information about visible GPUs:

```bash
DXVK_LOG_LEVEL=none WINEDEBUG=-all WINEDLLOVERRIDES=dxgi,nvapi64=n wine nvapi64-tests.exe
```

The test executable also runs on Windows against NVIDIA's `nvapi64.dll`. Ensure that DXVK-NVAPI's `nvapi64.dll`is not present in the current `PATH` for this scenario.

The actual unit tests can be run with `nvapi64-tests.exe [@unit-tests]` to validate DXVK-NVAPI's internal implementation.

## References and inspirations

- [DXVK](https://github.com/doitsujin/dxvk)
- [DXVK-AGS](https://github.com/doitsujin/dxvk-ags)
- [VKD3D-Proton](https://github.com/HansKristian-Work/vkd3d-proton)
- [NVAPI](https://docs.nvidia.com/gameworks/content/gameworkslibrary/coresdk/nvapi/group__dx.html)
- [NVAPI Open Source SDK](https://download.nvidia.com/XFree86/nvapi-open-source-sdk/)
- [NVAPI (wine-staging) standalone](https://github.com/SveSop/nvapi_standalone)

Many thanks to the corresponding authors!
