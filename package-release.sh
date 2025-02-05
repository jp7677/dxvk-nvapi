#!/bin/bash

set -e

shopt -s extglob

if [ -z "$1" ] || [ -z "$2" ]; then
  echo "Usage: $0 version destdir"
  exit 1
fi

VERSION="$1"
SRC_DIR=$(dirname "$(readlink -f "$0")")
BUILD_DIR=$(realpath "$2")"/dxvk-nvapi-$VERSION"

if [ -e "$BUILD_DIR" ]; then
  echo "Build directory $BUILD_DIR already exists"
  exit 1
fi

shift 2

opt_enabletests=false
opt_disablelayer=0
opt_devbuild=0

crossfile="build-win"

while [ $# -gt 0 ]; do
  case "$1" in
  "--no-package")
    echo "Ignoring option: --no-package"
    ;;
  "--enable-tests")
    opt_enabletests=true
    ;;
  "--disable-layer")
    opt_disablelayer=1
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

function prepare {
  python3 validate-methods.py \
    src/nvapi.cpp             \
    src/nvapi_sys.cpp         \
    src/nvapi_disp.cpp        \
    src/nvapi_mosaic.cpp      \
    src/nvapi_ngx.cpp         \
    src/nvapi_drs.cpp         \
    src/nvapi_gpu.cpp         \
    src/nvapi_d3d.cpp         \
    src/nvapi_d3d11.cpp       \
    src/nvapi_d3d12.cpp       \
    src/nvapi_vulkan.cpp      \
    src/nvapi_interface.cpp   \
    external/nvapi/nvapi_interface.h
}

function build_arch {
  cd "$SRC_DIR"

  # remove generated files, because otherwise the existing
  # files get into the build instead of the generated ones
  rm -f version.h config.h

  meson setup                                \
    --cross-file "$SRC_DIR/$crossfile$1.txt" \
    --buildtype "release"                    \
    --prefix "$BUILD_DIR"                    \
    --strip                                  \
    --bindir "x$1"                           \
    --libdir "x$1"                           \
    -Denable_tests=$opt_enabletests          \
    "$BUILD_DIR/build.$1"

  cd "$BUILD_DIR/build.$1"
  ninja install

  cp version.h "$SRC_DIR"
  cp config.h "$SRC_DIR"

  if [ $opt_devbuild -eq 0 ]; then
    # get rid of some useless .a files
    rm "$BUILD_DIR/x$1/"*.!(dll|exe)
    rm -R "$BUILD_DIR/build.$1"
  fi
}

function build_layer {
  # remove generated files, because otherwise the existing
  # files get into the build instead of the generated ones
  rm -f "$SRC_DIR"/layer/{version,config}.h

  meson setup                     \
    --buildtype "release"         \
    --prefix "$BUILD_DIR/layer"   \
    --libdir ''                   \
    --strip                       \
    -Dabsolute_library_path=false \
    -Dlibrary_path_prefix=./      \
    -Dmanifest_install_dir=.      \
    "$BUILD_DIR/build.layer"      \
    "$SRC_DIR/layer"

  ninja -C "$BUILD_DIR/build.layer" install
 
  cp "$BUILD_DIR"/build.layer/{version,config}.h "$SRC_DIR/layer"

  if (( ! opt_devbuild )); then
    rm -R "$BUILD_DIR/build.layer"
  fi
}

function copy_extra {
  cd "$SRC_DIR"
  cp LICENSE "$BUILD_DIR"
  cp README.md "$BUILD_DIR"
}

prepare
build_arch 32
build_arch 64
cd "$SRC_DIR"
if (( ! opt_disablelayer )); then
  build_layer
fi
copy_extra
