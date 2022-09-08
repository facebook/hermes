#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

import argparse
import enum
import os
import re
import shutil
import subprocess
import sys
import tempfile
import textwrap
import time
from collections import namedtuple
from multiprocessing import Pool, Value
from os import path


try:
    import testsuite.esprima_test_runner as esprima
    from testsuite.testsuite_skiplist import (
        HANDLESAN_SKIP_LIST,
        INTL_TESTS,
        LAZY_SKIP_LIST,
        PERMANENT_SKIP_LIST,
        PERMANENT_UNSUPPORTED_FEATURES,
        SKIP_LIST,
        UNSUPPORTED_FEATURES,
    )
except ImportError:
    import esprima_test_runner as esprima

    # Hacky way to handle non-buck builds that call the file immediately.
    from testsuite_skiplist import (
        HANDLESAN_SKIP_LIST,
        INTL_TESTS,
        LAZY_SKIP_LIST,
        PERMANENT_SKIP_LIST,
        PERMANENT_UNSUPPORTED_FEATURES,
        SKIP_LIST,
        UNSUPPORTED_FEATURES,
    )


## This is a simple script that runs the hermes compiler on
## external test suites.  The script expects to find the hermes compiler under
## ./bin/hermes. The script adds
## some basic test built-ins such as assertTrue, assertEquals that are used in
## the V8 test suite.

## How results are computed:
## If a test is on the skip list or contains unsupported ES6 features,
## it is skipped, and thus not executed at all.

## Result classes:
## Compile fail:    The Hermes compiler failed when it should have succeeded,
##                  or vice versa.
## Compile timeout: The Hermes compiler timed out.
## Execute fail:    Bytecode execution with the chosen backend
##                  failed when it should have succeeded, or vice versa.
## Execute timeout: Bytecode execution with the chosen backend timed out.

## Strictness:
## The default strictness mode is currently non-strict.
## For the test suites themselves:
## - test262: Require the test to pass in both strictness modes,
##            generating source code for both modes automatically.
## - mjsunit: Run the tests in non-strict mode,
##            because the test suite adds its own "use strict" directives.

## The content of this string is prepended to the test files and is used to
## provide the basic test built-ins.
test_builtins_content = """
// v8 test harness:
function internal_arraysEqual(a, b) {
  if (a === b) return true;
  if (a.length != b.length) return false;
  for (var i = 0; i < a.length; ++i) { if (a[i] !== b[i]) return false; }
  return true;
}
function builtin_nop(x) { return x; }
function builtin_false() { return false; }

var nopSentinel = {};

function v8pragma_HaveSameMap(obj1, obj2) {
  // This function doesn't work for all tests, but works for many.
  var keysAreSubset = function(lhs, rhs) {
    for (var property in lhs) {
      if (lhs[property] !== rhs[property]) {
        return false;
      }
    }
    return true;
  }
  return keysAreSubset(obj1, obj2) && keysAreSubset(obj2, obj1);
}

function v8pragma_FunctionSetPrototype(f, p) {
  // Set f.prototype.
  f.prototype = p;
}

function v8pragma_ClassOf(obj) {
  // Turn "[object ClassName]" into just "ClassName".
  return Object.prototype.toString.call(obj).slice(8, -1);
}

function v8pragma_Call(f, thisVal) {
  return f.apply(thisVal, Array.prototype.slice.call(arguments, 2));
}

function v8pragma_StringCharFromCode(i) {
  return String.fromCharCode(i);
}

function v8pragma_StringCharCodeAt(s, i) {
  return s.charCodeAt(i);
}

// debug variable sometimes used in mjsunit.
// Implemented the same way JSC does.
var debug = function(s) {
  alert('-->', s);
};

// The idea here is that some pragmas are meaningless for our JS interpreter,
// but we don't want to throw out the whole test case. In those cases, just
// throw out the assertions in those test cases resulting from checking the
// results of those pragmas.
function v8pragma_NopSentinel() {
  return nopSentinel;
}

// test262 requirements.
// Leave the unimplemented features unset in $262.
var $262 = {};
$262.global = this;
$262.evalScript = eval;
if (typeof HermesInternal === 'object') {
  $262.detachArrayBuffer = HermesInternal.detachArrayBuffer;
}

// Browser functions:
var alert = typeof print !== 'undefined' ? print : console.log;

"""


# Colors for stdout.
@enum.unique
class Color(enum.Enum):
    RESET = enum.auto()
    RED = enum.auto()
    GREEN = enum.auto()

    def __str__(self):
        if not sys.stdout.isatty():
            return ""
        return {
            Color.RESET.value: "\033[0m",
            Color.RED.value: "\033[31m",
            Color.GREEN.value: "\033[32m",
        }[self.value]


