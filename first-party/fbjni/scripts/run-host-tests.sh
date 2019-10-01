#!/bin/bash
# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the LICENSE file
# in the root directory of this source tree.

set -exo pipefail

BASE_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )/.."
CMAKE=$ANDROID_HOME/cmake/3.10.2.4988404/bin/cmake

mkdir -p "$BASE_DIR/host-build-cmake"
cd "$BASE_DIR/host-build-cmake"

# Configure CMake project
$CMAKE -DJAVA_HOME="$JAVA_HOME" ..
# Build binaries and libraries
make
# Run C++ tests
make test
# LD_LIBRARY_PATH is needed for native library dependencies to load cleanly
TEST_LD_LIBRARY_PATH="$BASE_DIR/host-build-cmake:$BASE_DIR/host-build-cmake/test/jni"
# Build and run JNI tests
cd "$BASE_DIR"
env LD_LIBRARY_PATH="$TEST_LD_LIBRARY_PATH" ./gradlew -b host.gradle -PbuildDir=host-build-gradle test
