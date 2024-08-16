---
id: cross-compilation
title: Cross Compilation
---

This document describes how to build Hermes in a cross compilation setting, e.g.
building for Android, WASM (with Emscripten), or any other platforms different
than the host development machines.

## Android compilation without ICU

Edit paths:
* `CMAKE_TOOLCHAIN_FILE` should point to whatever `android_ndk` you need.
* `IMPORT_HOST_COMPILERS` should point to whatever SH host build you want to use for InternalBytecode compilation.
* `-B` path is to your build directory, or omit it to build in current directory.

Ensure that `CMAKE_CXX_FLAGS_RELEASE` is `-O3 -DNDEBUG` if you're in Release mode.

Then you can use `adb push` to copy any files into `/data/local/tmp/` on an Android device and run it there using 'adb shell'.

```
cmake ~/fbsource/xplat/static_h \
  -B ~/static_h/build_android
  -DHERMES_FACEBOOK_BUILD=OFF \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE=/opt/android_ndk/r17fb2/build/cmake/android.toolchain.cmake \
  -DHERMES_UNICODE_LITE=ON -DICU_FOUND=1 \
  -DIMPORT_HOST_COMPILERS=~/static_h/build_release/ImportHostCompilers.cmake \
  -DHERMES_ENABLE_DEBUGGER=OFF \
  -DANDROID_ABI=arm64-v8a -G Ninja
```

## (Optional) Cross-compiling ICU

1. Make sure that:
```
ANDROID_NDK_REPOSITORY=/opt/android_ndk
ANDROID_NDK=/opt/android_ndk/android-ndk-whatever
```
and `$PATH` includes `/opt/android_ndk/android-ndk-whatever`.

2. Clone, patch and build:

```
git clone https://github.com/SwiftAndroid/libiconv-libicu-android.git
cd libiconv-libicu-android
git checkout 4bc78f4646bd54f71fa5fd13322b8ab1e8fe9d2a
git apply $path_to_hermes/android/cross-compile/swift-libicu.patch
./build.sh
```

The libraries will be built in `./armeabi-v7a/`.

## Emscripten

For Emscripten, you can find an example from the `test-emscripten` job from `hermes/.circleci/config.yml`. Also see more details at [Building with Emscripten](./Emscripten.md)
