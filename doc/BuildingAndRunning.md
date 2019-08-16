---
id: building-and-running
title: Building and Running Hermes
---

This document describes how to build and run Hermes as a standalone compiler and VM. To use Hermes in the context of a React Native app, see the [React Native](https://facebook.github.io/react-native/docs/getting-started) documentation.

## Dependencies

Hermes is a C++11 project. clang, gcc, and Visual C++ are supported. Hermes also requires cmake, git, ICU, Python, and zip. It builds with [CMake](https://cmake.org) and [ninja](https://ninja-build.org).

The Hermes REPL will also use libreadline, if available.

To install dependencies on Ubuntu:

    apt install cmake git ninja-build libicu-dev python zip libreadline-dev

On Mac via Homebrew:

    brew install cmake git ninja

## Building on Linux and macOS

Hermes will place its build files in the current directory by default. Note that Hermes will download and build LLVM as part of its own build.
You can also give explicit source and build directories, use `--help` on the build scripts to see how.

Create a base directory to work in, e.g. ~/workspace, and cd into it. Follow the steps below to build LLVM and generate the Hermes build system:

    git clone https://github.com/facebook/hermes.git
    hermes/utils/build/build_llvm.py
    hermes/utils/build/configure.py

The build system has now been generated in the `build` directory. To perform the build:

    cd build && ninja

## Building on Windows

The Windows build depends on which particular combination of GitBash/Cygwin/WSL and Visual Studio is used.

    git -c core.autocrlf=false clone https://github.com/facebook/hermes.git
    hermes/utils/build/build_llvm.py --build-system='Visual Studio 16 2019' --cmake-flags='-A x64' --distribute
    hermes/utils/build/configure.py --build-system='Visual Studio 16 2019' --cmake-flags='-A x64 -DLLVM_ENABLE_LTO=OFF' --distribute
    cd build_release && MSBuild.exe ALL_BUILD.vcxproj /p:Configuration=Release

## Running Hermes

The primary binary is the `hermes` tool, which will be found at `build/bin/hermes`. This tool compiles JavaScript to Hermes bytecode. It can also execute JavaScript, from source or bytecode.

### Executing JavaScript with Hermes

    hermes test.js

## Compiling and Executing JavaScript with Bytecode

    hermes -emit-binary -out test.hbc test.js
    hermes test.hbc


## Running Tests

To run the Hermes test suite:

    ninja check-hermes


## Release Build

The above instructions create an unoptimized debug build. The `--distribute` flag will enable a release build, in the `build_release` directory. Example:

    hermes/utils/build/build_llvm.py --distribute
    hermes/utils/build/configure.py --distribute
    cd build_release && ninja

### Other Tools

In addition to `hermes`, the following tools will be built:

- `hdb`: JavaScript command line debugger
- `hermes-repl`: JavaScript interactive REPL
- `hbcdump`: Hermes bytecode disassembler
- `hermesc`: Standalone Hermes compiler. This can compile JavaScript to Hermes bytecode, but does not support executing it.
- `hvm`: Standalone Hermes VM. This can execute Hermes bytecode, but does not support compiling it.