# These flags indicate the status of a job.
@enum.unique
class TestFlag(enum.Enum):
    TEST_FAILED = enum.auto()
    TEST_PASSED = enum.auto()
    TEST_SKIPPED = enum.auto()
    TEST_PERMANENTLY_SKIPPED = enum.auto()
    TEST_UNEXPECTED_PASSED = enum.auto()
    COMPILE_FAILED = enum.auto()
    COMPILE_TIMEOUT = enum.auto()
    EXECUTE_FAILED = enum.auto()
    EXECUTE_TIMEOUT = enum.auto()

    def __str__(self):
        return {
            TestFlag.TEST_FAILED.value: "TEST_FAILED",
            TestFlag.TEST_PASSED.value: "TEST_PASSED",
            TestFlag.TEST_SKIPPED.value: "TEST_SKIPPED",
            TestFlag.TEST_PERMANENTLY_SKIPPED.value: "TEST_PERMANENTLY_SKIPPED",
            TestFlag.TEST_UNEXPECTED_PASSED.value: "TEST_UNEXPECTED_PASSED",
            TestFlag.COMPILE_FAILED.value: "COMPILE_FAILED",
            TestFlag.COMPILE_TIMEOUT.value: "COMPILE_TIMEOUT",
            TestFlag.EXECUTE_FAILED.value: "EXECUTE_FAILED",
            TestFlag.EXECUTE_TIMEOUT.value: "EXECUTE_TIMEOUT",
        }[self.value]


TIMEOUT_COMPILER = 200
TIMEOUT_VM = 200

includesMatcher = re.compile(r"includes:\s*\[(.*)\]")
# This matches a special case in which the includes looks like:
# includes:
#    - foo.js
# This regex works only because the few cases which use this pattern
# only include one file.
specialIncludesMatcher = re.compile(
    "includes:\n" r".*-\s*(.*\.js)" "\n", re.MULTILINE | re.DOTALL
)


def generateSource(content, strict, suite, flags):
    """
    Generate the source code for a test case resulting from resolving pragmas in
    the given file and adding a use-strict directive, if necessary.
    Return a tuple: (source, includes)
    """

    # The raw flag specifies that the source code shouldn't be modified.
    if "raw" in flags:
        return (content, [])

    v8_pragmas = {
        "%OptimizeObjectForAddingMultipleProperties": "builtin_nop",
        "%ClearFunctionTypeFeedback": "builtin_nop",
        "%OptimizeFunctionOnNextCall": "builtin_nop",
        "%DeoptimizeFunction": "builtin_nop",
        "%DeoptimizeNow": "builtin_nop",
        "%_DeoptimizeNow": "builtin_nop",
        "%NeverOptimizeFunction": "builtin_nop",
        "%OptimizeOsr": "builtin_nop",
        "%ClearFunctionTypeFeedback": "builtin_nop",
        "%BaselineFunctionOnNextCall": "builtin_nop",
        "%SetForceInlineFlag": "builtin_nop",
        "%OptimizeObjectForAddingMultipleProperties": "builtin_nop",
        "%ToFastProperties": "builtin_nop",
        "%NormalizeElements": "builtin_nop",
        "%ArrayBufferNeuter": "HermesInternal.detachArrayBuffer",
        # ArrayBufferDetach is the more modern version of ArrayBufferNeuter.
        "%ArrayBufferDetach": "HermesInternal.detachArrayBuffer",
        "%RunMicrotasks": "builtin_nop",
        "%SetAllocationTimeout": "builtin_nop",
        "%UnblockConcurrentRecompilation": "builtin_nop",
        "%DebugPrint": "builtin_nop",
        "%HaveSameMap": "v8pragma_HaveSameMap",
        "%HasFastDoubleElements": "v8pragma_NopSentinel",
        "%HasFastSmiElements": "v8pragma_NopSentinel",
        "%HasFastObjectElements": "v8pragma_NopSentinel",
        "%HasFastHoleyElements": "v8pragma_NopSentinel",
        "%HasFastProperties": "v8pragma_NopSentinel",
        "%IsAsmWasmCode": "v8pragma_NopSentinel",
        "%IsNotAsmWasmCode": "v8pragma_NopSentinel",
        "%NotifyContextDisposed": "v8pragma_NopSentinel",
        "%FunctionSetPrototype": "v8pragma_FunctionSetPrototype",
        "%_ClassOf": "v8pragma_ClassOf",
        "%_Call": "v8pragma_Call",
        "%RunningInSimulator": "builtin_false",
        "%IsConcurrentRecompilationSupported": "builtin_false",
        "%_StringCharFromCode": "v8pragma_StringCharFromCode",
        "%_StringCharCodeAt": "v8pragma_StringCharCodeAt",
    }
    for pragma, replacement in v8_pragmas.items():
        content = content.replace(pragma, replacement)

    source = ""
    if strict:
        source += "'use strict';\n"

    includes = []
    if suite:
        if "test262" in suite:
            match = includesMatcher.search(content)
            includes = ["assert.js", "sta.js"]
            if match:
                includes += [i.strip() for i in match.group(1).split(",")]
            match = specialIncludesMatcher.search(content)
            if match:
                includes.append(match.group(1))
            for i in includes:
                filepath = path.join(suite, "harness", i)
                with open(filepath, "rb") as f:
                    source += f.read().decode("utf-8") + "\n"
        if "mjsunit" in suite:
            filepath = path.join(suite, "mjsunit.js")
            with open(filepath, "rb") as f:
                source += f.read().decode("utf-8") + "\n"

    source += test_builtins_content
    source += content
    return (source, includes)


