---
id: cross-compilation
title: Cross Compilation
---

This document describes how to build Hermes in a cross compilation setting, e.g.
building for Android, WASM (with Emscripten), or any other platforms different
than the host development machines.

## A Two-stage Build

Hermes now requires a two stage build proecess because the VM now contains
Hermes bytecode which needs to be compiled by Hermes

### Setting up the workspace

We will use environment variable `$HERMES_WS_DIR` to indicate the root of your
workspace where the `hermes` git checkout directory is a subdirectory.


### 1st Stage: Building the Host Hermes Compiler

```
cd "$HERMES_WS_DIR"

# Generate the build system at $HERMES_WS_DIR/build_host_hermesc
cmake -S hermes -B ./build_host_hermesc

# Build the Hermes compiler
cmake --build ./build_host_hermesc --target hermesc
```

### 2nd Stage: Building the target Hermes

The key is that we need to pass a CMake flag `-DIMPORT_HERMESC:PATH=$HERMES_WS_DIR/build_host_hermesc/ImportHermesc.cmake` during the cross compilation build of
Hermes so it can access the host `hermesc` from the first stage to build the VM.

This process is currently happened in different places for different platforms:

1. For Android, this happened in `hermes/android/build.gradle`
2. For Apple platforms, this happend in `hermes/utils/build-apple-framework.sh`
3. For Emscripten, you can find an example from the `test-emscripten` job from `hermes/.circleci/config.yml`. Also see more details at [Building with Emscripten](./Emscripten.md)
