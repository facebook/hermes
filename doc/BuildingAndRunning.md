---
id: building-and-running
title: Building and Running
---

This document describes how to build and run Hermes as a standalone compiler and VM. To use Hermes in the context of a React Native app, see the [React Native](https://reactnative.dev/docs/getting-started) documentation.

## Dependencies

Hermes is a C++14 project. clang, gcc, and Visual C++ are supported. Hermes also requires cmake, git, ICU, Python, and zip. It builds with [CMake](https://cmake.org) and [ninja](https://ninja-build.org).

The Hermes REPL will also use libreadline, if available.

To install dependencies on Ubuntu:

    apt install cmake git ninja-build libicu-dev python zip libreadline-dev

On Arch Linux:

    pacman -S cmake git ninja icu python zip readline

On Mac via Homebrew:

    brew install cmake git ninja

## Building on Linux and macOS

Hermes will place its build files in the current directory by default.
You can also give explicit source and build directories, use `--help` on the build scripts to see how.

Create a base directory to work in, e.g. `~/workspace`, and cd into it.
(Tip: avoid naming it `hermes`, as `hermes` will be one of several subdirectories in the workspace).
After `cd`ing, follow the steps below to generate the Hermes build system:

    git clone https://github.com/facebook/hermes.git
    cmake -S hermes -B build -G Ninja

The build system has now been generated in the `build` directory. To perform the build:

    cmake --build ./build


## Release Build

The above instructions create an unoptimized debug build. The `-DCMAKE_BUILD_TYPE=Release` flag will create a release build:

    cmake -S hermes -B build_release -G Ninja -DCMAKE_BUILD_TYPE=Release
    cmake --build ./build_release

## Building on Windows

To build on Windows using Visual Studio with a checkout in the `hermes` directory:

    cmake -S hermes -B build -G 'Visual Studio 16 2019'
    cmake --build ./build

## Running Hermes

The primary binary is the `hermes` tool, which will be found at `build/bin/hermes`. This tool compiles JavaScript to Hermes bytecode. It can also execute JavaScript, from source or bytecode or be used as a REPL.

### Executing JavaScript with Hermes

    hermes test.js

## Compiling and Executing JavaScript with Bytecode

    hermes -emit-binary -out test.hbc test.js
    hermes test.hbc

## Running Tests

To run the Hermes test suite:

    ninja check-hermes

To run Hermes against the test262 suite, you need to have a Hermes binary built
already and a clone of the [test262 repo](https://github.com/tc39/test262/):

    hermes/utils/testsuite/run_testsuite.py -b <hermes_build> <test262>

E.g. if we configured at `~/hermes_build` (i.e. `~/hermes_build/bin/hermes` is
an executable) and cloned test262 at `~/test262`, then perform:

    hermes/utils/testsuite/run_testsuite.py -b ~/hermes_build ~/test262/test

Note that you can also only test against part of a test suite, e.g. to run the
Intl402 subset of the test262, you can specifiy a subdir:

    hermes/utils/testsuite/run_testsuite.py -b ~/hermes_build ~/test262/test/intl402

## Formatting Code

To automatically format all your changes, you will need `clang-format`, then
simply run:

    hermes/utils/format.sh

## AddressSanitizer (ASan) Build

 The `-HERMES_ENABLE_ADDRESS_SANITIZER=ON` flag will create a ASan build:

    git clone https://github.com/facebook/hermes.git
    cmake -S hermes -B asan_build -G Ninja -D HERMES_ENABLE_ADDRESS_SANITIZER=ON
    cmake --build ./asan_build

You can verify the build by looking for `asan` symbols in the `hermes` binary:

    nm asan_build/bin/hermes | grep asan

### Other Tools

In addition to `hermes`, the following tools will be built:

- `hdb`: JavaScript command line debugger
- `hbcdump`: Hermes bytecode disassembler
- `hermesc`: Standalone Hermes compiler. This can compile JavaScript to Hermes bytecode, but does not support executing it.
- `hvm`: Standalone Hermes VM. This can execute Hermes bytecode, but does not support compiling it.
