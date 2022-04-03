# Hermes JS Engine
[![MIT license](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/facebook/hermes/blob/HEAD/LICENSE)
[![npm version](https://img.shields.io/npm/v/hermes-engine.svg?style=flat)](https://www.npmjs.com/package/hermes-engine)
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg)](https://github.com/facebook/hermes/blob/HEAD/CONTRIBUTING.md)
<img src="./website/static/img/logo.svg" alt="Hermes logo - large H with wings" align="right" width="20%"/>

Hermes is a JavaScript engine optimized for fast start-up of [React Native](https://reactnative.dev/) apps. It features ahead-of-time static optimization and compact bytecode.

If you're only interested in using pre-built Hermes in a new or existing React Native app, you do not need to follow this guide or have direct access to the Hermes source. Instead, just follow [these instructions to enable Hermes](https://reactnative.dev/docs/hermes).

> Noted that each Hermes release is aimed at a specific RN version. The rule of thumb is to always follow [Hermes releases](https://github.com/facebook/hermes/releases) strictly. Version mismatch can result in instant crash of your apps in the worst case scenario.

If you want to know how to build and hack on Hermes directly, and/or integrate Hermes built from source into a React Native app then read on.

The instructions here very briefly cover steps to build the Hermes CLI. They assume you have typical native development tools setup for your OS, and support for cmake and Ninja. For more details of required dependencies, building Hermes with different options, etc. follow these links instead:

* [Building and Running Hermes](doc/BuildingAndRunning.md)
* [Using a custom Hermes build in a React Native app](doc/ReactNativeIntegration.md#using-a-custom-hermes-build-in-a-react-native-app)

To build a local debug version of the Hermes CLI tools the following steps should get you started on macOS/Linux:

```shell
mkdir hermes_workingdir
cd hermes_workingdir
git clone https://github.com/facebook/hermes.git
cmake -S hermes -B build -G Ninja
cmake --build ./build
```

Or if you're using Windows, the following should get you going in a Git Bash shell:

```shell
mkdir hermes_workingdir
cd hermes_workingdir
git -c core.autocrlf=false clone https://github.com/facebook/hermes.git
cmake -S hermes -B build -G 'Visual Studio 16 2019' -A x64
cmake --build ./build
```

You will now be in a directory with the output of building Hermes into CLI tools. From here you can run a piece of JavaScript as follows:

```shell
echo "'use strict'; function hello() { print('Hello World'); } hello();" | ./bin/hermes
```

## Contributing

The main purpose of this repository is to continue to evolve Hermes, making it faster and more efficient. We are grateful to the community for contributing bugfixes and improvements. Read below to learn how you can take part in improving Hermes.

### Code of Conduct

Facebook has adopted a [Code of Conduct](./CODE_OF_CONDUCT.md) that we expect project participants to adhere to. Please read [the full text](https://code.fb.com/codeofconduct) so that you can understand what actions will and will not be tolerated.

### Contributing Guide

Read our [contributing guide](CONTRIBUTING.md) to learn about our development process, how to propose bugfixes and improvements, and how to build and test your changes to Hermes.

### License

Hermes is [MIT licensed](./LICENSE).
