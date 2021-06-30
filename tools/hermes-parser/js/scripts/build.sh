#!/bin/bash
# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set -xe -o pipefail

THIS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

PACKAGES=(hermes-parser hermes-eslint)

# Yarn install all packages
yarn install

# Use internal FB build or pass path to WASM parser as first command line argument
FB_BUILD_WASM_PARSER="$THIS_DIR/facebook/buildWasmParser.sh"
if [[ -f "$1" ]]; then
  WASM_PARSER="$1"
elif [[ -f "$FB_BUILD_WASM_PARSER" ]]; then
  WASM_PARSER=$("$FB_BUILD_WASM_PARSER")
else
  echo "Failed to get WASM parser" 1>&2
  exit 1
fi

# Use internal FB build or pass path to include path as second command line argument
FB_GET_INCLUDE_PATH="$THIS_DIR/facebook/getIncludePath.sh"
if [[ -d "$2" ]]; then
  INCLUDE_PATH="$2"
elif [[ -f "$FB_GET_INCLUDE_PATH" ]]; then
  INCLUDE_PATH=$("$FB_GET_INCLUDE_PATH")
else
  echo "Failed to get include path" 1>&2
  exit 1
fi

# Create fresh dist directory for each package, and copy source files in
for package in "${PACKAGES[@]}"; do
  PACKAGE_DIR="$THIS_DIR/../$package"
  rm -rf "$PACKAGE_DIR/dist"
  cp -r "$PACKAGE_DIR/src" "$PACKAGE_DIR/dist"
done

# Generate code, written into package dist directories
node "$THIS_DIR/genWasmParser.js" "$WASM_PARSER"
node "$THIS_DIR/genParserVisitorKeys.js" "$INCLUDE_PATH"
node "$THIS_DIR/genESLintVisitorKeys.js" "$INCLUDE_PATH"
node "$THIS_DIR/genNodeDeserializers.js" "$INCLUDE_PATH"

for package in "${PACKAGES[@]}"; do
  PACKAGE_DIST_DIR="$THIS_DIR/../$package/dist"
  babel --config-file="$THIS_DIR/../.babelrc" "$PACKAGE_DIST_DIR" --out-dir="$PACKAGE_DIST_DIR"
done
