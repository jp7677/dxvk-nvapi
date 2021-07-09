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
  python validate-methods.py \
    src/nvapi.cpp \
    src/nvapi_sys.cpp \
    src/nvapi_disp.cpp \
    src/nvapi_mosaic.cpp \
    src/nvapi_gpu.cpp \
    src/nvapi_d3d.cpp \
    src/nvapi_d3d11.cpp \
    src/nvapi_d3d12.cpp \
    src/nvapi_interface.cpp

  # remove existing version.h, because otherwise the existing one gets into the build instead of the generated one
  if [ -e version.h ]; then
    rm version.h
  fi
}

function build_arch {
  cd "$SRC_DIR"

  meson --cross-file "$SRC_DIR/$crossfile$1.txt" \
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

  if [ $opt_devbuild -eq 0 ]; then
    # get rid of some useless .a files
    rm "$BUILD_DIR/x$1/"*.!(dll|exe)
    rm -R "$BUILD_DIR/build.$1"
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
copy_extra
