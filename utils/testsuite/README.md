# Hermes Test Runner

## How to run

The runner (`cli.py`) requires two paths:
1. Path to the test suite directory that we support (i.e., test262, mjsunit, esprima, flow and a CVE testsuite). This can be
a list separated by whitespace, so you can run multiple test suites at once.
2. Path to the directory of Hermes binaries, including `hermesc`, `hvm` (and `hermes`, `shermes` depending on extra flags).

Example usage:

```sh
# ./build is the cmake build directory.

# flow
./utils/testsuite2/cli.py external/flowtest/test/flow -b ./build/bin

# esprima
./utils/testsuite2/cli.py external/esprima/test_fixtures -b ./build/bin
```

## Skiplist

Run with flag `--test-skiplist` to force running tests that are included in the `skiplist.json` config.

[TODO]
