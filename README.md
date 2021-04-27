# Hermes JS Engine for React Native Windows
[![MIT license](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/facebook/hermes/blob/master/LICENSE)
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg)](https://github.com/facebook/hermes/blob/master/CONTRIBUTING.md)
<img src="./website/static/img/logo.svg" alt="Hermes logo - large H with wings" align="right" width="20%"/>

Hermes is a JavaScript engine optimized for fast start up of [React Native](https://reactnative.dev/) apps. It features ahead-of-time static optimization and compact bytecode. This repo is Microsoft’s fork of [facebook/hermes](https://github.com/facebook/hermes) and brings Hermes support to [React Native Windows](https://github.com/microsoft/react-native-windows).

If you're only interested in using pre-built Hermes in a new or existing React Native app, you do not need to follow this guide or have direct access to the Hermes source. Instead, just follow [these instructions to enable Hermes](https://reactnative.dev/docs/hermes).

> Noted that each Hermes release is aimed at a specific RN version. The rule of thumb is to always follow [Hermes releases](https://github.com/facebook/hermes/releases) strictly. Version mismatch can result in instant crash of your apps in the worst case scenario.

If you want to know how to build and hack on Hermes directly, and/or integrate Hermes built from source into a React Native app then read on.

The instructions here very briefly cover steps to build the Hermes CLI. They assume you have typical native development tools setup for your OS, and support for cmake and Ninja. For more details of required dependencies, building Hermes with different options, etc. follow these links instead:

* [Building and Running Hermes](doc/BuildingAndRunning.md)
* [Using a custom Hermes build in a React Native app](doc/ReactNativeIntegration.md#using-a-custom-hermes-build-in-a-react-native-app)

To build a local debug version of the Hermes CLI tools the following steps should get you started on macOS/Linux:
The following commands should get you going in a Windows Command Prompt:

```shell
mkdir hermes_workingdir
cd hermes_workingdir
git clone https://github.com/facebook/hermes.git
hermes/utils/build/configure.py
cd build
ninja
```

Or if you're using Windows, the following should get you going in a Git Bash shell:

```shell
mkdir hermes_workingdir
cd hermes_workingdir
git -c core.autocrlf=false clone https://github.com/facebook/hermes.git
hermes/utils/build/configure.py --build-system='Visual Studio 16 2019' --cmake-flags='-A x64' --distribute
cd build
MSBuild.exe ALL_BUILD.vcxproj /p:Configuration=Release
```

You will now be in a directory with the output of building Hermes into CLI tools. From here you can run a piece of JavaScript as follows:

```shell
echo 'use strict'; function hello() { print('Hello World'); } hello(); | .\bin\Release\hermes.exe
```

For more details on Hermes for Android, see [here](https://github.com/facebook/hermes/blob/master/README.md).

## Contributing

The main purpose of this repository is to brings Hermes support to [React Native Windows](https://github.com/microsoft/react-native-windows). We are grateful to the community for contributing bugfixes and improvements. Read below to learn how you can participate.

### Code of Conduct

Both Microsoft and Facebook have adopted [Codes of Conduct](./CODE_OF_CONDUCT.md) that we expect project participants to adhere to. Microsoft's Code of Conduct can be found [here](https://opensource.microsoft.com/codeofconduct)  and Facebook's [here](https://code.fb.com/codeofconduct). Please read through them so that you can understand what actions will and will not be tolerated.

### Contributing Guide

Read our [contributing guide](CONTRIBUTING.md) to learn about our development process as well as how to propose bugfixes and improvements.

### License

Hermes is [MIT licensed](./LICENSE).
