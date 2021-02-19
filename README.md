# [DXVK-NVAPI](https://github.com/jp7677/dxvk-nvapi)

## Experimental non-complete NVAPI implementation on top of [DXVK](https://github.com/doitsujin/dxvk)

This [repository](https://github.com/jp7677/dxvk-nvapi) provides a basic alternative implementation of the NVAPI extensions for DXVK. It is mostly a direct copy of DXVK-AGS, but adjusted for NVAPI.

This implementation currently forwards the following NVAPI D3D11 features to DXVK:

- `SetDepthBoundsTest`
- `BeginUAVOverlap`/`EndUAVOverlap`
- `MultiDrawInstancedIndirect`/`MultiDrawIndexedInstancedIndirect`

It also implements some methods for adapter/display topology and system information.

This implementation has been tested with Unreal Engine 4, mostly the game `Assetto Corsa Competizione` and several UE4 technology demos. Unreal Engine 4 utilizes `SetDepthBoundsTest`, it may yield like 1% extra performance which seems to be the norm when `Depth bounds test` is used.

The 32bits version of this implementation has been briefly tested with the `Monster Hunter Official Benchmark` where it also yields a similar small gain in performance.

Basic topology and system information (vendor ID, driver version etc) has been tested with `GPU Caps Viewer` and `GPU-Shark`. The game `Get Even`, which seem to verify the driver version during launch, starts fine with this implementation.

## Requirements

This implementation uses several DXVK extension points, thus using DXVK is a requirements. Using Wine's D3D11 will fail. Initialization of NVAPI-DXVK currently fails when no NVIDIA card is exposed through DXVK. Please leave a note in the issue tracker if it makes sense to change this requirement since no specific NVIDIA hardware features are needed.

## How to build

Like DXVK, this library is being built as a Windows DLL using MinGW, and has essentially the same build requirements. Development of DXVK-NVAPI is done with MinGW 7, older versions might fail.

Run:

```bash
./package-release.sh master /your/path
```

Alternatively [DXVK-Docker](https://github.com/jp7677/dxvk-docker) provides a way for a build setup using docker/podman.
Pre-built binaries are available at [https://github.com/jp7677/dxvk-nvapi/releases](https://github.com/jp7677/dxvk-nvapi/releases).

## How to use

- Copy `nvapi64.dll`/`nvapi.dll` into the `system32`/`syswow64` folder of your x64/x86 Wine prefix.
- Make sure that your prefix uses the native version of nvapi64, e.g. with `WINEDLLOVERRIDES=nvapi,nvapi64=n`.
- Disable the `nvapiHack` in DXVK, see [dxvk.conf](https://github.com/doitsujin/dxvk/blob/master/dxvk.conf#L34), this needs DXVK's `dxgi.dll`, use it e.g. with `WINEDLLOVERRIDES=dxgi=n`.
- Eventually the Wine prefix needs to be *touched* by a wine-staging version. Running `winecfg` is sufficient. The exact circumstances why this is needed are (still) unknown, that said `Assetto Corsa Competizione` wants that with a prefix created with Proton 5.13.

## Debugging

DXVK-NVAPI prints some logging statements to the console. Optionally those statements can be written to a log file using the following environment variable:

- `DXVK_NVAPI_LOG_PATH` Enables file logging and sets the path where the log file `dxvk-nvapi.log` should be written to. Loging is appended to an existing file. Please remove this file once in a while to prevent excessive grow.

## References and inspirations

- [DXVK](https://github.com/doitsujin/dxvk)
- [DXVK-AGS](https://github.com/doitsujin/dxvk-ags)
- [NVAPI](https://docs.nvidia.com/gameworks/content/gameworkslibrary/coresdk/nvapi/group__dx.html)
- [NVAPI Open Source SDK](http://download.nvidia.com/XFree86/nvapi-open-source-sdk/)
- [https://github.com/SveSop/nvapi_standalone](https://github.com/SveSop/nvapi_standalone)

Many thanks to the corresponding authors!
