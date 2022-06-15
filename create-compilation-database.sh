#!/bin/bash

set -e

shopt -s extglob

DB="compile_commands.json"
SRC_DIR=$(dirname "$(readlink -f "$0")")
BUILD_DIR="$SRC_DIR/db"

if [ -e "$BUILD_DIR" ]; then
  echo "Build directory $BUILD_DIR already exists"
  exit 1
fi

if [ -e "$DB" ]; then
  echo "Clang Compilation Database ($DB) already exists"
  exit 1
fi

mkdir "$BUILD_DIR"

meson --cross-file "$SRC_DIR/build-win64.txt"  \
      --buildtype "release"                    \
      --prefix "$BUILD_DIR"                    \
      --strip                                  \
      --bindir "x64"                           \
      --libdir "x64"                           \
      -Denable_tests=True                      \
      "$BUILD_DIR"

sed -i 's/\/db"/\/"/g; s/\.\.\///g' "$BUILD_DIR"/"$DB"

cp "$BUILD_DIR"/"$DB" "$SRC_DIR"
rm -R "$BUILD_DIR"

echo "Clang Compilation Database ($DB) created"
