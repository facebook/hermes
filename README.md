# Hermes JS Engine for React Native Windows
[![MIT license](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/facebook/hermes/blob/master/LICENSE)
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg)](https://github.com/facebook/hermes/blob/master/CONTRIBUTING.md)
<img src="./website/static/img/logo.svg" alt="Hermes logo - large H with wings" align="right" width="20%"/>

Hermes is a JavaScript engine optimized for fast start up of [React Native](https://reactnative.dev/) apps. It features ahead-of-time static optimization and compact bytecode. This repo is Microsoft’s fork of [facebook/hermes](https://github.com/facebook/hermes) and brings Hermes support to [React Native Windows](https://github.com/microsoft/react-native-windows).

The following commands should get you going in a Windows Command Prompt:

```shell
mkdir hermes-windows-workingdir
cd hermes-windows-workingdir
git -c core.autocrlf=false clone https://github.com/microsoft/hermes-windows.git
python hermes-windows/utils/build/configure.py --build-system="Visual Studio 16 2019" --cmake-flags="-A x64" --distribute
cd build_release
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