evalMatcher = re.compile(r"\beval\s*\(")
indirectEvalMatcher = re.compile(r"\(.*,\s*eval\)\s*\(")
assignEvalMatcher = re.compile(r"=\s*eval\s*;")
withMatcher = re.compile(r"\bwith\s*\(")
constMatcher = re.compile(r"\bconst\b")
negativeMatcher = re.compile(
    r"""
    /\*---.*
    negative:.*\n
    \s*phase:\s*(\S+).*\n
    \s*type:\s*(\S+).*\n
    ---\*/
    """,
    re.MULTILINE | re.DOTALL | re.VERBOSE,
)
negativeMatcher2 = re.compile(
    r"""
    /\*---.*
    negative:.*\n
    \s*type:\s*(\S+).*\n
    \s*phase:\s*(\S+).*\n
    ---\*/
    """,
    re.MULTILINE | re.DOTALL | re.VERBOSE,
)
flagsMatcher = re.compile(r"\s*flags:\s*\[(.*)\]")
featuresMatcher = re.compile(r"\s*features:\s*\[(.*)\]")

# Alternate features syntax has "features:" and then bullet points using "-".
featuresMatcher2 = re.compile(r"\s*features:\s*\n(.*)\*\/", re.MULTILINE | re.DOTALL)


def getSuite(filename):
    suite = None
    # Try all possible test suites to see which one we're in.
    for s in ["test262", "mjsunit", "CVEs", "esprima", "flow"]:
        if (s + "/") in filename:
            suite = filename[: filename.find(s) + len(s)]
            break
    return suite


verbose = False


def printVerbose(s):
    global verbose
    if verbose:
        print(s)


istty = sys.stdout.isatty()
completed = Value("i", 0)
ttyWidth = os.get_terminal_size().columns if istty else 0
count = -1


def showStatus(filename):
    global completed, istty, verbose, count
    if istty and not verbose and count > 0:
        with completed.get_lock():
            record = ("\r{:" + str(ttyWidth) + "s}\n").format("Testing " + filename)
            status = "{:06.2f}% ({:d} / {:d})".format(
                100.0 * completed.value / count, completed.value, count
            )
            sys.stdout.write(record + status)
            sys.stdout.flush()
            completed.value += 1
    else:
        print("Testing " + filename)


es6_args = ["-Xes6-promise", "-Xes6-proxy"]
extra_run_args = ["-Xhermes-internal-test-methods"]
useMicrotasksFlag = ["-Xmicrotask-queue"]

extra_compile_flags = ["-fno-static-builtins"]


def fileInSkiplist(filename, skiplist):
    for blName in skiplist:
        if isinstance(blName, str):
            if blName in filename:
                return True
        else:
            # Assume it's a regex if it's not a string.
            if blName.search(filename):
                return True
    return False


# should_run: bool, If the test should run
# skip_reason: str, Reason for skipping, if the test shouldn't be run.
#               Empty if the test should be run (str)
# permanent: bool, If the test shouldn't be run, whether that condition is permanent
# flags: Set[str], The flags that were found for the file
# strict_modes: List[str], The strict modes that this file should be run with
TestContentParameters = namedtuple(
    "TestContentFlags",
    ["should_run", "skip_reason", "permanent", "flags", "strict_modes"],
)


