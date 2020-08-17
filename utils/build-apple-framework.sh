#!/bin/bash
# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

if [[ "$CONFIGURATION" = *Debug* ]]; then
    BUILD_TYPE="--build-type=Debug"
else
    BUILD_TYPE="--distribute"
fi

function command_exists {
  command -v ${1} > /dev/null 2>&1
}

# Utility function to build an Apple framework
function build_apple_framework {
  echo "Building framework for $1 with architectures: $2"
  
  local cmake_flags=" \
    -DHERMES_APPLE_TARGET_PLATFORM:STRING=$1 \
    -DCMAKE_OSX_ARCHITECTURES:STRING=$2 \
    -DHERMES_ENABLE_DEBUGGER:BOOLEAN=true \
    -DHERMES_ENABLE_FUZZING:BOOLEAN=false \
    -DHERMES_ENABLE_TEST_SUITE:BOOLEAN=false \
    -DHERMES_BUILD_APPLE_FRAMEWORK:BOOLEAN=true \
    -DHERMES_BUILD_APPLE_DSYM:BOOLEAN=true
    -DCMAKE_INSTALL_PREFIX:PATH=../destroot"

  
  if command_exists "cmake"; then
    if command_exists "ninja"; then
      local build_system="Ninja"
    else
      local build_system="Unix Makefiles"
    fi
  else
    echo >&2 'CMake is required to install Hermes, install it with: brew install cmake'
    exit 1
  fi

  ./utils/build/configure.py "$BUILD_TYPE" --cmake-flags "$cmake_flags" --build-system="$build_system" build

  if [[ "$build_system" == "Ninja" ]]; then
    (cd build && ninja install/strip)
  else 
    (cd build && make install/strip)
  fi

  echo "Succesfully built framework for $1"
}

function create_universal_framework {
  cd ./destroot/Library/Frameworks

  local platforms=("$@")

  echo "Creating universal framework for platforms: ${platforms[@]}"

  # Create a placeholder for universal framework based on one of the existing frameworks
  cp -r "./${platforms[0]}/hermes.framework" ./hermes.framework

  for i in "${!platforms[@]}"; do
    platforms[$i]="${platforms[$i]}/hermes.framework/hermes"
  done

  lipo -create -output "hermes.framework/hermes" "${platforms[@]}"

  lipo -info "hermes.framework/hermes"
}

# Build frameworks for every platform and architecture.
# In the future, this list can be extended ith other Apple platforms,
# such as `ipados`, depending on our prefernces
build_apple_framework "iphoneos" "armv7;armv7s;arm64"
build_apple_framework "iphonesimulator" "x86_64"

# Create universal framework for requested platforms
create_universal_framework "iphoneos" "iphonesimulator"



