#!/bin/bash

set -e

shopt -s extglob

if [ -z "$1" ] || [ -z "$2" ]; then
  echo "Usage: $0 version destdir"
  exit 1
fi

VERSION="$1"
SRC_DIR=`dirname $(readlink -f $0)`
BUILD_DIR=$(realpath "$2")"/dxvk-nvapi-$VERSION"

if [ -e "$BUILD_DIR" ]; then
  echo "Build directory $BUILD_DIR already exists"
  exit 1
fi

shift 2

opt_nopackage=0
opt_devbuild=0

crossfile="build-win"

while [ $# -gt 0 ]; do
  case "$1" in
  "--no-package")
    opt_nopackage=1
    ;;
  "--dev-build")
    opt_devbuild=1
    ;;
  *)
    echo "Unrecognized option: $1" >&2
    exit 1
  esac
  shift
done

function validate_methods {
  python validate-methods.py src/nvapi.cpp src/nvapi_sys.cpp src/nvapi_disp.cpp src/nvapi_mosaic.cpp src/nvapi_gpu.cpp src/nvapi_d3d.cpp src/nvapi_d3d11.cpp src/nvapi_interface.cpp
}

function build_arch {
  cd "$SRC_DIR"

  meson --cross-file "$SRC_DIR/$crossfile$1.txt" \
        --buildtype "release"                    \
        --prefix "$BUILD_DIR"                    \
        --strip                                  \
        --bindir "x$1"                           \
        --libdir "x$1"                           \
        "$BUILD_DIR/build.$1"

  cd "$BUILD_DIR/build.$1"
  ninja install

  if [ $opt_devbuild -eq 0 ]; then
    # get rid of some useless .a files
    rm "$BUILD_DIR/x$1/"*.!(dll)
    rm -R "$BUILD_DIR/build.$1"
  fi
}

function copy_extra {
  cd "$SRC_DIR"
  cp LICENSE $BUILD_DIR
  cp README.md $BUILD_DIR
}

validate_methods
build_arch 32
build_arch 64
copy_extra
