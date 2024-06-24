#!/bin/bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set -xe -o pipefail

HG_ROOT=$(hg root)
XPLAT="$HG_ROOT/xplat"
XPLAT_YARN="$XPLAT/third-party/yarn/yarn"

REPO_URI="https://github.com/pieterv/prettier.git"
HERMES_PARSER_JS="$XPLAT/hermes/tools/hermes-parser/js"
HERMES_PARSER_DIST="$HERMES_PARSER_JS/hermes-parser/dist"
PRETTIER_DIR="$HERMES_PARSER_JS/prettier-hermes-v2-backport"
PLUGIN_DIR="$HERMES_PARSER_JS/prettier-plugin-hermes-parser"
PRETTIER_YARN="$PRETTIER_DIR/.yarn/releases/yarn-4.1.0.cjs"

if [ ! -d "$HERMES_PARSER_DIST" ]; then
    echo "$HERMES_PARSER_DIST does not exist, running initial build"
    $XPLAT_YARN build
fi

if [ ! -d "$PRETTIER_DIR" ]; then
    echo "Cloning prettier fork locally"
    git clone -b hermes-v2-backport "$REPO_URI" "$PRETTIER_DIR"
fi

pushd "$PRETTIER_DIR"

# Set upstream remote to canonical url
if git remote get-url upstream > /dev/null 2>&1; then
  git remote set-url upstream "$REPO_URI"
else
  git remote add upstream "$REPO_URI"
fi

# Remove nested eslintrc to keep tooling happy
rm -f .eslintrc.cjs

echo "Checking prettier branch is up to date"
git fetch upstream
# Check if local branch is behind upstream/hermes-v2-backport
COMMITS_BEHIND=$(git rev-list --left-right --count upstream/hermes-v2-backport...HEAD | awk '{print $1}')
if [ "$COMMITS_BEHIND" -gt 0 ]; then
  echo "Your local prettier branch is $COMMITS_BEHIND commits behind upstream/hermes-v2-backport branch. Please rebase and try again."
  popd
  exit 1
fi

# Install Deps
echo "Running yarn install"
node $PRETTIER_YARN install

echo "Copying hermes-parser dist folder into node_modules"
cp -r "$HERMES_PARSER_DIST" "./node_modules/hermes-parser/"

# Build prettier
echo "Building assets"
node $PRETTIER_YARN build --no-minify

# Copy assets to prettier plugin dir
echo "Copy assets"
cp -r "./dist/plugins" "$PLUGIN_DIR/src/third-party/internal-prettier-v3/"
cp "./dist/ast-to-doc.js" "$PLUGIN_DIR/src/third-party/internal-prettier-v3/ast-to-doc.js"

echo "Success!"
echo "Be sure to commit your changes and create a PR to the upstream repo after committing"

popd
