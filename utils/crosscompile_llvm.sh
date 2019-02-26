#!/bin/bash
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
[ -d "llvm_build/" ] || die "Can't find llvm_build. Did you build_llvm.sh?"

if [ ! -e 'llvm/hermes' ]
then
  (
    cd llvm
    git apply ../hermes/doc/android/llvm.patch
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
          -DANDROID_ABI="$abi"
          -DANDROID_PLATFORM="android-22"
          -DCMAKE_TOOLCHAIN_FILE="$ANDROID_SDK/ndk-bundle/build/cmake/android.toolchain.cmake"
          -DCMAKE_SYSTEM_NAME="Android"
          -DCMAKE_ANDROID_ARCH_ABI="$abi"
          -DCMAKE_ANDROID_NDK="$ANDROID_NDK"
          -DCMAKE_ANDROID_STL_TYPE="c++_shared"
          -DCMAKE_ANDROID_NDK_TOOLCHAIN_VERSION="clang"
          -DCMAKE_POSITION_INDEPENDENT_CODE="True"
          -DLLVM_TARGETS_TO_BUILD=
          -DCMAKE_BUILD_TYPE=MinSizeRel
          -DLLVM_VERSION_PRINTER_SHOW_HOST_TARGET_INFO=Off
          -DLLVM_TABLEGEN="$HERMES_WS_DIR/llvm_build/bin/llvm-tblgen"
          -DCLANG_TABLEGEN="$HERMES_WS_DIR/llvm_build/bin/clang-tblgen"
          )

      printf '%q ' "${cmd[@]}"
      echo
      "${cmd[@]}"

      # The build does not complete successfully,
      # so just build what we need for now.
      ninja libLLVM{Analysis,Core,Support,Demangle,Object}.a
    )
done

