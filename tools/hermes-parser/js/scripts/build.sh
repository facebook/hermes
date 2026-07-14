#!/bin/bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set -xe -o pipefail

THIS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

PACKAGES=(
  hermes-estree
  hermes-parser
  hermes-eslint
  hermes-transform
  flow-api-translator
  babel-plugin-syntax-hermes-parser
)

# Subset of PACKAGES whose dist/ must exist before babel.config.js can enable
# the `babel-plugin-syntax-hermes-parser` parser override. We strip Flow from
# their copied dist files before Babel runs, which breaks the parser bootstrap
# dependency cycle without requiring stock @babel/parser to understand every
# current Flow syntax form.
BOOTSTRAP_PACKAGES=(
  hermes-estree
  hermes-parser
  babel-plugin-syntax-hermes-parser
)

# The parser override in babel.config.js requires the workspace plugin to be
# built. Disable it for the gen scripts and the bootstrap pass below; we
# unset just before the final pass when the chain is ready.
export SKIP_HERMES_PARSER_OVERRIDE=1

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
yarn babel-node "$THIS_DIR/genESTreeVisitorKeys.js"
yarn babel-node "$THIS_DIR/genPredicateFunctions.js"
yarn babel-node "$THIS_DIR/genTransformNodeTypes.js"

# Create fresh dist directory for each package, and copy source files in
for package in "${PACKAGES[@]}"; do
  PACKAGE_DIR="$THIS_DIR/../$package"
  DIST_DIR="$PACKAGE_DIR/dist"
  SRC_DIR="$PACKAGE_DIR/src"

  # Clean dist
  rm -rf "$DIST_DIR"
  cp -r "$SRC_DIR" "$DIST_DIR"

  # There is no system for flow to emit flow declarations for files
  # So we rename all the JS files to .js.flow so they are treated like flow declarations
  find "$DIST_DIR" -type f -name "*.js" | while read -r file; do
    # Check if file contains flow annotation
    if grep -q " @flow" "$file"; then
      # Create a new file with .js.flow extension
      new_file="${file}.flow"
      # Only proceed if the destination doesn't already exist
      if [ ! -f "$new_file" ]; then
        cp "$file" "$new_file"
      fi
    fi
  done

  # Copy just the JS files again
  rsync -a --include="*/" --include="*.js" --exclude="*" "$SRC_DIR" "$DIST_DIR"
done

# Generate source code that only applies to dist directory
yarn babel-node "$THIS_DIR/genWasmParser.js" "$WASM_PARSER"
# TODO: Move these to `src` directory, currently causes Flow errors.
yarn babel-node "$THIS_DIR/genSelectorTypes.js"
yarn babel-node "$THIS_DIR/genTransformCloneTypes.js"
yarn babel-node "$THIS_DIR/genTransformModifyTypes.js"
yarn babel-node "$THIS_DIR/genTransformReplaceNodeTypes.js"

# Bootstrap pass: strip Flow from the parser-plugin chain first, with the
# parser override still disabled (SKIP_HERMES_PARSER_OVERRIDE is set above).
# After this loop their dist/index.js files are valid plain JS, so the
# override can be re-enabled for the remaining packages.
for package in "${BOOTSTRAP_PACKAGES[@]}"; do
  PACKAGE_DIST_DIR="$THIS_DIR/../$package/dist"
  find "$PACKAGE_DIST_DIR" -type f -name "*.js" | while read -r file; do
    yarn flow-remove-types --quiet --pretty --remove-empty-imports \
      --out-file "$file.stripped" \
      "$file"
    mv "$file.stripped" "$file"
  done
  yarn babel --config-file="$THIS_DIR/../babel.config.js" "$PACKAGE_DIST_DIR" --out-dir="$PACKAGE_DIST_DIR"
done

# Re-enable the override so the remaining packages can be parsed with
# hermes-parser (they contain Flow `as` cast syntax).
unset SKIP_HERMES_PARSER_OVERRIDE

for package in "${PACKAGES[@]}"; do
  # Skip packages already processed in the bootstrap pass above.
  case " ${BOOTSTRAP_PACKAGES[*]} " in
    *" $package "*) continue ;;
  esac
  PACKAGE_DIST_DIR="$THIS_DIR/../$package/dist"
  yarn babel --config-file="$THIS_DIR/../babel.config.js" "$PACKAGE_DIST_DIR" --out-dir="$PACKAGE_DIST_DIR"
done

# Validate that the generated flow files are sane
# We don't bother validating the raw-js files as they are validated by babel first
yarn eslint "*/dist/**/*.js.flow" --no-ignore
