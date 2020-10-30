#!/bin/bash
# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

. ./utils/build-apple-framework.sh

if [ ! -d destroot/Library/Frameworks/iphoneos/hermes.framework ]; then
    ios_deployment_target=$(get_ios_deployment_target)

    build_apple_framework "iphoneos" "armv7;armv7s;arm64" "$ios_deployment_target"
    build_apple_framework "iphonesimulator" "x86_64;i386" "$ios_deployment_target"

    create_universal_framework "iphoneos" "iphonesimulator"
else
    echo "Skipping; Clean \"destroot\" to rebuild".
fi
