# [DXVK-NVAPI](https://github.com/jp7677/dxvk-nvapi)

## Experimental non-complete NVAPI implementation on top of [DXVK](https://github.com/doitsujin/dxvk)

This [repository](https://github.com/jp7677/dxvk-nvapi) provides a basic alternative implementation of the NVAPI extensions for DXVK. Its way of working is very similar to [DXVK-AGS](https://github.com/doitsujin/dxvk-ags), but adjusted for NVAPI.

This implementation currently forwards the following NVAPI D3D11 features to DXVK:

- `SetDepthBoundsTest`
- `BeginUAVOverlap`/`EndUAVOverlap`
- `MultiDrawInstancedIndirect`/`MultiDrawIndexedInstancedIndirect`

It also implements several methods for adapter/display topology and system information. Some of those are a requirement for enabling other NVIDIA related technologies.

This implementation has been mostly tested with Unreal Engine 4, with the game `Assetto Corsa Competizione` and several UE4 technology demos. Unreal Engine 4 utilizes `SetDepthBoundsTest`, newer engine versions also utilize `BeginUAVOverlap`/`EndUAVOverlap`. Forwarding those methods may yield up to 4% extra performance, although this heavily depends on setup and scenery.

Basic topology and system information (vendor ID, driver version and others) has been mostly tested with `GPU Caps Viewer`. The game `Get Even`, which verifies the driver version during launch, starts fine with this implementation.

## Requirements

This implementation is supposed to be used on Linux using Wine or derivatives like Proton. It uses several DXVK extension points, thus using DXVK (D3D11 and DXGI) is a requirement. Using Wine's D3D11 or DXGI will fail. Usage of DXVK-NVAPI is not restricted to NVIDIA GPUs, no specific NVIDIA hardware features are needed.

When available, DXVK-NVAPI uses NVIDIA's NVML management library to query temperature, utilization and others for NVIDIA GPUs. See [wine-nvml](https://github.com/Saancreed/wine-nvml) how to add NVML support to Wine/Proton.

## How to build

Like DXVK, this library is being built as a Windows DLL using MinGW, and has essentially the same build requirements. Using MinGW versions older than MinGW 7 might fail. Python 3 is needed for a prebuild validation script.

Run:

```bash
./package-release.sh master /your/path [--enable-tests]
```

Alternatively [DXVK-Docker](https://github.com/jp7677/dxvk-docker) provides a way for a build setup using docker/podman.
Pre-built binaries of release versions are available at [https://github.com/jp7677/dxvk-nvapi/releases](https://github.com/jp7677/dxvk-nvapi/releases).

## How to use

Using DXVK-NVAPI in Proton, Lutris or Wine requires DXVK to see the GPU as an NVIDIA GPU.

- Disable the `nvapiHack` in DXVK with `dxgi.nvapiHack = False` set in a DXVK configuration file, see [dxvk.conf](https://github.com/doitsujin/dxvk/blob/master/dxvk.conf#L51).
- Spoof an NVIDIA GPU when running a non-NVIDIA GPU with `dxgi.customVendorId = 10de` set in a DXVK configuration file, see [dxvk.conf](https://github.com/doitsujin/dxvk/blob/master/dxvk.conf#L31).

### Proton (6.3-6 and higher)

- Copy `nvapi64.dll`/`nvapi.dll` into the `dist/lib/wine/nvapi`/`dist/lib64/wine/nvapi` folder of your Proton installation, eg in `.steam/steam/steamapps/common/Proton 6.3/`.
- Use `PROTON_ENABLE_NVAPI=1` as launch argument.

### Lutris

- TODO

### Wine

- Copy `nvapi64.dll`/`nvapi.dll` into the `system32`/`syswow64` folder of your x64/x86 Wine prefix.
- Ensure that Wine uses the native version of `nvapi64`/`nvapi`, e.g. with `WINEDLLOVERRIDES=nvapi64,nvapi=n`.
- Ensure that Wine uses DXVK's `dxgi.dll`, use it e.g. with `WINEDLLOVERRIDES=dxgi=n`.

## Debugging

The following environment variables tweak DXVK-NVAPI's runtime behavior:

- `DXVK_NVAPI_DRIVER_VERSION` lets you override the reported driver version. Valid values are numbers between 100 and 99999. Use e.g. `DXVK_NVAPI_DRIVER_VERSION=47141` to report driver version `471.41`.
- `DXVK_NVAPI_LOG_LEVEL` set to `info` prints log statements. The default behavior omits any logging. Please fill an issue if using log servery `info` creates log spam. There are no other log levels.
- `DXVK_NVAPI_LOG_PATH` enables file logging additionally to console output and sets the path where the log file `dxvk-nvapi.log` should be written to. Log statements are appended to an existing file. Please remove this file once in a while to prevent excessive grow. This requires `DXVK_NVAPI_LOG_LEVEL` set to `info`.

This project provides a test suite. Run the package script with `--enable-tests` (see above) to build `nvapi64-tests.exe`. Running the tests executable without arguments queries the local system and provides system information about visible GPU's:

```bash
DXVK_LOG_LEVEL=none WINEDEBUG=-all WINEDLLOVERRIDES=dxgi,nvapi64=n wine nvapi64-tests.exe
```

The test executable also runs on Windows against NVIDIA's `nvapi64.dll`. Ensure that DXVK-NVAPI's `nvapi64.dll`is not present in the current `PATH` for this scenario.

The actual unit tests can be run with `nvapi64-tests.exe [@unit-tests]` to validate DXVK-NVAPI's internal implementation.

## References and inspirations

- [DXVK](https://github.com/doitsujin/dxvk)
- [DXVK-AGS](https://github.com/doitsujin/dxvk-ags)
- [NVAPI](https://docs.nvidia.com/gameworks/content/gameworkslibrary/coresdk/nvapi/group__dx.html)
- [NVAPI Open Source SDK](https://download.nvidia.com/XFree86/nvapi-open-source-sdk/)
- [https://github.com/SveSop/nvapi_standalone](https://github.com/SveSop/nvapi_standalone)

Many thanks to the corresponding authors!
