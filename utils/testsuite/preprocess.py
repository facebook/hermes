# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

import os
from dataclasses import dataclass, field
from enum import auto, Flag
from typing import List

from .external.parse_test262 import parseTestRecord

from .typing_defs import OptExpectedFailure, PathT


class StrictMode(Flag):
    NO_STRICT = auto()
    STRICT = auto()
    ALL = NO_STRICT | STRICT


@dataclass
class TestCase:
    """
    Source code and all metadata provided in a test262 test case.
    For details about these metadata, see
    https://chromium.googlesource.com/external/github.com/tc39/test262/+/HEAD/CONTRIBUTING.md
    """

    source: str
    """Preprocessed source code."""
    includes: List[str] = field(default_factory=list)
    """
    A list of files in the test262/harness/ directory that needs to be
    included to run the test. The content of these included files has been
    prepended to the source field.
    """
    strict_mode: StrictMode = StrictMode.NO_STRICT
    """Run the test in strict or/and non-strict mode."""
    flags: List[str] = field(default_factory=list)
    """
    Flags specified in the test262 frontmatter, possible values are:
    onlyStrict, noStrict, module, raw, async, generated, CanBlockIsFalse,
    CanBlockIsTrue, non-deterministic.
    """
    expected_failure: OptExpectedFailure = None
    """
    The "negative" field in the test262 frontmatter. This means the test is
    expected to throw an error of given type. The two fields are (if not None):
    - phase, potential values are:
        - parse, meaning that the test must throw before execution
        - resolution, throw when performing ES2015 module resolution
        - runtime, thrown when executed the bytecode
    - type, the concrete error type
    """
    features: List[str] = field(default_factory=list)


def generate_test262_source(content: str, suite: PathT, filepath: PathT) -> TestCase:
    """
    Parse the give test262 test file and perform necessary preprocessing, e.g.,
    inserting code from included files to the test.
    """
    test = parseTestRecord(content, filepath)

    flags = test.get("flags", [])
    strict_mode = StrictMode.ALL
    if "onlyStrict" in flags:
        strict_mode = StrictMode.STRICT
    elif "noStrict" in flags or "raw" in flags:
        strict_mode = StrictMode.NO_STRICT

    negative = test.get("negative", None)

    includes = ["sta.js", "assert.js"]
    if "async" in flags:
        includes += ["doneprintHandle.js"]
    includes += test.get("includes", [])

    full_src = ""
    if "raw" not in flags:
        for include in includes:
            include_path = os.path.join(suite, "harness", include)
            with open(include_path, "rb") as f:
                full_src += f.read().decode("utf-8") + "\n"
    full_src += test["src"]

    return TestCase(
        source=full_src,
        includes=includes,
        strict_mode=strict_mode,
        flags=flags,
        expected_failure=negative,
        features=test.get("features", []),
    )


