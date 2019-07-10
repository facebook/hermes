# Hermes JS VM
[![MIT license](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/facebook/hermes/blob/master/LICENSE)
[![npm version](https://img.shields.io/npm/v/hermesvm.svg?style=flat)](https://www.npmjs.com/package/hermesvm)
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg)](https://github.com/facebook/hermes/blob/master/CONTRIBUTING.md)

Hermes is a JavaScript virtual machine optimized for fast start up of
[React Native](https://facebook.github.io/react-native/) apps on Android.
It features ahead-of-time static optimization and compact bytecode.

## Using Hermes in React Native

Head over to [React Native](https://facebook.github.io/react-native/)
if you are looking to build a React Native app powered by Hermes,
or migrate an existing React Native app to Hermes. You can find some
guides below.

* [Create a new React Native app powered by Hermes]
  * Do we want to be explicit here that the app will continue to use JSC on iOS?
* [Migrate a React Native app to use Hermes]
* [Debug JS code in a React Native app powered by Hermes]

## Hacking on Hermes

This section describes how to build Hermes from source code.
This is not needed if you are trying to build a React Native
app using Hermes. You don't need to read this unless you are trying to
make changes to Hermes: adding features, fixing bugs, etc.

### Building compiler and REPL

The following steps allow you to build and run Hermes on OS X and Linux.
You will be able to run Hermes compiler and execute generated Hermes bytecode
on the host platform (your computer). Later, we will describe how to build
Hermes library that runs on Android.

Hermes compiler and REPL builds and runs on Windows as well. See [Windows build
instructions] for details.
Note that, with Windows, you will not be able to build Hermes library that runs on Android.

System requirements:

* The project depends on the tools that are required to build LLVM (a C++ compiler, CMake, Python, etc)
* The REPL uses 'libreadline' for editing, if it is installed.
* `ICU_ROOT`

Create a base directory to work in, e.g. `~/workspace`, and `cd` into it. Then
follow the steps below:

1. `export HERMES_WS_DIR="$PWD"`.
2. Set `ICU_ROOT` environment variable to the location of ICU:
   `export ICU_ROOT=......`.
3. Clone Hermes: `git clone git@github.com:facebook/hermes.git`.
4. Run `./hermes/utils/build_llvm.py` to download and build LLVM dependency.
   This may take a while.
5. Configure the build for the Hermes compiler and REPL for the host platform:
   `./hermes/utils/configure.sh`. Set up environment variable `DISTRIBUTE=1`
   for release build.
6. Build the compiler and REPL: `( cd build_release && ninja github-cli-release )`

### Running and testing compiler and REPL

After compiling the project, the vm driver binary will be located in the `/bin`
directory under the name `./bin/hermes`.  Run `./bin/hermes --help` to learn
more about using the vm test driver.

To run the tests run the `check-hermes` target. If you are using the default
build system, ninja, then the command to run the tests is `ninja check-hermes`.

The default compilation mode is `Debug`. This means that the compiler itself is
easy to debug because it has debug info, lots of assertions, and the
optimizations are disabled. If you wish to benchmark the compiler or release it
then you should compile the compiler in Release mode.

When configuring the project add the flag `-DCMAKE_BUILD_TYPE=Release`. Refer to
the LLVM build instructions for more details:

    http://llvm.org/docs/GettingStarted.html

To enable an ASan build, configure the project with the flag
`-DLLVM_USE_SANITIZER=Address`.

### Building Hermes for React Native

Make sure you have followed instructions in `Building Hermes on host`.

First make sure that the Android SDK and NDK are installed, and that the
environment variables `ANDROID_SDK` and `ANDROID_NDK` are set. Here's an
example:

```
$ echo "$ANDROID_SDK"
/opt/android_sdk

$ echo "$ANDROID_NDK"
/opt/android_ndk/r15c
```

You also need node.js and babel.

1. Cross-compile LLVM dependencies for all Android ABIs: `hermes/utils/crosscompile_llvm.sh`
2. Compile libhermes for Android: ( cd hermes/android && gradle githubRelease )
3. Build the hermesvm npm

```
cp build_android/distributions/hermes-runtime-android-v*.tar.gz hermes/npm
cp build_release/github/hermes-cli-*-v*.tar.gz hermes/npm
(cd hermes/npm && yarn && yarn run prepack-dev && yarn link)

# for release build (this will not work with the private repo, unless
# you add a personal access token <https://github.com/settings/tokens>
# to the URLs in fetch.js, like ?access_token=...)
(TODO: it seems the content below is for internal use only?)
# Create a github release
# Update the release_version number in hermes/CMakeLists.txt
# Add files to the github release.  This will require building on more than one machine:
#   build_android/distributions/hermes-runtime-android-v<version>.tar.gz
#   build_release/github/hermes-cli-darwin-v<version>.tar.gz
#   build_release/github/hermes-cli-linux-v<version>.tar.gz
# Update the release version and file digests in hermes/npm/package.json
# (cd hermes/npm && yarn pack)
# TODO: have travis automate this

# 9. Clone react-native-hermes here
git clone git@github.com:facebookexperimental/react-native-hermes.git
# 10. Replace the RN template app version number (0.60.0-rc.1) with the path
#    to your react-native-hermes directory.
printf '%s\n' "%s|0.60.0-rc.1|file://${HERMES_WS_DIR:?}/react-native-hermes" wq |
    ed react-native-hermes/template/package.json
# 11. Fetch react-native-hermes' dependencies
( cd react-native-hermes && yarn install )
# 12. Create a React Native demo project from the react-native-hermes template
npx @react-native-community/cli@2.0.0-alpha.16 init AwesomeProject --template "file://${HERMES_WS_DIR:?}/react-native-hermes"
( cd AwesomeProject/node_modules/react-native && yarn link hermesvm )
# 13. Build and run the demo project
( cd AwesomeProject && react-native start ) &
( cd AwesomeProject && npx @react-native-community/cli@2.0.0-alpha.16 run-android )
# 14. Verify that you are using Hermes by checking that `typeof(HermesInternal)`
# is `"object"` (and not `"undefined"`) in JavaScript.
# 15. If you want to build RNTester:
( cd react-native-github && yarn link hermesvm )
( cd react-native-github && ./gradlew RNTester:android:app:installDebug )
```

## Contributing

The main purpose of this repository is to continue to evolve Hermes, making it faster and more efficient. We are grateful to the community for contributing bugfixes and improvements. Read below to learn how you can take part in improving React.

### Code of Conduct

Facebook has adopted a [Code of Conduct](./CODE_OF_CONDUCT) that we expect project participants to adhere to. Please read [the full text](https://code.fb.com/codeofconduct) so that you can understand what actions will and will not be tolerated.

### Contributing Guide

Read our [contributing guide] to learn about our development process, how to propose bugfixes and improvements, and how to build and test your changes to Hermes.

### License

React is [MIT licensed](./LICENSE).
