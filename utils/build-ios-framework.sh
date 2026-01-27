#!/bin/bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

if [ "$CI" ]; then
  set -x
fi
set -e

# Given a specific target, retrieve the right architecture for it
# $1 the target you want to build. Allowed values: iphoneos, iphonesimulator, catalyst, xros, xrsimulator
function get_architecture {
    if [[ $1 == "iphoneos" || $1 == "xros" ]]; then
      echo "arm64"
    elif [[ $1 == "iphonesimulator" || $1 == "xrsimulator" ]]; then
      echo "x86_64;arm64"
    elif [[ $1 == "appletvos" ]]; then
      echo "arm64"
    elif [[ $1 == "appletvsimulator" ]]; then
      echo "x86_64;arm64"
    elif [[ $1 == "catalyst" ]]; then
      echo "x86_64;arm64"
    else
      echo "Error: unknown architecture passed $1"
      exit 1
    fi
}

function get_deployment_target {
    if [[ $1 == "xros" || $1 == "xrsimulator" ]]; then
      echo "$(get_visionos_deployment_target)"
    else # tvOS and iOS use the same deployment target
      echo "$(get_ios_deployment_target)"
    fi
}

function build_catalyst {
  # $1 is the deployment_target here

  # get the architectures
  architectures=$(get_architecture "catalyst")

  # loop over the architectures and build them
  echo "$architectures" | tr ';' '\n' | while read -r arch; do
    build_apple_framework "catalyst" "$arch" "$1"

    echo "Finding the hermesvm.framework"
    find "." -name "hermesvm.framework" -print
    echo "=============================="
    ls -lr .
    echo "=============================="

    mkdir -p "./build_catalyst/lib/$arch/hermesvm.framework"
    mv "./build_catalyst/lib/hermesvm.framework" "./build_catalyst/lib/$arch/"
    mv "./build_catalyst/lib/hermesvm.framework.dSYM" "./build_catalyst/lib/$arch/"
  done

  echo "Create the framework for both Catalyst architectures"
  cp -R "./build_catalyst/lib/arm64/hermesvm.framework" "./build_catalyst/lib/hermesvm.framework"
  lipo -create \
    "./build_catalyst/lib/x86_64/hermesvm.framework/hermesvm" \
    "./build_catalyst/lib/arm64/hermesvm.framework/hermesvm" \
    -output "./build_catalyst/lib/hermesvm.framework/hermesvm"

  echo "Create the dSYMs for both Catalyst architectures"
  cp -R "./build_catalyst/lib/arm64/hermesvm.framework.dSYM" "./build_catalyst/lib/hermesvm.framework.dSYM"
  lipo -create \
    "./build_catalyst/lib/x86_64/hermesvm.framework.dSYM/Contents/Resources/DWARF/hermesvm" \
    "./build_catalyst/lib/arm64/hermesvm.framework.dSYM/Contents/Resources/DWARF/hermesvm" \
    -output "./build_catalyst/lib/hermesvm.framework.dSYM/Contents/Resources/DWARF/hermesvm"

  echo "Remove the individual architectures folders"
  echo "$architectures" | tr ';' '\n' | while read -r arch; do
    rm -rf "./build_catalyst/lib/$arch"
  done
}

function build_framework {
  if [ ! -d destroot/Library/Frameworks/universal/hermesvm.xcframework ]; then
    deployment_target=$(get_deployment_target "$1")

    # If $1 (platform) is catalyst call build catalyst
    # if [[ $1 == "catalyst" ]]; then
    #   build_catalyst "$deployment_target"
    #   return
    # fi

    deployment_target=$(get_deployment_target "$1")

    architecture=$(get_architecture "$1")

    build_apple_framework "$1" "$architecture" "$deployment_target"
  else
    echo "Skipping; Clean \"destroot\" to rebuild".
  fi
}

# group the frameworks together to create a universal framework
function build_universal_framework {
    if [ ! -d destroot/Library/Frameworks/universal/hermesvm.xcframework ]; then
        create_universal_framework "iphoneos" "iphonesimulator" "catalyst" "xros" "xrsimulator" "appletvos" "appletvsimulator"
    else
        echo "Skipping; Clean \"destroot\" to rebuild".
    fi
}

# single function that builds sequentially iphoneos, iphonesimulator and catalyst
# this is used to preserve backward compatibility
function create_framework {
    if [ ! -d destroot/Library/Frameworks/universal/hermesvm.xcframework ]; then
        build_framework "iphoneos"
        build_framework "iphonesimulator"
        build_framework "appletvos"
        build_framework "appletvsimulator"
        build_framework "catalyst"
        build_framework "xros"
        build_framework "xrsimulator"
        build_universal_framework
    else
        echo "Skipping; Clean \"destroot\" to rebuild".
    fi
}


CURR_SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"
# shellcheck source=xplat/hermes/utils/build-apple-framework.sh
source "${CURR_SCRIPT_DIR}/build-apple-framework.sh"

if [[ -z $1 ]]; then
  create_framework
elif [[ $1 == "build_framework" ]]; then
  build_universal_framework
else
  build_framework "$1"
fi
