# CLAUDE.md

This file provides guidance to Claude Code when working with code in this repository.

## CRITICAL: Never Change the Working Directory

**NEVER use `cd` to change the current directory from the project root.** Almost all operations can be performed by passing the correct path to commands and tools. Changing directories causes confusion and errors in subsequent operations.

In the rare cases where changing directory is absolutely unavoidable, use a subshell so the directory change does not persist:
```bash
(cd other-dir; command;)
```

## Overview

This is the **hermes-windows** fork of facebook/hermes, adapted for Windows.
Hermes is a JavaScript engine optimized for fast start-up of React Native apps.
It features ahead-of-time static optimization and compact bytecode.

The fork syncs with the upstream `static_h` branch.

## Build Commands

**CRITICAL: NEVER run cmake or ninja directly.** The build environment requires
specific Visual Studio environment variables that are set up by the build
scripts. Always use `build.js` via PowerShell or the `.\dev.ps1` wrapper.

### PowerShell Wrapper (preferred)

```powershell
.\dev build --platform x64 --configuration debug
.\dev build --platform arm64ec --configuration release --binskim
```

### Direct build.js (alternative)

Use PowerShell to invoke:

```powershell
node .ado/scripts/build.js --platform x64 --configuration debug
node .ado/scripts/build.js --platform x64 --configuration release
```

### Available Platforms

- `x64` (default) — uses Clang
- `x86` — uses Clang
- `arm64` — uses Clang
- `arm64ec` — uses MSVC (Clang not supported yet)

### Build Options

```powershell
# Build specific targets
node .ado/scripts/build.js --targets hermes_rt,hermes-icu --configuration debug

# Run lit tests (JS regression tests)
node .ado/scripts/build.js --jstest --configuration debug

# Run Test262 intl tests
node .ado/scripts/build.js --test262-intl --configuration debug

# Run BinSkim security validation
node .ado/scripts/build.js --no-build --platform arm64ec --configuration release --binskim

# Configure only (no build)
node .ado/scripts/build.js --configure --no-build --platform x64

# Clean build
node .ado/scripts/build.js --clean-build --platform x64
```

### Build Output

Build output goes to: `out/build/win32-<platform>-<config>/`

### Build Script

The main build script is `.ado/scripts/build.js`. It handles:
- Visual Studio environment setup via `vcvarsall.bat`
- CMake configuration with correct flags per platform
- Building, testing, packaging, and BinSkim validation

## Code Architecture

The build produces two VM variants: "regular" (full VM with compiler) and "lean" (excludes parser and compiler for smaller binary size).

### Core Components

