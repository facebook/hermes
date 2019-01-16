#!/usr/bin/env python3

import argparse
import enum
import os
import re
import subprocess
import sys
import tempfile
import textwrap
from multiprocessing.dummy import Lock, Pool
from os.path import basename, isdir, isfile, join, splitext


try:
    from testsuite.testsuite_blacklist import BLACK_LIST, UNSUPPORTED_FEATURES
except ImportError:
    # Hacky way to handle non-buck builds that call the file immediately.
    from testsuite_blacklist import BLACK_LIST, UNSUPPORTED_FEATURES


## This is a simple script that runs the hermes compiler on
## external test suites.  The script expects to find the hermes compiler under
## ./bin/hermes. The script adds
## some basic test built-ins such as assertTrue, assertEquals that are used in
## the V8 test suite.

## How results are computed:
## If a test is blacklisted or contains unsupported ES6 features,
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
  print('-->', s);
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
var alert = print;

"""


# These flags indicate the status of a job.
@enum.unique
class TestFlag(enum.Enum):
    TEST_FAILED = enum.auto()
    TEST_PASSED = enum.auto()
    TEST_SKIPPED = enum.auto()
    COMPILE_FAILED = enum.auto()
    COMPILE_TIMEOUT = enum.auto()
    EXECUTE_FAILED = enum.auto()
    EXECUTE_TIMEOUT = enum.auto()

    def __str__(self):
        return {
            TestFlag.TEST_FAILED.value: "TEST_FAILED",
            TestFlag.TEST_PASSED.value: "TEST_PASSED",
            TestFlag.TEST_SKIPPED.value: "TEST_SKIPPED",
            TestFlag.COMPILE_FAILED.value: "COMPILE_FAILED",
            TestFlag.COMPILE_TIMEOUT.value: "COMPILE_TIMEOUT",
            TestFlag.EXECUTE_FAILED.value: "EXECUTE_FAILED",
            TestFlag.EXECUTE_TIMEOUT.value: "EXECUTE_TIMEOUT",
        }[self.value]


TIMEOUT_COMPILER = 40
TIMEOUT_VM = 200

includesMatcher = re.compile(r"includes:\s*\[(.*)\]")
# This matches a special case in which the includes looks like:
# includes:
#    - foo.js
# This regex works only because the few cases which use this pattern
# only include one file.
specialIncludesMatcher = re.compile(
    "includes:\n" ".*-\s*(.*\.js)\n", re.MULTILINE | re.DOTALL
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
        "%ArrayBufferNeuter": "builtin_nop",
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
                filepath = join(suite, "harness", i)
                with open(filepath, "rb") as f:
                    source += f.read().decode("utf-8") + "\n"
        if "mjsunit" in suite:
            filepath = join(suite, "mjsunit.js")
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
    r"/\*---.*" "negative:.*" "\n\s*phase:\s*(\S+).*" "\n\s*type:\s*(\S+).*" "\n---\*/",
    re.MULTILINE | re.DOTALL,
)
flagsMatcher = re.compile(r"\s*flags:\s*\[(.*)\]")
featuresMatcher = re.compile(r"\s*features:\s*\[(.*)\]")

# Alternate features syntax has "features:" and then bullet points using "-".
featuresMatcher2 = re.compile(r"\s*features:\s*\n(.*)\*\/", re.MULTILINE | re.DOTALL)


def getSuite(filename):
    suite = None
    # Try all possible test suites to see which one we're in.
    for s in ["test262", "mjsunit", "CVEs"]:
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
statusLock = Lock()
completed = 0
ttyWidth = os.get_terminal_size().columns if istty else 0


def showStatus(filename):
    global completed, istty, verbose, count, statusLock
    if istty and not verbose and count > 0:
        with statusLock:
            record = ("\r{:" + str(ttyWidth) + "s}\n").format("Testing " + filename)
            status = "{:06.2f}% ({:d} / {:d})".format(
                100.0 * completed / count, completed, count
            )
            sys.stdout.write(record + status)
            sys.stdout.flush()
            completed += 1
    else:
        print("Testing " + filename)


es6_args = ["-Xes6-symbol"]


def fileInBlacklist(filename):
    for blName in BLACK_LIST:
        if blName in filename:
            return True
    return False


def testShouldRun(filename, content):
    """Return a tuple of:
    * If the test should run (bool)
    * Reason for skipping, if the test shouldn't be run.
        Empty if the test should be run (str).
    * The flags that were found for the file (Set[str])
    * The strict modes that this file should be run with (List[str])
    """
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
        return False, "Skipping test with async", flags, strictModes
    if "module" in flags:
        # We don't support module code.
        return False, "Skipping test with modules", flags, strictModes

    # Be picky about which tests to run unless we are running the CVEs suite
    runAll = "CVEs" in suite
    if not runAll:
        # Skip tests that use 'eval'.
        if evalMatcher.search(content):
            return False, "Skipping test with eval()", flags, strictModes
        # Skip tests that use indirect 'eval' that look like (1, eval)(...).
        if indirectEvalMatcher.search(content):
            return False, "Skipping test with indirect eval()", flags, strictModes
        # Skip tests that use indirect 'eval' by assigning a variable to eval.
        if assignEvalMatcher.search(content):
            return False, "Skipping test with alias to eval()", flags, strictModes
        # Skip tests that use 'with'.
        if withMatcher.search(content):
            return False, "Skipping test with with()", flags, strictModes
        if constMatcher.search(content):
            return False, "Skipping test with 'const'", flags, strictModes

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
            if f in UNSUPPORTED_FEATURES:
                return False, "Skipping unsupported feature: " + f, flags, strictModes

    return True, "", flags, strictModes


def runTest(filename, keep_tmp, binary_path, hvm):
    """
    Runs a single js test pointed by \p filename
    """
    baseFileName = basename(filename)
    suite = getSuite(filename)

    if fileInBlacklist(filename):
        printVerbose("Skipping test in blacklist: {}".format(filename))
        return (TestFlag.TEST_SKIPPED, "")

    showStatus(filename)

    content = open(filename, "rb").read().decode("utf-8")

    shouldRun, skipReason, flags, strictModes = testShouldRun(filename, content)
    if not shouldRun:
        printVerbose(skipReason + ": " + filename)
        return (TestFlag.TEST_SKIPPED, "")

    # Check if the test is expected to fail, and how.
    m = negativeMatcher.search(content)
    negativePhase = m.group(1) if m else ""

    for strictEnabled in strictModes:
        temp = tempfile.NamedTemporaryFile(
            prefix=splitext(baseFileName)[0] + "-", suffix=".js", delete=False
        )
        source, includes = generateSource(content, strictEnabled, suite, flags)
        source = source.encode("utf-8")
        if "testIntl.js" in includes:
            # No support for multiple Intl constructors in that file.
            return (TestFlag.TEST_SKIPPED, "")
        temp.write(source)
        temp.close()

        printVerbose("\n==============")
        printVerbose("Strict Mode: {}".format(str(strictEnabled)))
        printVerbose("Temp js file name: " + temp.name)

        errString = ""
        binfile = tempfile.NamedTemporaryFile(
            prefix=splitext(baseFileName)[0] + "-", suffix=".hbc", delete=False
        )
        binfile.close()
        for optEnabled in (True, False):
            printVerbose("\nRunning with Hermes...")
            printVerbose("Optimization: {}".format(str(optEnabled)))
            run_vm = True

            # Compile to bytecode with Hermes.
            try:
                printVerbose("Compiling: {} to {}".format(filename, binfile.name))
                args = [
                    os.path.join(binary_path, "hermes"),
                    temp.name,
                    "-hermes-parser",
                    "-emit-binary",
                    "-out",
                    binfile.name,
                ] + es6_args
                if optEnabled:
                    args.append("-O")
                if not strictEnabled:
                    args.append("-non-strict")
                subprocess.check_output(
                    args, timeout=TIMEOUT_COMPILER, stderr=subprocess.STDOUT
                )

                if negativePhase == "early":
                    run_vm = False
                    printVerbose(
                        "FAIL: Compilation failure expected on {} with Hermes".format(
                            baseFileName
                        )
                    )
                    return (TestFlag.COMPILE_FAILED, "")
            except subprocess.CalledProcessError as e:
                run_vm = False
                if negativePhase != "early":
                    printVerbose(
                        "FAIL: Compilation failed on {} with Hermes".format(
                            baseFileName
                        )
                    )
                    errString = e.output.decode("utf-8").strip()
                    printVerbose(textwrap.indent(errString, "\t"))
                    return (TestFlag.COMPILE_FAILED, errString)
                printVerbose("PASS: Hermes correctly failed to compile")
            except subprocess.TimeoutExpired:
                printVerbose("FAIL: Compilation timed out on {}".format(baseFileName))
                return (TestFlag.COMPILE_TIMEOUT, "")

            # If the compilation succeeded, run the bytecode with the specified VM.
            if run_vm:
                try:
                    printVerbose("Running with HBC VM: {}".format(filename))
                    # Run the hermes vm.
                    args = [os.path.join(binary_path, hvm), binfile.name] + es6_args
                    env = {"LC_ALL": "en_US.UTF-8"}
                    if sys.platform == "linux":
                        env["ICU_DATA"] = binary_path
                    subprocess.check_output(
                        args, timeout=TIMEOUT_VM, stderr=subprocess.STDOUT, env=env
                    )

                    if negativePhase == "runtime":
                        printVerbose("FAIL: Expected execution to throw")
                        return (TestFlag.EXECUTE_FAILED, "")
                    else:
                        printVerbose("PASS: Execution completed successfully")
                except subprocess.CalledProcessError as e:
                    if negativePhase != "runtime":
                        printVerbose(
                            "FAIL: Execution of {} threw unexpected error".format(
                                filename
                            )
                        )
                        printVerbose("Return code: {}".format(e.returncode))
                        if e.output:
                            printVerbose("Output:")
                            errString = e.output.decode("utf-8").strip()
                            printVerbose(textwrap.indent(errString, "\t"))
                        else:
                            printVerbose("No output received from process")
                        return (TestFlag.EXECUTE_FAILED, errString)
                    else:
                        printVerbose(
                            "PASS: Execution of binary threw an error as expected"
                        )
                except subprocess.TimeoutExpired:
                    printVerbose("FAIL: Execution of binary timed out")
                    return (TestFlag.EXECUTE_TIMEOUT, "")

    if not keep_tmp:
        os.unlink(temp.name)
        os.unlink(binfile.name)

    printVerbose("PASS: Test completed successfully")
    return (TestFlag.TEST_PASSED, "")


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


def testLoop(calls, jobs, fail_fast):
    results = []

    # Histogram for results from the Hermes compiler.
    resultsHist = {
        TestFlag.COMPILE_FAILED: 0,
        TestFlag.COMPILE_TIMEOUT: 0,
        TestFlag.EXECUTE_FAILED: 0,
        TestFlag.EXECUTE_TIMEOUT: 0,
        TestFlag.TEST_PASSED: 0,
        TestFlag.TEST_SKIPPED: 0,
    }

    with Pool(processes=jobs) as pool:
        for res in pool.imap_unordered(
            lambda params: (params[0], runTest(*params)), calls, 1
        ):
            results.append(res)
            (hermesStatus, errString) = res[1]
            resultsHist[hermesStatus] += 1
            if (
                fail_fast
                and hermesStatus != TestFlag.TEST_PASSED
                and hermesStatus != TestFlag.TEST_SKIPPED
            ):
                break
    return results, resultsHist


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
        "-f",
        "--fast-fail",
        dest="fail_fast",
        action="store_true",
        help="Exit script immediately when a test failed.",
    )
    parser.add_argument(
        "-k",
        "--keep-tmp",
        dest="keep_tmp",
        action="store_true",
        help="Keep temporary files of successful tests.",
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
        "-v",
        "--verbose",
        dest="verbose",
        default=False,
        action="store_true",
        help="Show intermediate output",
    )
    return parser


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
    keep_tmp,
    show_all,
):
    global count
    global verbose

    verbose = is_verbose

    onlyfiles = []
    for path in paths:
        if isdir(path):
            for root, _dirnames, filenames in os.walk(path):
                for filename in filenames:
                    onlyfiles.append(os.path.join(root, filename))
        elif isfile(path):
            onlyfiles.append(path)
        else:
            print("Invalid path: " + path)
            sys.exit(1)

    onlyfiles = [f for f in onlyfiles if f.endswith(".js") if not match or match in f]

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

    if not os.path.isfile(join(binary_path, "hermes")):
        print("{} not found.".format(join(binary_path, "hermes")))
        sys.exit(1)

    if not os.path.isfile(join(binary_path, hvm)):
        print("{} not found.".format(join(binary_path, hvm)))
        sys.exit(1)

    calls = makeCalls((keep_tmp, binary_path, hvm), onlyfiles, rangeLeft, rangeRight)
    results, resultsHist = testLoop(calls, jobs, fail_fast)

    # Sort the results for easier reading of failed tests.
    results.sort(key=lambda f: f[1][0].value)
    if results:
        print("")
    for testName, (hermesStatus, errString) in results:
        if show_all or (
            (hermesStatus != TestFlag.TEST_PASSED)
            and (hermesStatus != TestFlag.TEST_SKIPPED)
        ):
            print("{} {}".format(str(hermesStatus), testName))
        if errString:
            print("{}".format(textwrap.indent(errString, "\t")))

    skipped = resultsHist[TestFlag.TEST_SKIPPED]

    eligible = sum(resultsHist.values()) - skipped
    if eligible > 0:
        passRate = "{0:.2%}".format(resultsHist[TestFlag.TEST_PASSED] / eligible)
    else:
        passRate = "--"

    print("------------------------------")
    print("| RESULTS: HVM    |          |")
    print("|-----------------+----------|")
    print("| Pass            | {:>8} |".format(resultsHist[TestFlag.TEST_PASSED]))
    print("|-----------------+----------|")
    print("| Compile fail    | {:>8} |".format(resultsHist[TestFlag.COMPILE_FAILED]))
    print("| Compile timeout | {:>8} |".format(resultsHist[TestFlag.COMPILE_TIMEOUT]))
    print("| Execute fail    | {:>8} |".format(resultsHist[TestFlag.EXECUTE_FAILED]))
    print("| Execute timeout | {:>8} |".format(resultsHist[TestFlag.EXECUTE_TIMEOUT]))
    print("------------------------------")

    print("")
    print("--------------------------------")
    print("| Tests run         | {:>8} |".format(eligible))
    print("| Tests skipped     | {:>8} |".format(skipped))
    print("| Pass Rate         | {:>8} |".format(passRate))
    print("--------------------------------")

    return (eligible - resultsHist[TestFlag.TEST_PASSED]) > 0
