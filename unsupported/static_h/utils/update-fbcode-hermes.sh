#!/bin/sh
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# README:
# To update the fbcode copy of Hermes, follow these steps:
# 1) cd to fbsource/xplat/hermes/utils
# 2) Run the current script: ./update-fbcode-hermes.sh
#    This should leave you with files copied over, and new files as hg adds.
# 3) Until we update the xplat hermes to the newer LLVM used in fbcode,
#    if any of the files we had to modify for the LLVM version (only
#    4, I believe) changed, we have to re-apply those changes.
#    Those changes are in D10353003.  So attempt to build, in fbsource/fbcode:
#      buick build xplat/hermes/...
#    For any errors, check whether the file is one of those modified
#    in the given diff, and reapply the changes.
#    You may also get errors if BUCK files changed; if so, you need to
#    make equivalent changes in the corresponding TARGETS file.
#
# Not completely automatic, but not too bad.

case $(pwd) in
    *fbsource/xplat/hermes/utils) ;;
    *) echo Must run from fbsource/xplat/hermes/utils; exit 1;;
esac

# Move to fbsource
cd ../../.. || exit

# copy hermes to fbcode
echo Copying hermes.
/bin/cp -r xplat/hermes fbcode/xplat
# Now remove some directories with large files we don't need in fbcode.
echo Removing unnecessary hermes subdirs.
/bin/rm -rf fbcode/xplat/hermes/facebook/js_bundles
/bin/rm -rf fbcode/xplat/hermes/facebook/sandcastle

# copy jsi to fbcode
echo Copying jsi.
/bin/cp -r xplat/jsi fbcode/xplat

# Now do hg adds for any new files:

echo Doing hg adds.
hg status > /tmp/hg-status.out
grep "^\\? " /tmp/hg-status.out > /tmp/hg-status2.out
sed -e "s/^\\?/hg add/" /tmp/hg-status2.out > /tmp/hg-status3.out
# This executes the shell commands in /tmp/hg-status3.out.  Like "source".
. /tmp/hg-status3.out

/bin/rm /tmp/hg-status.out /tmp/hg-status2.out /tmp/hg-status3.out
