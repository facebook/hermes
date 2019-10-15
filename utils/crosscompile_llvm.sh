#!/bin/bash
# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set -e

die() {
  echo "$*"
  exit 1
}

[ -n "$HERMES_WS_DIR" ]   || die "HERMES_WS_DIR unset"
cd "$HERMES_WS_DIR"

[ -d "$ANDROID_NDK" ] ||
    die "Android NDK does not exist (\$ANDROID_NDK=$ANDROID_NDK)"

[ -d "$ANDROID_SDK" ] ||
    die "Android SDK does not exist (\$ANDROID_SDK=$ANDROID_SDK)"

[ -d "hermes/" ]     || die "Can't find hermes/ in $PWD"
[ -d "llvm/" ]       || die "Can't find llvm/ in $PWD"
[ -d "llvm_build/" ] || die "Can't find llvm_build. Did you build_llvm.py?"

if [ ! -e 'llvm/hermes' ]
then
  (
    cd llvm
    git apply ../hermes/android/cross-compile/llvm.patch
    echo "Patched by $0" > hermes
  )
fi

for abi in "armeabi-v7a" "arm64-v8a" "x86_64" "x86"
do
    (
      mkdir -p "llvm-$abi"
      cd "llvm-$abi"
      # shellcheck disable=SC2191  # Allow literal = in array elements
      cmd=(
        cmake -G Ninja "$HERMES_WS_DIR/llvm"
          -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK/build/cmake/android.toolchain.cmake"
          -DCMAKE_CXX_FLAGS="-fvisibility=hidden"
          -DANDROID_PLATFORM="android-16"
          -DANDROID_ABI="$abi"
          -DANDROID_NDK="$ANDROID_NDK"
          -DANDROID_STL="c++_shared"
          -DANDROID_PIE="True"
          -DLLVM_TARGETS_TO_BUILD=
          -DCMAKE_BUILD_TYPE=MinSizeRel
          -DLLVM_VERSION_PRINTER_SHOW_HOST_TARGET_INFO=Off
          -DLLVM_TABLEGEN="$HERMES_WS_DIR/llvm_build/bin/llvm-tblgen"
          )

      printf '%q ' "${cmd[@]}"
      echo
      "${cmd[@]}"

      # The build does not complete successfully,
      # so just build what we need for now.
      ninja libLLVM{Analysis,Core,Support,Demangle,Object}.a
    )
done

