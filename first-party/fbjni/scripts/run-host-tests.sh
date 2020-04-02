#!/bin/bash
# Copyright (c) Facebook, Inc. and its affiliates.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set -exo pipefail

BASE_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )/.."
CMAKE=$ANDROID_HOME/cmake/3.10.2.4988404/bin/cmake
export CXX=clang++

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
