# LLVH

This is a copy of LLVM libraries and headers that are used by Hermes.

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