def testShouldRun(filename, content):
    suite = getSuite(filename)

    # Determine flags and strict modes before deciding to skip a test case.
    flags = set()
    strictModes = []

    if not suite:
        strictModes = [False]
    else:
        if "test262" in suite:
            match = flagsMatcher.search(content)
            if match:
                flags = {flag.strip() for flag in match.group(1).split(",")}
                if "onlyStrict" in flags:
                    strictModes = [True]
                elif "noStrict" in flags or "raw" in flags:
                    strictModes = [False]
                else:
                    strictModes = [True, False]
            else:
                strictModes = [True, False]
        elif "mjsunit" in suite:
            strictModes = [False]
        elif "CVEs" in suite:
            strictModes = [False]
        else:
            raise Exception("Unknown suite")

    # Now find if this test case should be skipped.

    if "async" in flags:
        # We don't support async operations.
        return TestContentParameters(
            False, "Skipping test with async", False, flags, strictModes
        )
    if "module" in flags:
        # We don't support module code.
        return TestContentParameters(
            False, "Skipping test with modules", False, flags, strictModes
        )

    # Be picky about which tests to run unless we are running the CVEs suite
    runAll = "CVEs" in suite
    if not runAll:
        # Skip tests that use 'eval'.
        if evalMatcher.search(content):
            return TestContentParameters(
                False, "Skipping test with eval()", True, flags, strictModes
            )
        # Skip tests that use indirect 'eval' that look like (1, eval)(...).
        if indirectEvalMatcher.search(content):
            return TestContentParameters(
                False, "Skipping test with indirect eval()", True, flags, strictModes
            )
        # Skip tests that use indirect 'eval' by assigning a variable to eval.
        if assignEvalMatcher.search(content):
            return TestContentParameters(
                False, "Skipping test with alias to eval()", True, flags, strictModes
            )
        # Skip tests that use 'with'.
        if withMatcher.search(content):
            return TestContentParameters(
                False, "Skipping test with with()", True, flags, strictModes
            )
        if constMatcher.search(content):
            return TestContentParameters(
                False, "Skipping test with 'const'", False, flags, strictModes
            )

    if suite and "test262" in suite:
        # Skip unsupported features.
        match = featuresMatcher.search(content)
        match2 = featuresMatcher2.search(content)
        features = set()
        if match:
            features.update(feature.strip() for feature in match.group(1).split(","))
        if match2:
            features.update(
                feature.strip(" \t\n\r-") for feature in match2.group(1).split("\n")
            )
        features.discard("")
        for f in features:
            if f in UNSUPPORTED_FEATURES + PERMANENT_UNSUPPORTED_FEATURES:
                return TestContentParameters(
                    False,
                    "Skipping unsupported feature: " + f,
                    f in PERMANENT_UNSUPPORTED_FEATURES,
                    flags,
                    strictModes,
                )

    return TestContentParameters(True, "", False, flags, strictModes)


ESPRIMA_TEST_STATUS_MAP = {
    esprima.TestStatus.TEST_PASSED: TestFlag.TEST_PASSED,
    esprima.TestStatus.TEST_FAILED: TestFlag.COMPILE_FAILED,
    esprima.TestStatus.TEST_SKIPPED: TestFlag.TEST_SKIPPED,
    esprima.TestStatus.TEST_TIMEOUT: TestFlag.COMPILE_TIMEOUT,
}


