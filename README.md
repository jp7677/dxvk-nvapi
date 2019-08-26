# Experimental non-complete NVAPI D3D11 implementation for [DXVK](https://github.com/doitsujin/dxvk)

This repository provide a very (very very) basic proof-of-concept implementation of the NVAPI extensions for DXVK. It is mostly a direct copy DXVK-AGS, but adjusted for NVAPI.

This implementation currently advertises `Depth bounds test` only.

This implemenation has been tested with a single game only: Assetto Corsa Competizione, based on Unreal Engine 4. It even seems that `Depth bounds test` is actually used by the game. Due to the complexity of the API this implementation surely wont work with anything else. Unreal Engine 4 is so kind to query every method it uses before actually using a method. Any engine or program that uses the NVAPI methods directly will fail.

## How to build

Like DXVK, this library is being built as a Windows DLL using MinGW, and has essentially the same build requirements.

Run:

```bash
./package-release.sh master /your/path
```

## How to use

- Copy `nvapi64.dll` into the `system32` folder of your x64 Wine prefix.
- Make sure that your prefix uses the native version of nvapi64.
- Disable the `nvapiHack` in DXVK.

## References and inspirations

- [DXVK](https://github.com/doitsujin/dxvk)
- [DXVK-AGS](https://github.com/doitsujin/dxvk-ags)
- [NVAPI](https://docs.nvidia.com/gameworks/content/gameworkslibrary/coresdk/nvapi/group__dx.html)
- [https://github.com/pchome/dxvk-nvapi-module](https://github.com/pchome/dxvk-nvapi-module)
- [https://github.com/SveSop/nvapi_standalone](https://github.com/SveSop/nvapi_standalone)
- [https://github.com/falahati/NvAPIWrapper/blob/master/NvAPIWrapper/Native/Helpers/FunctionId.cs](https://github.com/falahati/NvAPIWrapper/blob/master/NvAPIWrapper/Native/Helpers/FunctionId.cs)

Many thanks to the coresponding authors!
