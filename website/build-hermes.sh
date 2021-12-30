#!/bin/bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# This script creates a build subdirectory called "build_wasm" and builds Hermes
# inside it. The result (hermes.js, hermes.wasm) will be under "build_wasm/bin".
# It requires three parameters:
# - the path to Emscripten, so that it can find
#   "emscripten/cmake/Modules/Platform/Emscripten.cmake" relative to it
# - the path to the Hermes source tree
# - the path to a host hermesc, so that it can find "ImportHermesc.cmake"
#   relative to it. See <https://hermesengine.dev/docs/emscripten>
#
# An example of usage:
#   ./build-hermes.sh ~/github/emsdk/upstream/ ~/github/hermes ~/hermes-host-hermesc

SCRIPT="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
BUILD_DIR="${SCRIPT}/build_wasm"
REL_DIR="${SCRIPT}/static"

set -e

[ -z "$1" ] && echo "Emscripten path required" && exit 1
EMS_PATH="$1"
shift

[ -z "$1" ] && echo "Hermes path required" && exit 1
HERMES_PATH="$1"
shift

[ -z "$1" ] && echo "Host Hermesc path required" && exit 1
HOST_HERMESC_PATH="$1"
shift

FLAGS=""
FLAGS="$FLAGS -s WASM=1"
FLAGS="$FLAGS -s ALLOW_MEMORY_GROWTH=0"
FLAGS="$FLAGS -s TOTAL_MEMORY=33554432"
FLAGS="$FLAGS -s MODULARIZE=1 -s EXPORT_NAME=createHermes"
FLAGS="$FLAGS -s INVOKE_RUN=0"
FLAGS="$FLAGS -s EXIT_RUNTIME=1"
FLAGS="$FLAGS -s NODERAWFS=0"
FLAGS="$FLAGS -s EXTRA_EXPORTED_RUNTIME_METHODS=[callMain,FS]"

cleanup() {
  rm -rf "$BUILD_DIR"
}

configure() {
  mkdir "$BUILD_DIR" && cd "$BUILD_DIR"
  cmake "$HERMES_PATH" \
    -DCMAKE_TOOLCHAIN_FILE="$EMS_PATH/emscripten/cmake/Modules/Platform/Emscripten.cmake" \
    -DCMAKE_BUILD_TYPE=MinSizeRel \
    -DIMPORT_HERMESC:PATH="$HOST_HERMESC_PATH/ImportHermesc.cmake" \
    -DCMAKE_EXE_LINKER_FLAGS="$FLAGS"
}

build () {
  make -j hermes
}

# copy the build artifacts to release dir
copy () {
  cp "$BUILD_DIR/bin/hermes.js" "$REL_DIR/hermes.js"
  cp "$BUILD_DIR/bin/hermes.wasm" "$REL_DIR/hermes.wasm"
}

cleanup && configure && build && copy