def runTest(
    filename,
    tests_home,
    test_skiplist,
    generate_only,
    workdir,
    binary_path,
    hvm,
    esprima_runner,
    lazy,
    test_intl,
):
    """
    Runs a single js test pointed by filename
    """
    baseFileName = path.basename(filename)
    suite = getSuite(filename)
    skiplisted = fileInSkiplist(filename, SKIP_LIST + PERMANENT_SKIP_LIST)

    if lazy:
        skiplisted = skiplisted or fileInSkiplist(filename, LAZY_SKIP_LIST)

    if not test_intl:
        skiplisted = skiplisted or fileInSkiplist(filename, INTL_TESTS)

    skippedType = (
        TestFlag.TEST_PERMANENTLY_SKIPPED
        if fileInSkiplist(filename, PERMANENT_SKIP_LIST)
        else TestFlag.TEST_SKIPPED
    )

    if skiplisted and not test_skiplist:
        printVerbose(
            "Skipping test in skiplist{}: {}".format(
                " (permanently)"
                if skippedType is TestFlag.TEST_PERMANENTLY_SKIPPED
                else "",
                filename,
            )
        )
        return (skippedType, "", 0)

    showStatus(filename)

    if "esprima" in suite or "flow" in suite:
        hermes_path = path.join(binary_path, "hermes")
        test_res = esprima_runner.run_test(suite, filename, hermes_path)
        if test_res[0] == esprima.TestStatus.TEST_PASSED and skiplisted:
            printVerbose("FAIL: A skiplisted test completed successfully")
            return (TestFlag.TEST_UNEXPECTED_PASSED, "", 0)
        return (
            ESPRIMA_TEST_STATUS_MAP[test_res[0]],
            "" if test_res[0] == esprima.TestStatus.TEST_PASSED else test_res[1],
            0,
        )

    content = open(filename, "rb").read().decode("utf-8")

    shouldRun, skipReason, permanent, flags, strictModes = testShouldRun(
        filename, content
    )

    js_sources = []
    for strictEnabled in strictModes:
        tempdir = path.join(workdir, path.dirname(path.relpath(filename, tests_home)))
        os.makedirs(tempdir, exist_ok=True)

        js_source = path.join(
            tempdir,
            "{basename}{strict}{flags}.js".format(
                basename=path.splitext(baseFileName)[0],
                strict=".strict" if strictEnabled else "",
                flags=("." + ("_".join(sorted(flags)))) if flags else "",
            ),
        )

        source, includes = generateSource(content, strictEnabled, suite, flags)
        source = source.encode("utf-8")
        if "testIntl.js" in includes:
            # No support for multiple Intl constructors in that file.
            return (TestFlag.TEST_SKIPPED, "", 0)

        with open(js_source, "wb") as f:
            f.write(source)
            js_sources.append(js_source)

        printVerbose("\n==============")
        printVerbose("Strict Mode: {}".format(str(strictEnabled)))
        printVerbose("Temp js file name: " + js_source)

    if generate_only or not shouldRun:
        skippedType = (
            TestFlag.TEST_SKIPPED
            if not permanent
            else TestFlag.TEST_PERMANENTLY_SKIPPED
        )
        if not test_skiplist:
            printVerbose(
                skipReason
                + "{}: ".format(" (permanently)" if permanent else "")
                + filename
            )
            return (skippedType, "", 0)

    # Check if the test is expected to fail, and how.
    negativePhase = ""
    m = negativeMatcher.search(content)
    if m:
        negativePhase = m.group(1)
    else:
        m = negativeMatcher2.search(content)
        if m:
            negativePhase = m.group(2)

    # Report the max duration of any successful run for the variants of a test.
    # Unsuccessful runs are ignored for simplicity.
    max_duration = 0
    for js_source in js_sources:
        if lazy:
            run_vm = True
            fileToRun = js_source
            start = time.time()
        else:
            errString = ""
            fileToRun = js_source + ".hbc"
            for optEnabled in (True, False):
                printVerbose("\nRunning with Hermes...")
                printVerbose("Optimization: {}".format(str(optEnabled)))
                run_vm = True
                start = time.time()

                # Compile to bytecode with Hermes.
                try:
                    printVerbose(f"Compiling: {js_source} to {fileToRun}")
                    args = [
                        path.join(binary_path, "hermesc"),
                        js_source,
                        "-hermes-parser",
                        "-emit-binary",
                        "-out",
                        fileToRun,
                    ] + extra_compile_flags
                    if optEnabled:
                        args.append("-O")
                    else:
                        args.append("-O0")
                    if strictEnabled:
                        args.append("-strict")
                    else:
                        args.append("-non-strict")
                    subprocess.check_output(
                        args, timeout=TIMEOUT_COMPILER, stderr=subprocess.STDOUT
                    )

                    if negativePhase == "early" or negativePhase == "parse":
                        run_vm = False
                        printVerbose(
                            "FAIL: Compilation failure expected on {} with Hermes".format(
                                baseFileName
                            )
                        )
                        # If the test was in the skiplist, it was possible a
                        # compiler failure was expected. Else, it is unexpected and
                        # will return a failure.
                        return (
                            (skippedType, "", 0)
                            if skiplisted
                            else (TestFlag.COMPILE_FAILED, "", 0)
                        )
                except subprocess.CalledProcessError as e:
                    run_vm = False
                    if negativePhase != "early" and negativePhase != "parse":
                        printVerbose(
                            "FAIL: Compilation failed on {} with Hermes".format(
                                baseFileName
                            )
                        )
                        errString = e.output.decode("utf-8").strip()
                        printVerbose(textwrap.indent(errString, "\t"))
                        return (
                            (skippedType, "", 0)
                            if skiplisted
                            else (TestFlag.COMPILE_FAILED, errString, 0)
                        )
                    printVerbose("PASS: Hermes correctly failed to compile")
                except subprocess.TimeoutExpired:
                    printVerbose(
                        "FAIL: Compilation timed out on {}".format(baseFileName)
                    )
                    return (
                        (skippedType, "", 0)
                        if skiplisted
                        else (TestFlag.COMPILE_TIMEOUT, "", 0)
                    )

        # If the compilation succeeded, run the bytecode with the specified VM.
        if run_vm:
            try:
                printVerbose("Running with HBC VM: {}".format(filename))
                # Run the hermes vm.
                if lazy:
                    binary = path.join(binary_path, "hermes")
                else:
                    binary = path.join(binary_path, hvm)
                disableHandleSanFlag = (
                    ["-gc-sanitize-handles=0"]
                    if fileInSkiplist(filename, HANDLESAN_SKIP_LIST)
                    else []
                )
                args = (
                    [binary, fileToRun]
                    + es6_args
                    + extra_run_args
                    + disableHandleSanFlag
                    + useMicrotasksFlag
                )
                if lazy:
                    args.append("-lazy")
                    if strictEnabled:
                        args.append("-strict")
                    else:
                        args.append("-non-strict")
                env = {"LC_ALL": "en_US.UTF-8"}
                if sys.platform == "linux":
                    env["ICU_DATA"] = binary_path
                printVerbose(" ".join(args))
                subprocess.check_output(
                    args, timeout=TIMEOUT_VM, stderr=subprocess.STDOUT, env=env
                )

                if (not lazy and negativePhase == "runtime") or (
                    lazy and negativePhase != ""
                ):
                    printVerbose("FAIL: Expected execution to throw")
                    return (
                        (skippedType, "", 0)
                        if skiplisted
                        else (TestFlag.EXECUTE_FAILED, "", 0)
                    )
                else:
                    printVerbose("PASS: Execution completed successfully")
            except subprocess.CalledProcessError as e:
                if (not lazy and negativePhase != "runtime") or (
                    lazy and negativePhase == ""
                ):
                    printVerbose(
                        "FAIL: Execution of {} threw unexpected error".format(filename)
                    )
                    printVerbose("Return code: {}".format(e.returncode))
                    if e.output:
                        printVerbose("Output:")
                        errString = e.output.decode("utf-8").strip()
                        printVerbose(textwrap.indent(errString, "\t"))
                    else:
                        printVerbose("No output received from process")
                    return (
                        (skippedType, "", 0)
                        if skiplisted
                        else (TestFlag.EXECUTE_FAILED, errString, 0)
                    )
                else:
                    printVerbose("PASS: Execution of binary threw an error as expected")
            except subprocess.TimeoutExpired:
                printVerbose("FAIL: Execution of binary timed out")
                return (
                    (skippedType, "", 0)
                    if skiplisted
                    else (TestFlag.EXECUTE_TIMEOUT, "", 0)
                )
        max_duration = max(max_duration, time.time() - start)

    if skiplisted:
        # If the test was skiplisted, but it passed successfully, consider that
        # an error case.
        printVerbose("FAIL: A skiplisted test completed successfully")
        return (TestFlag.TEST_UNEXPECTED_PASSED, "", max_duration)
    else:
        printVerbose("PASS: Test completed successfully")
        return (TestFlag.TEST_PASSED, "", max_duration)


