#!/bin/bash
# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# This script creates a build subdirectory called "build_asmjs" and builds Hermes inside
# it. The result (hermes.js, hermes.js.mem) will be under "build_asmjs/bin".
# It requires two parameters:
#  - the path to Emscripten, so that it can find
#   "emscripten/cmake/Modules/Platform/Emscripten.cmake" relative to it
#  - the path to the Hermes source tree
#
# In the short term, there will be an attempt made to maintain a resonably up-to-date copy
# of the compiled result here:
#   https://tmikov.github.io/hermes-explorer-page/hermes.js
#   https://tmikov.github.io/hermes-explorer-page/hermes.js.mem
# So generating the website doesn't always require Emscripten and building Hermes.

set -e

[ -z "$1" ] && echo "Emscripten path required" && exit 1
EMS_PATH="$1"
shift

[ -z "$1" ] && echo "Hermes path required" && exit 1
HERMES_PATH="$1"
shift

FLAGS=""
FLAGS="$FLAGS -s WASM=0"
FLAGS="$FLAGS -s ALLOW_MEMORY_GROWTH=0"
FLAGS="$FLAGS -s TOTAL_MEMORY=33554432"
FLAGS="$FLAGS -s MODULARIZE=1 -s EXPORT_NAME=createApp"
FLAGS="$FLAGS -s INVOKE_RUN=0"
FLAGS="$FLAGS -s EXIT_RUNTIME=1"
FLAGS="$FLAGS -s NODERAWFS=0"
FLAGS="$FLAGS -s EXTRA_EXPORTED_RUNTIME_METHODS=[callMain,FS]"

mkdir build_asmjs && cd build_asmjs
cmake "$HERMES_PATH" \
    -DCMAKE_TOOLCHAIN_FILE="$EMS_PATH/emscripten/cmake/Modules/Platform/Emscripten.cmake" \
    -DCMAKE_BUILD_TYPE=MinSizeRel \
    -DEMSCRIPTEN_FASTCOMP=1 \
    -DCMAKE_EXE_LINKER_FLAGS="$FLAGS"

make -j hermes
