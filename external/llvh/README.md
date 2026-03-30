# LLVH

This is a copy of LLVM libraries and headers that are used by Hermes.
The copy was made from https://github.com/llvm-mirror/llvm.git with
hash c179d7b006348005d2da228aed4c3c251590baa3 .

## gtest patches

There's a bug in googletest-1.8.0 when compiling on Windows, with clang,
with exceptions disabled: `GTEST_HAS_EXCEPTIONS` isn't defined to the correct
value.
Pulled in this fix: https://github.com/google/googletest/commit/3498a1ac52deb83f30b8170c78bfba9dc6227198
from googletest-1.10.0 into googletest-1.8.0.
It only modifies `./utils/unittest/googletest/include/gtest/internal/gtest-port.h`.

Fixed cxxabi.h detection for MSVC under Wine. MSVC doesn't have cxxabi.h, and
`__has_include` is buggy under Wine (see https://bugs.winehq.org/show_bug.cgi?id=54130).
Patch: `./utils/unittest/googletest/patches/gtest-port-msvc-cxxabi.patch`.

Added `UniversalTersePrinter` specialization for `std::nullptr_t` to fix MSVC
ambiguous `operator<<` error.
Patch: `./utils/unittest/googletest/patches/gtest-printers-nullptr.patch`.

Added `UniversalTersePrinter` specialization for `char16_t*`. MSVC explicitly
deletes `operator<<` for `char16_t*` in `<ostream>`. Uses proper C-style
escaping consistent with gtest's `char*`/`wchar_t*` string printing.
Patch: `./utils/unittest/googletest/patches/gtest-printers-char16.patch`.


Suppressed `-Wundef` warnings in `gtest.h` and `gmock.h` by wrapping the
header contents with `#pragma GCC diagnostic push/ignored/pop`. These headers
use `#if` on macros that may not be defined, which is harmless but noisy under
`-Wundef`. Files modified:
- `./utils/unittest/googletest/include/gtest/gtest.h`
- `./utils/unittest/googlemock/include/gmock/gmock.h`

## LIT patches

In order to fix some Windows python3 problems, it was necessary to pull in some
updates to the LIT testing tool from LLVM.
The version used was LLVM release v9.0.1,
commit hash `c1a0a213378a458fbea1a5c77b315c7dce08fd05`, from November 27, 2019.

This was done like so:
```
git clone https://github.com/llvm/llvm-project.git
cd llvm-project
git checkout c1a0a213378a458fbea1a5c77b315c7dce08fd05
cp -r ./llvm/utils/lit/* /path/to/hermes/external/llvh/utils/lit
```

Some copied files were omitted because we don't use them
(like lit self-tests and examples).

Update: pull in the `run_under` argument in googletest.py, from LLVM release
v20.1.8, commit hash `87f0227cb60147a26a1eeb4fb06e3b505e9c7261`.

Skip empty RUN lines after substitution in `TestRunner.py`. When a
substitution replaces a RUN line with an empty string, the line is dropped.
If all RUN lines are dropped, the test is marked `UNSUPPORTED`. This allows
`lit.cfg` to disable specific tools (e.g. `%shermes`) by substituting them
with a marker and then replacing the entire line with an empty string.
Patch: `patches/lit-skip-empty-run-lines.patch`.

## Other imports from v9.0.1

Support/JSON and Support/ToolOutputFile.

## Rename to LLVH

To avoid conflicts when linking with mainline LLVM, the namespace and include directories were
manually renamed to `llvh`. That left renaming some C public symbols. Unused files under `llvh-c/`
were removed and lastly a manual rename of the remaining global symbols was performed. The last
step is recorded in `patches/rename-c-interface.patch`.

## abi-breaking.h

Upstream LLVM generates `abi-breaking.h` via CMake's `configure_file()`,
defining `LLVM_ENABLE_ABI_BREAKING_CHECKS` and `LLVM_ENABLE_REVERSE_ITERATION`.
LLVH does not use the LLVM build system, so `include/llvh/Config/abi-breaking.h`
is a hand-written header that defines both macros to 0. Without these
definitions, `#if LLVM_ENABLE_ABI_BREAKING_CHECKS` silently evaluates to 0
(correct behavior, but produces `-Wundef` warnings).
