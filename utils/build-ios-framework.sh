#!/bin/bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

. ./utils/build-apple-framework.sh

ios_deployment_target=$(get_ios_deployment_target)
visionos_deployment_target=$(get_visionos_deployment_target)
tvos_deployment_target=$(get_tvos_deployment_target)

function build_all {
  if [ ! -d destroot/Library/Frameworks/universal/hermesvm.xcframework ]; then
    build_apple_framework "iphoneos" "arm64" "$ios_deployment_target"
    build_apple_framework "iphonesimulator" "x86_64;arm64" "$ios_deployment_target"
    build_apple_framework "catalyst" "x86_64;arm64" "$ios_deployment_target"
    build_apple_framework "xros" "arm64" "$visionos_deployment_target"
    build_apple_framework "xrsimulator" "arm64" "$visionos_deployment_target"
    build_apple_framework "appletvos" "arm64" "$tvos_deployment_target"
    build_apple_framework "appletvsimulator" "x86_64;arm64" "$tvos_deployment_target"

    create_universal_framework "iphoneos" "iphonesimulator" "catalyst" "xros" "xrsimulator" "appletvos" "appletvsimulator"
  else
    echo "Skipping; Clean \"destroot\" to rebuild".
  fi
}

if [[ $1 == "iphoneos" ]]; then
  build_apple_framework "iphoneos" "arm64" "$ios_deployment_target"
elif [[ $1 == "iphonesimulator" ]]; then
  build_apple_framework "iphonesimulator" "x86_64;arm64" "$ios_deployment_target"
elif [[ $1 == "appletvos" ]]; then
  build_apple_framework "appletvos" "arm64" "$tvos_deployment_target"
elif [[ $1 == "appletvsimulator" ]]; then
  build_apple_framework "appletvsimulator" "x86_64;arm64" "$tvos_deployment_target"
else
  echo "Build for all supported platforms and create universal framework."
  build_all
fi
