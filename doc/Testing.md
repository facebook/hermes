## Testing the Hermes compiler


The Hermes test suite contains three major categories: feature tests, regression
tests and conformance tests.  Feature tests are features that are added to the
compiler when developing a feature. For example, we test the output of IRGen and
look for specific patterns. Regression tests are tests that are added when we
fix bugs.  Both regression tests and feature tests are found under the "test/"
directory. To run the feature and regression tests run "make check-hermes".  We
also test the Hermes compiler with external test suites, described below.

## External test suites.

We rely on external test suites to test the Hermes compiler. We use the
run_benchmark script (located in the "utils/" directory) to run external test
suites. The script creates the test harness, executes one test at a time, and
reports a summary of the results. The script expects a directory that contains
plain JavaScript tests.

We currently support two test suites:

V8:
  repo: https://chromium.googlesource.com/v8/v8
  path: v8/test/mjsunit

Test262:
  repo: https://github.com/tc39/test262
  path: test262/test/language

To run the test suites clone the test repository and execute the run script from
within the hermes directory using the test path as an argument to the test
runner.

## Testing Hermes with libFuzzer

LibFuzzer is a library for in-process fuzzing that uses Sanitizer Coverage
instrumentation to guide test generation. With LibFuzzer one can implement a
guided fuzzer for some library by writing one simple entry point. Hermes has
supports for libFuzzer. The current support includes fuzzing of the frontend,
but not bytecode generation. The fuzzer support lives under /tools/fuzzer.

To configure Hermes to use libfuzzer, set the LIBFUZZER_PATH environment
variable to point to libfuzzer.a, that builds as part of libfuzzer.

More info about using libfuzzer : http://llvm.org/docs/LibFuzzer.html