def generate_mjsunit_source(content: str, suite: str) -> TestCase:
    """
    Perform preprocessing on a given mjsunit test file.
    """

    # The content of this string is prepended to the test files and is used to
    # provide the basic test built-ins.
    v8_harness = """
// v8 test harness:
function internal_arraysEqual(a, b) {
  if (a === b) return true;
  if (a.length != b.length) return false;
  for (var i = 0; i < a.length; ++i) { if (a[i] !== b[i]) return false; }
  return true;
}
function builtin_nop(x) { return x; }
function builtin_false() { return false; }
function builtin_true() { return true; }

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

function v8pragma_StringMaxLength() {
  return 256 * 1024 * 1024;
}

function v8pragma_TypeOf(val) {
  return typeof val;
}

function v8pragma_ConstructConsString(a, b) {
  return a;
}

// For 64-bit devices, v8 may do extra tests regarding optimization, but we can skip
function v8pragma_Is64Bit() {
  return false;
}

function v8pragma_ConstructSlicedString(str, i) {
  return str.slice(i);
}

function v8pragma_ConstructInternalizedString(a, b) {
  return a + b;
}

function v8pragma_ToLength(val) {
  return val.length;
}

function executeCode(code) {
  if (typeof code === 'function') return code();
  if (typeof code === 'string') return eval(code);
  failWithMessage(
      'Given code is neither function nor string, but ' + (typeof code) +
      ': <' + prettyPrinted(code) + '>');
}

function v8_assertException(e, type_opt, cause_opt) {
    if (type_opt !== undefined) {
      assertEquals('function', typeof type_opt);
      assertInstanceof(e, type_opt);
    }
}

// Error messages may differ between V8 and Hermes, so ignore the assert on the message.
function v8_assertThrows(code, type_opt, cause_opt) {
    if (arguments.length > 1 && type_opt === undefined) {
      failWithMessage('invalid use of assertThrows, unknown type_opt given');
    }
    if (type_opt !== undefined && typeof type_opt !== 'function') {
      failWithMessage(
          'invalid use of assertThrows, maybe you want assertThrowsEquals');
    }
    try {
      executeCode(code);
    } catch (e) {
      assertException(e, type_opt, undefined);
      return;
    }
    let msg = 'Did not throw exception';
    if (type_opt !== undefined && type_opt.name !== undefined)
      msg += ', expected ' + type_opt.name;
    failWithMessage(msg);
  };

// Note that this is not added in the pragma map to avoid overriding other variants
// of assertThrows, e.g. assertThrowsEquals
assertThrows = v8_assertThrows;

// The idea here is that some pragmas are meaningless for our JS interpreter,
// but we don't want to throw out the whole test case. In those cases, just
// throw out the assertions in those test cases resulting from checking the
// results of those pragmas.
function v8pragma_NopSentinel() {
  return nopSentinel;
}

"""

    v8_pragmas = {
        "%ClearFunctionTypeFeedback": "builtin_nop",
        "%OptimizeFunctionOnNextCall": "builtin_nop",
        "%DeoptimizeFunction": "builtin_nop",
        "%DeoptimizeNow": "builtin_nop",
        "%_DeoptimizeNow": "builtin_nop",
        "%NeverOptimizeFunction": "builtin_nop",
        "%OptimizeOsr": "builtin_nop",
        "%BaselineFunctionOnNextCall": "builtin_nop",
        "%SetForceInlineFlag": "builtin_nop",
        "%OptimizeObjectForAddingMultipleProperties": "builtin_nop",
        "%ToFastProperties": "builtin_nop",
        "%NormalizeElements": "builtin_nop",
        "%ArrayBufferNeuter": "HermesInternal.detachArrayBuffer",
        # ArrayBufferDetach is the more modern version of ArrayBufferNeuter.
        "%ArrayBufferDetach": "HermesInternal.detachArrayBuffer",
        "%RunMicrotasks": "HermesInternal.drainJobs",
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
        "%AbortJS": "throw new Error",
        "%EnqueueMicrotask": "queueMicrotask",
        "%PerformMicrotaskCheckpoint": "HermesInternal.drainJobs",
        "%PrepareFunctionForOptimization": "builtin_nop",
        "%OptimizeMaglevOnNextCall": "builtin_nop",
        "%IsDictPropertyConstTrackingEnabled": "v8pragma_NopSentinel",
        "%HasSmiElements": "v8pragma_NopSentinel",
        "%GetOptimizationStatus": "v8pragma_NopSentinel",
        "%StringMaxLength": "v8pragma_StringMaxLength",
        "%EnsureFeedbackVectorForFunction": "builtin_nop",
        "%HeapObjectVerify": "builtin_nop",
        "%ClearFunctionFeedback": "builtin_nop",
        "%TurbofanStaticAssert": "builtin_nop",
        "%DisableOptimizationFinalization": "builtin_nop",
        "%FinalizeOptimization": "builtin_nop",
        "%SimulateNewspaceFull": "builtin_nop",
        "%BaselineOsr": "builtin_nop",
        "%ForceFlush": "builtin_nop",
        "%CompleteInobjectSlackTracking": "builtin_nop",
        "%DisassembleFunction": "builtin_nop",
        "%InternalizeString": "builtin_nop",
        "%SetForceSlowPath": "builtin_nop",
        "%RegexpIsUnmodified": "builtin_nop",
        "%NewRegExpWithBacktrackLimit": "builtin_nop",
        "%PretenureAllocationSite": "builtin_nop",
        "%WaitForBackgroundOptimization": "builtin_nop",
        "%Typeof": "v8pragma_TypeOf",
        "%CompileBaseline": "builtin_nop",
        "%ConstructConsString": "v8pragma_ConstructConsString",
        "%Is64Bit": "builtin_false",
        "%ToLength": "v8pragma_ToLength",
        "%IsUndefinedDoubleEnabled": "builtin_false",
        "%Call": "v8pragma_Call",
        "%ConstructThinString": "v8pragma_ConstructConsString",
        "%ConstructSlicedString": "v8pragma_ConstructSlicedString",
        "%SystemBreak": "builtin_nop",
        "%CollectGarbage": "builtin_nop",
        "assertException": "v8_assertException",
        "assertOptimized": "builtin_nop",
        "assertUnoptimized": "builtin_nop",
    }

    for pragma, replacement in v8_pragmas.items():
        content = content.replace(pragma, replacement)

    mjsunit_path = os.path.join(suite, "mjsunit.js")
    full_src = ""
    with open(mjsunit_path, "rb") as f:
        full_src += f.read().decode("utf-8") + "\n"
    full_src += v8_harness
    full_src += content
    return TestCase(source=full_src, strict_mode=StrictMode.NO_STRICT)


def generate_cves_source(content: str, test_name: str) -> TestCase:
    """
    Read potential metadata in CVEs tests (currently only negative field).
    """

    # Unlike test262, we don't want to print warnings of missing
    # header and frontmatter since most tests under CVEs do not have them.
    test = parseTestRecord(content, test_name, False)
    # A few CVEs tests use the same style metadata as test262
    negative = test.get("negative", None)
    return TestCase(
        source=test["src"], strict_mode=StrictMode.NO_STRICT, expected_failure=negative
    )


def generate_source(content: str, suite: str, test_name: str) -> TestCase:
    """
    Preprocess the test code for different testsuites, return the preprocessed
    source and metadata for it (e.g., strict mode, flags).
    """

    if "test262" in suite:
        return generate_test262_source(content, suite, test_name)
    if "mjsunit" in suite:
        return generate_mjsunit_source(content, suite)
    if "CVEs" in suite:
        return generate_cves_source(content, test_name)

    # For all other suites, return the exact source instead.
    return TestCase(content, strict_mode=StrictMode.NO_STRICT)
