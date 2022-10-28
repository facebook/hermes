#!/bin/bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set -xe -o pipefail

THIS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

PACKAGES=(hermes-estree hermes-parser hermes-eslint hermes-transform hermes-translate)

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

# Generate the JSON blob used to drive the rest of the JS codegen
yarn babel-node "$THIS_DIR/genESTreeJSON.js" "$INCLUDE_PATH"

# Generate source code, written into package src directories
yarn babel-node "$THIS_DIR/genNodeDeserializers.js" "$INCLUDE_PATH"
yarn babel-node "$THIS_DIR/genParserVisitorKeys.js"
yarn babel-node "$THIS_DIR/genESLintVisitorKeys.js"
yarn babel-node "$THIS_DIR/genPredicateFunctions.js"
yarn babel-node "$THIS_DIR/genTransformNodeTypes.js"

# Create fresh dist directory for each package, and copy source files in
for package in "${PACKAGES[@]}"; do
  PACKAGE_DIR="$THIS_DIR/../$package"
  rm -rf "$PACKAGE_DIR/dist"

  cp -r "$PACKAGE_DIR/src" "$PACKAGE_DIR/dist"

  # There is no system for flow to emit flow declarations for files
  # So we rename all the JS files to .js.flow so they are treated like flow declarations
  find "$PACKAGE_DIR/dist" -type f -name "*.js" -exec rename --no-overwrite ".js" ".js.flow" {} \;

  # Copy just the JS files again
  (cd "$PACKAGE_DIR/src" && find . -type f -name '*.js' -exec cp --parents -t ../dist {} +)
done

# Generate source code that only applies to dist directory
yarn babel-node "$THIS_DIR/genWasmParser.js" "$WASM_PARSER"
# TODO: Move these to `src` directory, currently causes Flow errors.
yarn babel-node "$THIS_DIR/genSelectorTypes.js"
yarn babel-node "$THIS_DIR/genTransformCloneTypes.js"
yarn babel-node "$THIS_DIR/genTransformModifyTypes.js"
yarn babel-node "$THIS_DIR/genTransformReplaceNodeTypes.js"

for package in "${PACKAGES[@]}"; do
  PACKAGE_DIST_DIR="$THIS_DIR/../$package/dist"
  yarn babel --config-file="$THIS_DIR/../babel.config.js" "$PACKAGE_DIST_DIR" --out-dir="$PACKAGE_DIST_DIR"
done

# Validate that the generated flow files are sane
# We don't bother validating the raw-js files as they are validated by babel first
yarn eslint "*/dist/**/*.js.flow" --no-ignore
