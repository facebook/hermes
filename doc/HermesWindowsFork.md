# Windows Fork Maintenance Strategy

This document outlines the purpose, principles, and build process for the
`hermes-windows` fork.

## Purpose

This repository is a fork of
[facebook/hermes](https://github.com/facebook/hermes) that exists to provide
robust, production-ready Windows support for the Hermes JavaScript engine. Our
goal is to enable first-class Windows integration, including features and
security requirements not present in the upstream repository, while closely
tracking upstream development.

## Guiding Principles

To ensure long-term maintainability and simplify the process of merging upstream
changes, we adhere to the following principles:

### 1. Minimize Upstream Diff

The primary goal is to keep this fork as close to `facebook/hermes` as possible.
All changes should be made with the intention of minimizing merge conflicts
during upstream pulls. Before making a change, consider if it can be implemented
without modifying an upstream file.

### 2. Isolate Windows-Specific Code

All Windows-specific build logic, compiler flags, and linker settings **MUST**
be placed in the central configuration file:
[`cmake/modules/HermesWindows.cmake`](../cmake/modules/HermesWindows.cmake).

**Do not add Windows-specific code to upstream files** like `CMakeLists.txt` or
`cmake/modules/Hermes.cmake`. This separation is critical for reducing merge
conflicts and clearly delineating our additions from the upstream codebase.

## Build System Strategy

Our build system is designed to be clean, maintainable, and easy to debug.

### The Role of `HermesWindows.cmake`

This file is the single source of truth for all Windows-specific build
configurations. It is included from the root `CMakeLists.txt` and provides
functions to configure compiler and linker flags for both MSVC and Clang on
Windows.

### The Override Pattern

A key challenge is that upstream `Hermes.cmake` disables certain warnings (e.g.,
C4244, C4267) that are required for Microsoft's security compliance. To solve
this without modifying the upstream file, we use an "override pattern" that
relies on MSVC's flag precedence, where **the last flag applied wins**.

1.  **Upstream (`Hermes.cmake`)**: Applies `-wd4244` (disables warning).
2.  **Our Fork (`HermesWindows.cmake`)**: Later applies `/w44244` (re-enables
    warning at level 4).

This powerful pattern allows us to enforce our requirements without creating a
diff in the upstream file, dramatically simplifying merges.

### Hybrid CRT Configuration

A major feature of this fork is the **Hybrid CRT configuration**, which ensures
that `hermes.dll` and related binaries do not have a dependency on
`msvcp140.dll` or `vcruntime140.dll`. Instead, they link against the Universal C
Runtime (UCRT), which is available on all modern Windows systems.

This is achieved by setting `CMAKE_MSVC_RUNTIME_LIBRARY` to `MultiThreaded`
(static) and then using linker flags to replace the static UCRT with the dynamic
(system) version. This simplifies deployment and eliminates the need for
consumers to install the Visual C++ Redistributable.

## Build Process

The repository uses `CMakePresets.json` to define the four primary build
configurations for Windows.

### Supported Presets

- `ninja-clang-debug`
- `ninja-clang-release`
- `ninja-msvc-debug`
- `ninja-msvc-release`

### Building Hermes

There are two primary ways to build the project:

#### Official Build Script (CI/CD)

The official method for building, used by our CI, is the `build.js` script:

```powershell
# Example: Build all configurations
node .ado\scripts\build.js
```

This script handles all configurations and ensures a complete, validated build.

#### Developer Builds

For development, a more lightweight approach is to use CMake presets directly.
This is ideal for quick builds within VS Code or for use by developer tools.

To configure and build a specific preset, use the following commands:

```powershell
# Example: Configure and build MSVC Release
cmake --preset ninja-msvc-release
cmake --build build/ninja-msvc-release
```

### Verifying Dependencies

After a build, you can verify that the Hybrid CRT is working correctly by
checking the dependencies of `hermes.dll`. It should **not** link to
`msvcp140.dll`.

```powershell
# Release builds should depend on api-ms-win-crt-*.dll
dumpbin /dependents build/ninja-msvc-release/API/hermes_shared/hermes.dll
```

## How to Contribute

When adding or modifying Windows-specific features:

1.  **Isolate Changes**: Place all build-related logic in
    `cmake/modules/HermesWindows.cmake`.
2.  **Maintain Upstream Files**: Do not add Windows-specific code to upstream
    CMake files. Use the override pattern where possible.
3.  **Test All Configurations**: Ensure your changes build successfully across
    all four supported presets.
4.  **Document**: If you introduce a new high-level strategy, update this
    document.

## Preparing Local Development Environment

- Github Desktop (recommemded but not required)
- Python
  - `winget install 9NQ7512CXL7T`
  - `py install [--update]`
- Tools included in azure devbox
  - Git for Windows
  - Powershell 7 (included in azure devbox)
    - `winget install Microsoft.Powershell` to install if not present
  - Visual Studio 2026, but currently using v143 build tools
    - Open vsinstaller, click `modify` in 2026 followed by `individual components`, install in total 4 packages:
      - C++ AST for latest v143 build tools with Spectre Mitigation (ARM64)/(x86 & x64)
      - MSVC v143 - VS 2022 C++ ARM64/ARM64EC//x64/x86 Spectre-mitigated libs
      - After upgrading to v145 package names are completely different
  - Visual Studio Code
    - Install `CMake Tools` (by Microsoft) and `C++ TestMate`

### Getting Started

- Open `x64 Native Tools Command Prompt for VS` and launch `code` from there.
- Follow [TestingWindowsWithVSCode.md](./TestingWindowsWithVSCode.md) to build and run unit test.
  - In `CMake` tab choose `Ninja + Clang (Debug)` in `PROJECT STATUS/Configure` before first using.
  - Click the `Build All Projects` button in `PROJECT OUTLINE` (ignore warnings in C++ code).
  - In `Testing` tab click the `Run Tests` button. Only available when the code compiles.
- If everything runs smoothly, you will see all test cases turned green.

## References

- **Upstream Hermes**: https://github.com/facebook/hermes
- **Hybrid CRT details**:
  https://github.com/microsoft/WindowsAppSDK/blob/main/docs/Coding-Guidelines/HybridCRT.md
- **MSVC Security Features**:
  https://learn.microsoft.com/en-us/cpp/build/reference/sdl-enable-additional-security-checks
