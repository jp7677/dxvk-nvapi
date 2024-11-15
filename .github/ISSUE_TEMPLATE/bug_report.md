---
name: Bug report
about: Report issues, crashes etc.
title: ''
labels: ''
assignees: ''

---

Before opening an issue, see if the information available at [DXVK-NVAPI Wiki](https://github.com/jp7677/dxvk-nvapi/wiki) provides a solution.

Please describe your issue as accurately as possible including expected and actual results.   

### Software information
- Name of the application or game:
- DXVK-NVAPI, DXVK or VKD3D-Proton settings (environment variables and/or configuration files):

### System information
- GPU:
- Driver:
- Launcher (e.g Proton or Bottles):
- Proton/Wine version:
- DXVK-NVAPI version:
- DXVK version:
- VKD3D-Proton version:

### Log files

Please attach DXVK-NVAPI log files as a text file :
- When using Proton, set the Steam launch options for your game to `DXVK_NVAPI_LOG_PATH=/tmp/ DXVK_NVAPI_LOG_LEVEL=info %command%` and attach the corresponding `dxvk-nvapi.log` file in your `/tmp/` directory.
- See [Tweaks, debugging and troubleshooting](https://github.com/jp7677/dxvk-nvapi/blob/master/README.md#tweaks-debugging-and-troubleshooting) for general information.

Alternatively attach Proton logs as a text file:
- When using Proton, set the Steam launch options for your game to `PROTON_LOG=1 %command%` and attach the corresponding `steam-xxxxx.log` file in your home directory.

Do not forget to remove those settings once log files have been attached. Please refrain from posting long log sections inline, use [collapsed sections](https://docs.github.com/en/get-started/writing-on-github/working-with-advanced-formatting/organizing-information-with-collapsed-sections) instead.
