# Hermes Test Runner

## How to run

The runner requires two paths:
1. Path to the test suite directory that we support (i.e., test262, mjsunit,
esprima, flow and a CVE testsuite). This can be
a list separated by whitespace, so you can run multiple test suites at once.
2. Path to the directory of Hermes binaries, including `hermes` (and`shermes`
depending on extra flags).

Example usage:

```sh
# ./build is the cmake build directory.

# flow
./utils/test_runner.py external/flowtest/test/flow -b ./build/bin

# esprima
.utils/test_runner.py  external/esprima/test_fixtures -b ./build/bin
```

Use `--compile-args` and `--vm-args` if you want to apply extra compliler or VM
flags to run the testsuite. By default, we use the below flags for test262 and
the CVE testsuite:
```sh
# For hermes compiler
-test262 -fno-static-builtins -Xes6-block-scoping -Xenable-tdz
# For hermes VM (running the bytecode)
-Xes6-proxy -Xhermes-internal-test-methods -Xmicrotask-queue
```

Since esprima testsuite includes only AST tests, we simply use `-dump-ast`. For
flow testsuite, we use additionally below flags:
```sh
-parse-flow -Xparse-component-syntax -parse-jsx -Xinclude-empty-ast-nodes -Xparse-flow-match
```

## Skiplist

Run with flag `--test-skiplist` to force running tests that are included in the
`skiplist.json` config (but we will scan only "skiplist" and "manual_skiplist"
in the config, and "lazy_skip_list" if `--lazy` is given, "intl_tests" if
`--test-intl` is given).

After it finishes, all passed tests included in the skiplist config file will be
printed, and you can tell the runner to remove these tests from the config by
following the prompt ("manual_list" won't be touched). Note that if some tests
are covered by a folder path in the config, the test runner will first find all
tests under that folder, exclude those passed, and insert the rest into the
config file. If you don't want this behavior for a specific folder, please add
it to the manual list with a comment explaining the reason.
