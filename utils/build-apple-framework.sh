#!/bin/bash
# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

if [ "$DEBUG" = true ]; then
  BUILD_TYPE="--build-type=Debug"
else
  BUILD_TYPE="--distribute"
fi

function command_exists {
  command -v "${1}" > /dev/null 2>&1
}

if command_exists "cmake"; then
  if command_exists "ninja"; then
    BUILD_SYSTEM="Ninja"
  else
    BUILD_SYSTEM="Unix Makefiles"
  fi
else
  echo >&2 'CMake is required to install Hermes, install it with: brew install cmake'
  exit 1
fi

# Utility function to configure an Apple framework
function configure_apple_framework {
  local build_cli_tools enable_bitcode
  if [[ $1 == macosx ]]; then
    build_cli_tools="true"
    enable_bitcode="false"
  else
    build_cli_tools="false"
    enable_bitcode="true"
  fi

  local cmake_flags=" \
    -DHERMES_APPLE_TARGET_PLATFORM:STRING=$1 \
    -DCMAKE_OSX_ARCHITECTURES:STRING=$2 \
    -DCMAKE_OSX_DEPLOYMENT_TARGET:STRING=$3 \
    -DHERMES_ENABLE_DEBUGGER:BOOLEAN=true \
    -DHERMES_ENABLE_FUZZING:BOOLEAN=false \
    -DHERMES_ENABLE_TEST_SUITE:BOOLEAN=false \
    -DHERMES_ENABLE_BITCODE:BOOLEAN=$enable_bitcode \
    -DHERMES_BUILD_APPLE_FRAMEWORK:BOOLEAN=true \
    -DHERMES_BUILD_APPLE_DSYM:BOOLEAN=true \
    -DHERMES_ENABLE_TOOLS:BOOLEAN=$build_cli_tools \
    -DCMAKE_INSTALL_PREFIX:PATH=../destroot"

  ./utils/build/configure.py "$BUILD_TYPE" --cmake-flags "$cmake_flags" --build-system="$BUILD_SYSTEM" "build_$1"
}

# Utility function to build an Apple framework
function build_apple_framework {
  echo "Building framework for $1 with architectures: $2"
  
  configure_apple_framework "$1" "$2" "$3"

  if [[ "$BUILD_SYSTEM" == "Ninja" ]]; then
    (cd "./build_$1" && ninja install/strip)
  else 
    (cd "./build_$1" && make install/strip)
  fi
}

# Accepts an array of frameworks and will place all of
# the architectures into the first one in the list
function create_universal_framework {
  cd ./destroot/Library/Frameworks || exit 1

  local platforms=("$@")

  echo "Creating universal framework for platforms: ${platforms[*]}"

  for i in "${!platforms[@]}"; do
    platforms[$i]="${platforms[$i]}/hermes.framework/hermes"
  done

  lipo -create -output "${platforms[0]}" "${platforms[@]}"

  # Once all was linked into a single framework, clean destroot
  # from unused frameworks
  for platform in "${@:2}"; do
    rm -r "$platform"
  done

  lipo -info "${platforms[0]}"

  cd - || exit 1
}