def makeCalls(params, onlyfiles, rangeLeft, rangeRight):
    global count
    # Store all test parameters in calls[].
    calls = []
    count = -1
    for f in onlyfiles:
        count += 1
        if count < rangeLeft or count > rangeRight:
            continue
        calls.append((f,) + params)
    return calls


def calcParams(params):
    return (params[0], runTest(*params))


def testLoop(calls, jobs, fail_fast, num_slowest_tests):
    results = []

    # Histogram for results from the Hermes compiler.
    resultsHist = {
        TestFlag.COMPILE_FAILED: 0,
        TestFlag.COMPILE_TIMEOUT: 0,
        TestFlag.EXECUTE_FAILED: 0,
        TestFlag.EXECUTE_TIMEOUT: 0,
        TestFlag.TEST_PASSED: 0,
        TestFlag.TEST_SKIPPED: 0,
        TestFlag.TEST_PERMANENTLY_SKIPPED: 0,
        TestFlag.TEST_UNEXPECTED_PASSED: 0,
    }
    slowest_tests = [("", 0)] * num_slowest_tests

    with Pool(processes=jobs) as pool:
        for res in pool.imap_unordered(calcParams, calls, 1):
            testname = res[0]
            results.append(res)
            (hermesStatus, errString, duration) = res[1]
            resultsHist[hermesStatus] += 1
            insert_pos = len(slowest_tests)
            for i, (_, other_duration) in reversed(list(enumerate(slowest_tests))):
                if duration < other_duration:
                    break
                else:
                    insert_pos = i
            if insert_pos < len(slowest_tests):
                # If this was one of the slowest tests, push it into the list
                # and drop the bottom of the list.
                slowest_tests = (
                    slowest_tests[:insert_pos]
                    + [(testname, duration)]
                    + slowest_tests[insert_pos:-1]
                )
            if (
                fail_fast
                and hermesStatus != TestFlag.TEST_PASSED
                and hermesStatus != TestFlag.TEST_SKIPPED
                and hermesStatus != TestFlag.TEST_PERMANENTLY_SKIPPED
            ):
                break
    # Filter out missing test names in case there were fewer tests run than the top slowest tests.
    slowest_tests = [
        (testName, duration) for testName, duration in slowest_tests if testName
    ]
    return results, resultsHist, slowest_tests