- **lib/VM/**: Virtual machine core - runtime, interpreter, garbage collector, object model
- **lib/VM/JSLib/**: JavaScript standard library implemented in C++ (Array, Object, String, etc.)
- **lib/InternalJavaScript/**: JavaScript polyfills compiled into the VM (e.g., Math.sumPrecise, Promise)
- **lib/Parser/**: JavaScript parser
- **lib/AST/**: Abstract syntax tree definitions
- **lib/IR/**: Intermediate representation for optimization
- **lib/IRGen/**: Generates IR from AST
- **lib/BCGen/**: Bytecode generation from IR
- **lib/Sema/**: Semantic analysis
- **lib/Support/**: Shared utilities and data structures
- **include/hermes/**: Public header files organized by component
- **API/hermes/extensions/**: JSI-based runtime extensions (see Extensions section below)

### Key VM Files

- `lib/VM/Runtime.cpp`: Main runtime implementation
- `lib/VM/Interpreter.cpp`: Bytecode interpreter
- `lib/VM/Callable.cpp`: Function and callable object implementation
- `lib/VM/JSObject.cpp`: JavaScript object model
- `lib/VM/Operations.cpp`: Core JS operations (typeof, instanceof, etc.)
- `lib/VM/gcs/`: Garbage collector implementations

### Windows-Specific Files

- `cmake/modules/HermesWindows.cmake`: Windows build configuration (MSVC/Clang flags, security flags, linker flags)
- `.ado/scripts/build.js`: Main build orchestration script
- `dev.ps1`: PowerShell developer wrapper

### Testing

- `test/`: Lit-based integration tests organized by component
- `unittests/`: Google Test unit tests (VMRuntime, Support, API, Parser, etc.)

### Auto-Updating Tests

Some lit tests use `%FileCheckOrRegen` instead of `%FileCheck`. These are auto-updating tests whose expected output can be regenerated automatically.

If such a test fails due to intentional changes (e.g., changed output format, added runtime modules affecting IDs), and you understand why the failure occurred, rebuild with the `update-lit` target.

**Important:** Only use `update-lit` when you understand the cause of the failure. Review the changes to verify they match your expectations. Do not blindly regenerate tests to make them pass.

## Code Style

Key conventions:

- **C++17**, no exceptions or RTTI
- **Naming**: Classes (`PascalCase`), functions/methods (`camelCase`), variables (`camelCase`), member vars (`_suffix`), constants (`SNAKE_CASE` or `kCamelCase`)
- **structs**: PODs only; use `class` for anything with constructors/destructors
- **Line limit**: 80 characters, 2-space indent
- **Doc comments**: Required for every declaration
- **Inlining**: Only trivial one-line methods in class body

## Native Function Development

Standard signature for VM native functions:
```cpp
CallResult<HermesValue> funcName(void *context, Runtime &runtime)
```

Access arguments:
```cpp
NativeArgs args = runtime.getCurrentFrame().getNativeArgs();
```

## Macro-Generated Code

When making systematic changes, check for macro-generated code:
- `NATIVE_ERROR_TYPE` macro in `lib/VM/JSLib/Error.cpp` - generates error constructors
- `TYPED_ARRAY` macro in `lib/VM/JSLib/TypedArray.cpp` - generates typed array constructors
- `NATIVE_FUNCTION` macro in `include/hermes/VM/JSNativeFunctions.h` - generates function declarations

## GC Handle Patterns

### Locals Pattern (preferred for new code)
```cpp
struct : public Locals {
  PinnedValue<JSObject> objHandle;
  PinnedValue<PropertyAccessor> accessor;
  PinnedValue<> tempValue;  // untyped
} lv;
LocalsRAII lraii(runtime, &lv);

// Assignment from PseudoHandle
lv.value = std::move(pseudoHandle);

// Assignment from CallResult with known type
lv.obj.castAndSetHermesValue<JSObject>(callResult.getValue());
```

### Critical: Null Prototype Handling
```cpp
// When traversing prototype chains, always check for null:
if (!*protoRes) {
  lv.O = nullptr;
} else {
  lv.O.castAndSetHermesValue<JSObject>(protoRes->getHermesValue());
}
```

## Copyright Header

```cpp
/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */
```

## Extensions System

Hermes supports JSI-based extensions that add runtime functionality. Extensions use the stable JSI API rather than internal Hermes APIs, making them easier to maintain.

### Directory Structure

- `API/hermes/extensions/` - Core extensions maintained by the Hermes team
- `API/hermes/extensions/contrib/` - Community-contributed extensions

### Adding Extensions

Each extension consists of:
1. A JavaScript file (`NN-ExtensionName.js`) with a setup function
2. C++ files (`ExtensionName.h/cpp`) that call the JS setup and optionally provide native helpers

See `API/hermes/extensions/README.md` for detailed instructions.

### Contrib Extensions

Community contributions go in `extensions/contrib/`. These are:
- Maintained by contributors, not the Hermes team
- Enabled by default, but can be disabled with `-DHERMES_ENABLE_CONTRIB_EXTENSIONS=OFF`
- May be promoted to core if widely adopted

See `API/hermes/extensions/contrib/README.md` for contributor guidelines.
