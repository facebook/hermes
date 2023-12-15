#!/bin/bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set -xe -o pipefail

HG_ROOT=$(hg root)
XPLAT="$HG_ROOT/xplat"
YARN="$XPLAT/third-party/yarn/yarn"

REPO_URI="https://github.com/pieterv/prettier.git"
HERMES_PARSER_JS="$XPLAT/hermes/tools/hermes-parser/js"
PLUGIN_DIR="$HERMES_PARSER_JS/prettier-plugin-hermes-parser"
YARN_OFFLINE_MIRROR=$($YARN config get yarn-offline-mirror)

if [ ! -d "prettier-hermes-v2-backport" ]; then
    echo "Cloning prettier fork locally"
    git clone -b hermes-v2-backport "$REPO_URI" prettier-hermes-v2-backport
fi

pushd prettier-hermes-v2-backport

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
$YARN install

# Remove offline-mirror changes
# There is likely a better way to do this
echo "Purging any changes to the yarn-offline-mirror"
hg purge "$YARN_OFFLINE_MIRROR"

echo "Copying hermes-parser dist folder into node_modules"
cp -r "$HERMES_PARSER_JS/hermes-parser/dist" "./node_modules/hermes-parser/"

# Build prettier
echo "Building assets"
$YARN build --no-minify

# Copy assets to prettier plugin dir
echo "Copy assets"
cp -r "./dist/plugins" "$PLUGIN_DIR/src/third-party/internal-prettier-v3/"
cp "./dist/ast-to-doc.js" "$PLUGIN_DIR/src/third-party/internal-prettier-v3/ast-to-doc.js"

echo "Success!"
echo "Be sure to commit your changes and create a PR to the upstream repo after committing"

popd