def get_arg_parser():
    parser = argparse.ArgumentParser(description="Run javascript tests with Hermes.")
    parser.add_argument(
        "paths",
        type=str,
        nargs="+",
        help="Paths to test suite, can be either dir or file name",
    )
    parser.add_argument(
        "-c",
        "--chunk",
        dest="chunk",
        default=-1,
        type=int,
        help="Chunk ID (0, 1, 2), to only process 1/3 of all tests",
    )
    parser.add_argument(
        "--nuke-workdir",
        dest="nuke_workdir",
        default=False,
        action="store_true",
        help="Removes the work directory specified with --workdir if it exists.",
    )
    parser.add_argument(
        "-f",
        "--fast-fail",
        dest="fail_fast",
        action="store_true",
        help="Exit script immediately when a test failed.",
    )
    parser.add_argument(
        "-wd",
        "--workdir",
        dest="workdir",
        default=None,
        help=(
            "Specifies work directory where the test files will be generated. "
            "The work directory must not exist, unless -f is specified."
        ),
    )
    parser.add_argument(
        "--test-skiplist",
        dest="test_skiplist",
        action="store_true",
        help="Also test if tests in the skiplist fail",
    )
    parser.add_argument(
        "-a",
        "--show-all",
        dest="show_all",
        action="store_true",
        help="show results of successful tests.",
    )
    parser.add_argument(
        "--hvm-filename",
        dest="hvm_filename",
        default="hvm",
        help="Filename for hvm binary (e.g., hvm-lean)",
    )
    parser.add_argument(
        "-j",
        "--jobs",
        dest="jobs",
        default=None,
        type=int,
        help="Number of jobs to run simultaneously. By default "
        + "equal to the number of CPUs.",
    )
    parser.add_argument(
        "-m",
        "--match",
        dest="match",
        default=None,
        type=str,
        help="Optional. Substring that the test filename must "
        "contain in order to run.",
    )
    parser.add_argument(
        "-s",
        "--source",
        dest="source",
        action="store_true",
        help="Instead of running any tests, print the source of "
        "the matched test case (use -m/--match) to standard "
        "output, including any generated use-strict "
        "directives or stubbed pragmas. (You could then "
        "pipe this to hermes.)",
    )
    parser.add_argument(
        "--num-slowest-tests",
        dest="num_slowest_tests",
        type=int,
        default=10,
        help="Print the top N tests that take the longest time to execute on "
        "average, where N is the option value",
    )
    parser.add_argument(
        "-v",
        "--verbose",
        dest="verbose",
        default=False,
        action="store_true",
        help="Show intermediate output",
    )
    parser.add_argument(
        "--lazy",
        dest="lazy",
        default=False,
        action="store_true",
        help="Force lazy evaluation",
    )
    parser.add_argument(
        "--test-intl",
        dest="test_intl",
        default=False,
        action="store_true",
        help="Run supported Intl tests.",
    )
    parser.add_argument(
        "--generate-test-only",
        "-g",
        dest="generate_test_only",
        default=False,
        action="store_true",
        help="Generate/pre-process test sources only",
    )
    return parser


class _PersistentTemporaryDirectory:
    """Creates a work directory that's not automativally deleted.

    This is a drop in replacement for tempfile.TemporaryDirectory that does not
    clean itself up automatically. Used when the user specifies the work
    directory, which must not exist.
    """

    def __init__(self, workdir):
        try:
            os.makedirs(workdir, exist_ok=False)
        except FileExistsError:
            raise FileExistsError(
                "Work directory {} exists. Please remove it and try again".format(
                    workdir
                )
            )
        self.name = workdir

    def __enter__(self):
        return self.name

    def __exit__(self, ex_type, ex_value, ex_traceback):
        # re-raise exception.
        return ex_value is None


def test_workdir(workdir, nuke_workdir, **kwargs):
    if nuke_workdir and os.path.exists(workdir):
        shutil.rmtree(workdir)

    return (
        tempfile.TemporaryDirectory(**kwargs)
        if not workdir
        else (_PersistentTemporaryDirectory(workdir))
    )


