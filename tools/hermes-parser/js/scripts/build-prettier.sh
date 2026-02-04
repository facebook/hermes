#!/bin/bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set -xe -o pipefail

HG_ROOT=$(hg root)
XPLAT="$HG_ROOT/xplat"
NODE="$XPLAT/third-party/node/bin/node"
XPLAT_YARN="$XPLAT/third-party/yarn/yarn"

REPO_URI="https://github.com/pieterv/prettier.git"
HERMES_PARSER_JS="$XPLAT/static_h/tools/hermes-parser/js"
HERMES_PARSER_DIST="$HERMES_PARSER_JS/hermes-parser/dist"
PRETTIER_DIR="$HERMES_PARSER_JS/prettier-hermes-flow-fork"
PLUGIN_DIR="$HERMES_PARSER_JS/prettier-plugin-hermes-parser"
PRETTIER_YARN="$PRETTIER_DIR/.yarn/releases/yarn-4.9.2.cjs"
GENERATED="generated"

if [ ! -d "$HERMES_PARSER_DIST" ]; then
    echo "$HERMES_PARSER_DIST does not exist, running initial build"
    pushd "$HERMES_PARSER_JS"
    $XPLAT_YARN build
    popd
fi

if [ ! -d "$PRETTIER_DIR" ]; then
    echo "Cloning prettier fork locally"
    git clone -b flow-fork "$REPO_URI" "$PRETTIER_DIR"
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
# Check if local branch is behind upstream/flow-fork
COMMITS_BEHIND=$(git rev-list --left-right --count upstream/flow-fork...HEAD | awk '{print $1}')
if [ "$COMMITS_BEHIND" -gt 0 ]; then
  echo "Your local prettier branch is $COMMITS_BEHIND commits behind upstream/flow-fork branch. Please rebase and try again."
  popd
  exit 1
fi

echo "
httpProxy: http://fwdproxy:8080
httpsProxy: http://fwdproxy:8080" >> .yarnrc.yml

# Install Deps
echo "Running yarn install"
$NODE $PRETTIER_YARN install

# Build prettier
echo "Building assets"
$NODE $PRETTIER_YARN build --package=@prettier/plugin-hermes

# Copy assets to prettier plugin dir
echo "Copy assets"
echo "// @$GENERATED" > "$PLUGIN_DIR/index.mjs"
cat "./dist/plugin-hermes/index.mjs" >> "$PLUGIN_DIR/index.mjs"

echo "Success!"
echo "Be sure to commit your changes and create a PR to the upstream repo after committing"

popd
