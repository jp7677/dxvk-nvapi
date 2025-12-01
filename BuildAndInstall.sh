#!/bin/bash

if [ -z "$1" ]; then
  echo "Add wineprefix as an argument"
  exit
fi

WINEPREFIX=$1

DIR="$(cd "$(dirname "$0")" && pwd)"
cd $DIR

rm -rf ./builds/
./package-release.sh master ./builds/

cd ./builds/dxvk-nvapi-master/

cp ./x32/* $WINEPREFIX/drive_c/windows/syswow64/
cp ./x64/* $WINEPREFIX/drive_c/windows/system32/

echo 'Installing Nvidia Wine Driver Libraries'

cp /lib64/nvidia/wine/* $WINEPREFIX/drive_c/windows/system32

echo "Install layer to '~/.local/share/vulkan/implicit_layer.d/' [Y/n]"
read -r YESNO
if [ $YESNO = 'n' ] || [ $YESNO = 'N' ]; then
  echo Not installing layer
  exit
fi

echo "Installing layer to '~/.local/share/vulkan/implicit_layer.d/'"

cp ./layer/* ~/.local/share/vulkan/implicit_layer.d/