def run(
    paths,
    chunk,
    fail_fast,
    binary_path,
    hvm,
    jobs,
    is_verbose,
    match,
    source,
    test_skiplist,
    num_slowest_tests,
    workdir,
    nuke_workdir,
    generate_test_only,
    show_all,
    lazy,
    test_intl,
):
    global count
    global verbose

    verbose = is_verbose

    onlyfiles = []
    tests_home = None
    for p in paths:
        if path.isdir(p):
            for root, _dirnames, filenames in os.walk(p):
                for filename in filenames:
                    onlyfiles.append(path.join(root, filename))
            tests_home = p
        elif path.isfile(p):
            tests_home = path.dirname(p)
            onlyfiles.append(p)
        else:
            print("Invalid path: " + p)
            sys.exit(1)

    def isTest(f):
        isFixture = "test262" in f and f.endswith("_FIXTURE.js")
        return f.endswith(".js") and not isFixture

    onlyfiles = [f for f in onlyfiles if isTest(f) if not match or match in f]

    # Generates the source for the single provided file,
    # without an extra "use strict" directive prepended to the file.
    # Handles [noStrict] and [onlyStrict] flags.
    if source:
        if len(onlyfiles) != 1:
            print("Need exactly one file matched by the -m/--match option.")
            print("Got these files: " + ", ".join(onlyfiles))
            sys.exit(1)
        with open(onlyfiles[0], "rb") as f:
            content = f.read().decode("utf-8")
            match = flagsMatcher.search(content)
            flags = set()
            if match:
                flags = {flag.strip() for flag in match.group(1).split(",")}
            strict = False
            if "noStrict" in flags or "raw" in flags:
                strict = False
            if "onlyStrict" in flags:
                strict = True
            print(generateSource(content, strict, getSuite(onlyfiles[0]), flags)[0])
        sys.exit(0)

    rangeLeft = 0
    rangeRight = len(onlyfiles) - 1
    if chunk != -1:
        if chunk == 0:
            rangeRight = rangeRight // 3 - 1
        elif chunk == 1:
            rangeLeft = rangeRight // 3
            rangeRight = rangeRight - rangeLeft
        elif chunk == 2:
            rangeLeft = rangeRight - rangeRight // 3 + 1
        else:
            print("Invalid chunk ID")
            sys.exit(1)

    if not path.isfile(path.join(binary_path, "hermes")):
        print("{} not found.".format(path.join(binary_path, "hermes")))
        sys.exit(1)

    if not path.isfile(path.join(binary_path, hvm)):
        print("{} not found.".format(path.join(binary_path, hvm)))
        sys.exit(1)

    esprima_runner = esprima.EsprimaTestRunner(verbose)

    with test_workdir(workdir, nuke_workdir) as workdir:
        calls = makeCalls(
            (
                tests_home,
                test_skiplist,
                generate_test_only,
                workdir,
                binary_path,
                hvm,
                esprima_runner,
                lazy,
                test_intl,
            ),
            onlyfiles,
            rangeLeft,
            rangeRight,
        )

        results, resultsHist, slowest_tests = testLoop(
            calls, jobs, fail_fast, num_slowest_tests
        )

    # Sort the results for easier reading of failed tests.
    results.sort(key=lambda f: f[1][0].value)
    if results:
        print("")
    for testName, (hermesStatus, errString, _) in results:
        if show_all or (
            (hermesStatus != TestFlag.TEST_PASSED)
            and (hermesStatus != TestFlag.TEST_SKIPPED)
            and (hermesStatus != TestFlag.TEST_PERMANENTLY_SKIPPED)
        ):
            print("{} {}".format(str(hermesStatus), testName))
        if errString:
            print("{}".format(textwrap.indent(errString, "\t")))

    if slowest_tests:
        print()
        print("Top {:d} slowest tests".format(len(slowest_tests)))
        maxNameWidth = 0
        maxNumWidth = 0
        for testName, duration in slowest_tests:
            maxNameWidth = max(maxNameWidth, len(testName))
            maxNumWidth = max(maxNumWidth, len("{:.3f}".format(duration)))
        for testName, duration in slowest_tests:
            print(
                "{:<{testNameWidth}} {:>{durationWidth}.3f}".format(
                    testName,
                    duration,
                    # Add 3 just in case it's right at the borderline
                    testNameWidth=maxNameWidth + 3,
                    durationWidth=maxNumWidth,
                )
            )
        print()

    total = sum(resultsHist.values())
    failed = (
        resultsHist[TestFlag.COMPILE_FAILED]
        + resultsHist[TestFlag.COMPILE_TIMEOUT]
        + resultsHist[TestFlag.EXECUTE_FAILED]
        + resultsHist[TestFlag.EXECUTE_TIMEOUT]
        + resultsHist[TestFlag.TEST_UNEXPECTED_PASSED]
    )
    eligible = (
        sum(resultsHist.values())
        - resultsHist[TestFlag.TEST_SKIPPED]
        - resultsHist[TestFlag.TEST_PERMANENTLY_SKIPPED]
    )

    if eligible > 0:
        passRate = "{0:.2%}".format(resultsHist[TestFlag.TEST_PASSED] / eligible)
    else:
        passRate = "--"

    if (eligible - resultsHist[TestFlag.TEST_PASSED]) > 0:
        resultStr = "{}FAIL{}".format(Color.RED, Color.RESET)
    else:
        resultStr = "{}PASS{}".format(Color.GREEN, Color.RESET)

    # Turn off formatting so that the table looks nice in source code.
    # fmt: off
    print("-----------------------------------")
    print("| Results              |   {}   |".format(resultStr))
    print("|----------------------+----------|")
    print("| Total                | {:>8} |".format(total))
    print("| Pass                 | {:>8} |".format(resultsHist[TestFlag.TEST_PASSED]))
    print("| Fail                 | {:>8} |".format(failed))
    print("| Skipped              | {:>8} |".format(resultsHist[TestFlag.TEST_SKIPPED]))
    print("| Permanently Skipped  | {:>8} |".format(resultsHist[TestFlag.TEST_PERMANENTLY_SKIPPED]))
    print("| Pass Rate            | {:>8} |".format(passRate))
    print("-----------------------------------")
    print("| Failures             |          |")
    print("|----------------------+----------|")
    print("| Compile fail         | {:>8} |".format(resultsHist[TestFlag.COMPILE_FAILED]))
    print("| Compile timeout      | {:>8} |".format(resultsHist[TestFlag.COMPILE_TIMEOUT]))
    print("| Execute fail         | {:>8} |".format(resultsHist[TestFlag.EXECUTE_FAILED]))
    print("| Execute timeout      | {:>8} |".format(resultsHist[TestFlag.EXECUTE_TIMEOUT]))
    if test_skiplist:
        print("| Skiplisted passes    | {:>8} |".format(resultsHist[TestFlag.TEST_UNEXPECTED_PASSED]))
    print("-----------------------------------")

    # fmt: on

    return (eligible - resultsHist[TestFlag.TEST_PASSED]) > 0
