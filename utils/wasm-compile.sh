#!/bin/bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set -e  # Exit immediately if a command exits with a non-zero status
set -u  # Treat unset variables as an error and exit immediately

# Check if exactly one argument is provided
if [[ $# -ne 3 ]]; then
    echo "Usage: $0 <path-to-host-build> <path-to-wasm-build> <file>"
    exit 1
fi

# Check if HermesSourcePath is set
if [[ -z "${HermesSourcePath+x}" ]]; then
    echo "Error: HermesSourcePath is not set."
    exit 1
fi

# Check if the host build directory is valid
if [[ ! -x $1/bin/shermes ]]; then
    echo "Error: '$1' does not contain a bin/shermes executable."
    exit 1
fi
shermes="$1/bin/shermes"

# Check if the wasm build directory is valid
if [[ ! -f $2/bin/hermes.js ]]; then
    echo "Error: '$2' does not contain bin/hermes.js"
    exit 1
fi
wasm_build="$2"

# Check if the file exists
if [[ ! -f $3 ]]; then
    echo "Error: File '$3' does not exist."
    exit 1
fi
# Extract the filename without path and extension
input="$3"
file_name=$(basename "$input")   # Remove path
file_name="${file_name%.*}"      # Remove extension

echo "Using shermes to compile $input... to ${file_name}.c"
"$shermes" -Xenable-tdz -emit-c "$input"

echo "Using emcc to compile ${file_name}.c to ${file_name}.o"
emcc "${file_name}.c" -c \
    -O3 \
    -DNDEBUG \
    -fno-strict-aliasing -fno-strict-overflow \
    -I${wasm_build}/lib/config \
    -I${HermesSourcePath}/include

echo "Using emcc to link ${file_name}.o to ${file_name}-wasm.js/.wasm"
emcc -O3 ${file_name}.o -o ${file_name}-wasm.js \
    -L${wasm_build}/lib \
    -L${wasm_build}/jsi \
    -L${wasm_build}/tools/shermes \
    -lshermes_console_a -lhermesvm_a -ljsi \
    -sALLOW_MEMORY_GROWTH=1 -sSTACK_SIZE=256KB

ls -lh ${file_name}-wasm.*
