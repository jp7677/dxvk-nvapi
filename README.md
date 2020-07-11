# [DXVK-NVAPI](https://github.com/jp7677/dxvk-nvapi)

## Experimental non-complete NVAPI D3D11 implementation for [DXVK](https://github.com/doitsujin/dxvk)

This [repository](https://github.com/jp7677/dxvk-nvapi) provides a very basic alternative implementation of the NVAPI extensions for DXVK. It is mostly a direct copy DXVK-AGS, but adjusted for NVAPI.

This implementation currently advertises `Depth bounds test` only. It has been tested with a single game only: Assetto Corsa Competizione, based on Unreal Engine 4, which uses `Depth bounds test`. The game does not provide an in-game benchmark, so unfortunately there are no numbers how much it improves, but it may yield something like 1% extra performance which seems to be the norm when `Depth bounds test` is used. Due to the complexity of the API, this implementation wont work with games that rely on other aspects of this rather huge API, though other games based on Unreal Engine 4 might work with this.

The 32bits version of this implementation has been just briefly tested with the Monster Hunter Official Benchmark where it also yields a similar small gain in performce.

## How to build

Like DXVK, this library is being built as a Windows DLL using MinGW, and has essentially the same build requirements.

Run:

```bash
./package-release.sh master /your/path
```

Alternatively [DXVK-Docker](https://github.com/jp7677/dxvk-docker) provides a way for a build setup using docker.
Pre-built binaries are available at [https://github.com/jp7677/dxvk-nvapi/releases](https://github.com/jp7677/dxvk-nvapi/releases).

## How to use

- Copy `nvapi64.dll`/`nvapi.dll` into the `system32` folder of your x64/x86 Wine prefix.
- Make sure that your prefix uses the native version of nvapi64, e.g. with `export WINEDLLOVERRIDES=nvapi,nvapi64=n`.
- Disable the `nvapiHack` in DXVK, see [dxvk.conf](https://github.com/doitsujin/dxvk/blob/master/dxvk.conf#L34).

## References and inspirations

- [DXVK](https://github.com/doitsujin/dxvk)
- [DXVK-AGS](https://github.com/doitsujin/dxvk-ags)
- [NVAPI](https://docs.nvidia.com/gameworks/content/gameworkslibrary/coresdk/nvapi/group__dx.html)
- [NVAPI Open Source SDK](http://download.nvidia.com/XFree86/nvapi-open-source-sdk/)
- [https://github.com/pchome/dxvk-nvapi-module](https://github.com/pchome/dxvk-nvapi-module)
- [https://github.com/SveSop/nvapi_standalone](https://github.com/SveSop/nvapi_standalone)
- [https://stackoverflow.com/questions/13291783/how-to-get-the-id-memory-address-of-dll-function](https://stackoverflow.com/questions/13291783/how-to-get-the-id-memory-address-of-dll-function)
- [https://github.com/falahati/NvAPIWrapper/blob/master/NvAPIWrapper/Native/Helpers/FunctionId.cs](https://github.com/falahati/NvAPIWrapper/blob/master/NvAPIWrapper/Native/Helpers/FunctionId.cs)

Many thanks to the coresponding authors!
