# Using a custom Hermes build in a React Native app

To make Hermes usable in a React Native app we need to perform the following steps:

1. Package a Hermes build into an npm package structure.
2. Register this npm package with Yarn for use in other local projects.
3. Link the npm package registered with Yarn into the React Native tree for our app.

## Making a Hermes npm package

A Hermes npm package combines:
* Hermes CLI tools in binary form. These are used to compile JavaScript to bytecode when packaging up an app release.
* Pre-compiled release and debug versions of the native Hermes library for Android targets.

### Dependencies
As we will be compiling native libraries for Android, we first need to setup a suitable development environment. This subject is covered in more detail elsewhere, for example in the guide on [building React Native From Source](https://github.com/facebook/react-native/wiki/Building-from-source). However, a rough guide to the external dependencies for macOS/Linux is as follows:

* Node.js
* Gradle (version 4.10.1+)
* JDK (1.8+)
* Yarn
* The Android Native Development Kit (NDK)
* The Android Software Development Kit (SDK)

Note the Android SDK is typically downloaded and installed automatically as part of the flow in Android Studio. If you are following this common route, be sure to note the location Android Studio uses to install the SDK.

### Build environment

Ensure the following variables are present and set correctly in your environment:

* `ANDROID_NDK` - the root of your Android NDK install. E.g. "/opt/android_ndk/r15c".
* `ANDROID_SDK`, `ANDROID_HOME`, and `ANDROID_SDK_ROOT` - the root of your Android SDK install. E.g. "/opt/android_sdk".
* `JAVA_HOME` - the root of your Java Runtime Environment (JRE) install. Note a JRE instance is typically available in a sub-directory of a JDK install. E.g. "/opt/jdk-1.8/jre".
* `HERMES_WS_DIR` - the same workspace used to make a build Hermes CLI tools.

Make sure the `node`, `yarn`, and `gradle` binaries are available in your system PATH.

### Package build

To make a Hermes npm package, first follow the instructions on [building Hermes](BuildingAndRunning.md) to create a **release** version of Hermes. This will be the source of CLI binaries for use in the npm.

Next, we need to cross-compile LLVM for various different Android targets. This step only needs to be run once, and may take quite some time:

```shell
(cd $HERMES_WS_DIR && ./hermes/utils/crosscompile_llvm.sh)
```

Once cross-compiled LLVM is built we can build Hermes libraries for various Android platforms, and package everything together:

```shell
(cd $HERMES_WS_DIR/build_release && ninja github-cli-release)
(cd $HERMES_WS_DIR/hermes/android && gradle githubRelease)
cp $HERMES_WS_DIR/build_android/distributions/hermes-runtime-android-v*.tar.gz $HERMES_WS_DIR/hermes/npm
cp $HERMES_WS_DIR/build_release/github/hermes-cli-*-v*.tar.gz $HERMES_WS_DIR/hermes/npm
(cd $HERMES_WS_DIR/hermes/npm && yarn install && yarn run prepack-dev)
(cd $HERMES_WS_DIR/hermes/npm && yarn link)
```

The final `yarn link` command in this sequence registers your custom build of Hermes to Yarn for inclusion in local projects. It only needs to be run once, although running it again is benign.

## Linking Hermes into a React Native app

To use your custom Hermes npm package in an app, first make sure the app works with a normal release of Hermes by following [instructions in the React Native docs](https://reactnative.dev/docs/hermes).

Next, link the Hermes npm package into the React Native package in your app. For example, assuming your project is in the directory `$AWESOME_PROJECT` you would run this command:

```shell
(cd $AWESOME_PROJECT/node_modules/react-native && yarn link hermes-engine)
```

You can now develop your app in the normal way.

Note: if you are making changes to the compiler in Hermes, be sure to make sure you test your app in release mode as this enables bytecode compilation in advance.
