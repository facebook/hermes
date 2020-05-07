# Cross Compiling Hermes for Android

These are informal instructions for cross-compiling a "standalone" build of
Hermes (one that doesn't use Java).

## Cross-compile ICU

1. Make sure that:
```
ANDROID_NDK_REPOSITORY=/opt/android_ndk
ANDROID_NDK=/opt/android_ndk/android-ndk-r13b
```
and `$PATH` includes `/opt/android_ndk/android-ndk-r13b`.

2. Clone, patch and build:

```
git clone https://github.com/SwiftAndroid/libiconv-libicu-android.git
cd libiconv-libicu-android
git checkout 4bc78f4646bd54f71fa5fd13322b8ab1e8fe9d2a
git apply $path_to_this_directory/swift-libicu.patch
./build.sh
```

The libraries will be built in `./armeabi-v7a/`.

## Hermes
1. Edit paths in `config-hermes.sh'.
2. Configure and build Hermes:
```
mkdir hermes_arm && cd hermes_arm
$path_to_this_directory/config-llvm.sh
ninja hermes hvm
```
