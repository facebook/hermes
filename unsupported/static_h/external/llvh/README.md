# LLVH

This is a copy of LLVM libraries and headers that are used by Hermes.

## gtest patches

There's a bug in googletest-1.8.0 when compiling on Windows, with clang,
with exceptions disabled: `GTEST_HAS_EXCEPTIONS` isn't defined to the correct
value.
Pulled in this fix: https://github.com/google/googletest/commit/3498a1ac52deb83f30b8170c78bfba9dc6227198
from googletest-1.10.0 into googletest-1.8.0.
It only modifies `./utils/unittest/googletest/include/gtest/internal/gtest-port.h`.


## LIT patches

In order to fix some Windows python3 problems, it was necessary to pull in some
updates to the LIT testing tool from LLVM.

This was done like so:
```
git clone https://github.com/llvm/llvm-project.git
cd llvm-project
git checkout release/9.x
cp -r ./llvm/utils/lit/* /path/to/hermes/external/llvh/utils/lit
```

Some copied files were omitted because we don't use them
(like lit self-tests and examples).

The current LLVM 9 release is v9.0.1,
commit hash `c1a0a213378a458fbea1a5c77b315c7dce08fd05`, from November 27, 2019.

## Rename to LLVH

To avoid conflicts when linking with mainline LLVM, the namespace and include directories were
manually renamed to `llvh`. That left renaming some C public symbols. Unused files under `llvh-c/`
were removed and lastly a manual rename of the remaining global symbols was performed. The last
step is recorded in `patches/rename-c-interface.patch`.
