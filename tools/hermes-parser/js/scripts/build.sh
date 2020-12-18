#!/bin/bash
# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

THIS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

yarn install

# Create fresh build directory and copy source JS files in
BUILD_DIR="$THIS_DIR/../build"
rm -rf "$BUILD_DIR"
mkdir "$BUILD_DIR"
cp "$THIS_DIR"/../src/* "$BUILD_DIR"

# Use internal FB build or pass path to WASM parser as first command line argument
FB_BUILD_WASM_PARSER="$THIS_DIR/facebook/buildWasmParser.sh"
if [[ -f "$FB_BUILD_WASM_PARSER" ]]; then
  WASM_PARSER=$("$FB_BUILD_WASM_PARSER")
else
  WASM_PARSER="$1"
fi

# Use internal FB build or pass path to include path as second command line argument
FB_GET_INCLUDE_PATH="$THIS_DIR/facebook/getIncludePath.sh"
if [[ -f "$FB_GET_INCLUDE_PATH" ]]; then
  INCLUDE_PATH=$("$FB_GET_INCLUDE_PATH")
else
  INCLUDE_PATH="$2"
fi

node "$THIS_DIR/genWasmParser.js" "$WASM_PARSER"
node "$THIS_DIR/genVisitorKeys.js" "$INCLUDE_PATH"
node "$THIS_DIR/genNodeDeserializers.js" "$INCLUDE_PATH"
