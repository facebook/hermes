#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

import argparse
import asyncio
import importlib.resources
import os
import shutil
import sys
import tempfile
import time
from asyncio import Semaphore
from collections import defaultdict
from typing import Awaitable, Dict, List, Optional

from . import utils
from .hermes import ExtraCompileVMArgs
from .progress import (
    ProgressBar,
    SimpleProgressBar,
    TerminalController,
    TestCaseResult,
    TestingProgressDisplay,
)
from .skiplist import SkipCategory, SkippedPathsOrFeatures
from .test_run_defs import get_suite, StrictMode, TestRunArgs
from .typing_defs import PathT
from .utils import Color, TestResultCode

N_DEFAULT_CPU = 10
DEFAULT_TIMEOUT = 200
try:
    RESOURCE_ROOT = importlib.resources.files(__package__)
except TypeError:
    from pathlib import Path

    # With Python 3.9.6 on MacOS, importlib.resources.files raises a TypeError,
    # simply set it to the directory of current file as a fallback (it's unclear
    # whether this will always work).
    RESOURCE_ROOT = Path(os.path.dirname(__file__))


def create_parser():
    # Common argument for running all testsuite
    desp = """
    Run Hermes testsuite. Supported testsuites are:
    - test262
    - mjsunit
    - esprima
    - CVEs
    - flow
    """
    parser = argparse.ArgumentParser(description=desp)
    parser.add_argument(
        "-b",
        "--binary-dir",
        dest="binary_directory",
        default=RESOURCE_ROOT,
        help="Path to hermes built binary directory, default to the package root",
    )
    parser.add_argument(
        "-j",
        "--jobs",
        dest="n_jobs",
        default=os.cpu_count() or N_DEFAULT_CPU,
        type=int,
        help="Number of tests to run simultaneously, default is number of logical CPUs",
    )
    parser.add_argument(
        "--test-skiplist",
        dest="test_skiplist",
        action="store_true",
        help="Run tests in the skiplist and remove them from the list if passed",
    )
    parser.add_argument(
        "--test-intl",
        dest="test_intl",
        action="store_true",
        help="Run supported Intl tests",
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
        "--work-dir",
        dest="work_dir",
        default=None,
        type=str,
        help="Specifies work directory where the test files will be generated. "
        "The work directory will be deleted if it is not empty",
    )
    group = parser.add_mutually_exclusive_group()
    group.add_argument(
        "--lazy", dest="lazy", action="store_true", help="Force lazy evaluation"
    )
    group.add_argument(
        "--shermes", dest="shermes", action="store_true", help="Test with shermes"
    )
    parser.add_argument(
        "--opt", dest="opt", action="store_true", help="Enable compiler optimizations"
    )
    parser.add_argument(
        "--compile-args",
        dest="extra_compile_args",
        nargs="+",
        default=[],
        help="Extra compiler arguments, separated by whitespace",
    )
    parser.add_argument(
        "--vm-args",
        dest="extra_vm_args",
        nargs="+",
        default=[],
        help="Extra VM arguments, separated by whitespace",
    )
    parser.add_argument(
        "-d",
        "--dump-source",
        dest="dump_source",
        action="store_true",
        help="Instead of running any tests, print the preprocessed source of "
        "test case to standard output, including any generated use-strict "
        "directives or stubbed pragmas. When this flag is provided, only one "
        "path is expected.",
    )
    parser.add_argument(
        "--timeout",
        dest="timeout",
        default=DEFAULT_TIMEOUT,
        type=int,
        help="Timeout for compiling and running each test.",
    )
    parser.add_argument(
        "paths", type=str, nargs="+", help="Paths to testsuite, can be dir or file"
    )

    return parser


def validate_args(args: argparse.Namespace):
    """
    Validate provided arguments. For -b flag, check whether necessary Hermes
    executables do exist.
    """

    if args.dump_source:
        if len(args.paths) != 1:
            print("Argument error: Only one path is expected with --dump-source.")
            sys.exit(1)

    utils.check_hermes_exe(args.binary_directory, args.shermes)


def print_stats(stats: dict) -> None:
    """Print the result stats."""
    total = sum(stats.values())
    failed = (
        stats[TestResultCode.COMPILE_FAILED]
        + stats[TestResultCode.COMPILE_TIMEOUT]
        + stats[TestResultCode.EXECUTE_FAILED]
        + stats[TestResultCode.EXECUTE_TIMEOUT]
        + stats[TestResultCode.TEST_FAILED]
    )
    executed = (
        sum(stats.values())
        - stats[TestResultCode.TEST_SKIPPED]
        - stats[TestResultCode.TEST_PERMANENTLY_SKIPPED]
    )

    if executed > 0:
        passRate = "{0:.2%}".format(stats[TestResultCode.TEST_PASSED] / executed)
    else:
        passRate = "--"

    if (executed - stats[TestResultCode.TEST_PASSED]) > 0:
        resultStr = "{}FAIL{}".format(Color.RED, Color.RESET)
    else:
        resultStr = "{}PASS{}".format(Color.GREEN, Color.RESET)

    # Turn off formatting so that the table looks nice in source code.
    # fmt: off
    print("-----------------------------------")
    print("| Results              |   {}   |".format(resultStr))
    print("|----------------------+----------|")
    print("| Total                | {:>8} |".format(total))
    print("| Passes               | {:>8} |".format(stats[TestResultCode.TEST_PASSED]))
    print("| Failures             | {:>8} |".format(failed))
    print("| Skipped              | {:>8} |".format(stats[TestResultCode.TEST_SKIPPED]))
    print("| Permanently Skipped  | {:>8} |".format(stats[TestResultCode.TEST_PERMANENTLY_SKIPPED]))
    print("| Pass Rate            | {:>8} |".format(passRate))
    print("-----------------------------------")
    print("| Failures             |          |")
    print("|----------------------+----------|")
    print("| Compile fail         | {:>8} |".format(stats[TestResultCode.COMPILE_FAILED]))
    print("| Compile timeout      | {:>8} |".format(stats[TestResultCode.COMPILE_TIMEOUT]))
    print("| Execute fail         | {:>8} |".format(stats[TestResultCode.EXECUTE_FAILED]))
    print("| Execute timeout      | {:>8} |".format(stats[TestResultCode.EXECUTE_TIMEOUT]))
    print("| Other                | {:>8} |".format(stats[TestResultCode.TEST_FAILED]))
    print("-----------------------------------")
    # fmt: on


def print_failed_tests(tests: dict) -> None:
    def print_test_list(cat: TestResultCode, header: str):
        if len(tests[cat]) > 0:
            print("-----------------------------------")
            print(header)
            for test in tests[cat]:
                print(test)
            print("-----------------------------------")

    if len(tests) == 0:
        return
    print("\nDetails:")
    print_test_list(TestResultCode.COMPILE_FAILED, "Compile failed:")
    print_test_list(TestResultCode.COMPILE_TIMEOUT, "Compile timeout:")
    print_test_list(TestResultCode.EXECUTE_FAILED, "Execute failed:")
    print_test_list(TestResultCode.EXECUTE_TIMEOUT, "Execute timeout:")
    print_test_list(TestResultCode.TEST_FAILED, "Other test failure:")


def print_skipped_passed_tests(tests: Dict[str, str]) -> None:
    print("\nPassed tests in skiplist:")
    print("-----------------------------------")
    for test_name in tests:
        print(test_name)
    print("-----------------------------------")


def remove_tests_from_skiplist(
    tests: Dict[str, str], skipped_paths_features: SkippedPathsOrFeatures
) -> None:
    """Remove passed tests from the skiplist config."""

    ans = input("\nRemove these passed tests from skiplist? [y]es/[n]o: ").lower()
    if ans == "y" or ans == "yes":
        # Original full test names have form of "<suite> :: relative/path",
        # we need to convert them to form of "<suite>/relative/path" to match
        # paths in skiplist.
        tests_with_rel_paths = {
            test_name.replace(" :: ", os.sep): suite_dir
            for test_name, suite_dir in tests.items()
        }

        skipped_paths_features.remove_tests(tests_with_rel_paths)
        skipped_paths_features.persist()


async def run(
    tests_paths: List[PathT],
    binary_directory: PathT,
    skipped_paths_features: SkippedPathsOrFeatures,
    work_dir: PathT,
    n_jobs: int,
    test_skiplist: bool,
    test_intl: bool,
    lazy: bool,
    shermes: bool,
    opt: bool,
    extra_compile_vm_args: ExtraCompileVMArgs,
    timeout: int,
    verbose: bool,
) -> int:
    """
    Run all tests with async subprocess and wait for results in completion order
    of test jobs. Each subprocess invokes hermes binary on a given test file, so
    to restrict resource contention, we use asyncio.Semaphore to control the
    maximum number of alive tasks (configured by n_jobs).

    We first collect all tasks for given tests_paths, then wrap each of them
    in a new task guarded by Semaphore, and pass them to asyncio.as_completed().
    Each task can be awaited in it to get earliest next result. And once a
    task is done, its Semaphore counter is released and a new task can
    continue to run.

    After all tasks are done, print the test stats and relative paths of all
    failing tests (if there are).
    """

    # Get the common path of all file/directory paths to be used when creating
    # temporary files.
    tests_home = os.path.commonpath(tests_paths)
    tests_files = utils.list_all_files(tests_paths)

    header = f"-- Testing: {len(tests_files)} tests, max {n_jobs} concurrent tasks --"
    try:
        term = TerminalController()
        pb = ProgressBar(term, header)
    except ValueError:
        print(header)
        pb = SimpleProgressBar("Testing: ")
    pd = TestingProgressDisplay(len(tests_files), pb, verbose)
    current_n_tasks = Semaphore(n_jobs)

    start_time = time.time()
    tasks = []
    stats = defaultdict(int)
    # Record skipped tests and its suite path (as dict value).
    # we need the suite path because some paths in the skiplist config are
    # directories, we need to get their absolute paths to get all files under
    # them.
    skipped_tests = {}
    for test_file in tests_files:
        suite = get_suite(test_file)
        assert suite, f"Test suite root directory is not found for {test_file}"
        full_test_name = suite.get_full_test_name(test_file)

        # We always skip tests that are in permanent skiplist.
        if test_result := skipped_paths_features.try_skip(
            test_file, [SkipCategory.PERMANENT_SKIP_LIST], full_test_name
        ):
            pd.update(test_result)
            stats[test_result.code] += 1
            continue

        # Check if this file should be skipped w.r.t. the test_skiplist flag.
        skip_categories = [
            SkipCategory.SKIP_LIST,
            SkipCategory.MANUAL_SKIP_LIST,
        ]
        if lazy:
            skip_categories.append(SkipCategory.LAZY_SKIP_LIST)
        if test_result := skipped_paths_features.try_skip(
            test_file,
            skip_categories,
            full_test_name,
        ):
            if not test_skiplist:
                pd.update(test_result)
                stats[test_result.code] += 1
                continue
            else:
                skipped_tests[full_test_name] = suite.directory
        # We check for intl tests separately since we don't run them frequently.
        if test_result := skipped_paths_features.try_skip(
            test_file, [SkipCategory.INTL_TESTS], full_test_name
        ):
            if not test_intl:
                pd.update(test_result)
                stats[test_result.code] += 1
                continue
            else:
                skipped_tests[full_test_name] = suite.directory

        test_run_args = TestRunArgs(
            test_file,
            tests_home,
            work_dir,
            binary_directory,
            skipped_paths_features,
            test_skiplist,
            lazy,
            shermes,
            opt,
            extra_compile_vm_args,
            timeout,
        )
        tasks.append(suite.run_test(test_run_args))

    # Control maximum running tasks.
    async def wrap_task(fut: Awaitable) -> TestCaseResult:
        # Acquire the Semaphore and decrement it by one. If the counter is
        # already zero, wait until one running task is done, which will release
        # the Semaphore and increment the counter.
        async with current_n_tasks:
            return await fut

    failed_cases = defaultdict(list)
    # Similar to skipped_tests, but only store those passed.
    skipped_passed = {}
    for task in asyncio.as_completed([wrap_task(fut) for fut in tasks]):
        result = await task
        stats[result.code] += 1
        pd.update(result)
        if result.code.is_failure:
            failed_cases[result.code].append(result.test_name)
        # Some tests might be skipped due to unsupported features, we should consider
        # them as "passed" as well since they will be skipped anyway.
        if (
            result.code == TestResultCode.TEST_PASSED
            or result.code == TestResultCode.TEST_SKIPPED
        ):
            if test_suite_dir := skipped_tests.get(result.test_name):
                skipped_passed[result.test_name] = test_suite_dir

    pd.finish()
    elapsed = time.time() - start_time
    print(f"Testing time: {elapsed:.2f}")

    # Print result
    print_stats(stats)
    print_failed_tests(failed_cases)
    # The "intl_tests" list is simply a folder and we don't want to unfold
    # it. All intl tests that we can't pass are included in "skip_list". So
    # we will only remove them if both "--test-skiplist" and "--test-intl"
    # are provided.
    if len(skipped_passed) > 0 and test_skiplist:
        print_skipped_passed_tests(skipped_passed)
        remove_tests_from_skiplist(skipped_passed, skipped_paths_features)

    return 1 if len(failed_cases) > 0 else 0


def print_preprocessed_source(path: PathT) -> None:
    """
    Print the preprocessed source to STDOUT. For test262/mjsunit, it may include
    harness code and macros are replaced by actual code. For CVEs/flow/esprima,
    the exact source code is returned.
    """

    from .preprocess import generate_source

    suite = get_suite(path)
    if not suite:
        print("Not a test from valid test suite.")
        return
    with open(path, "rb") as f:
        source = f.read().decode("utf-8")
    test_case = generate_source(source, suite.directory, suite.get_full_test_name(path))
    # This is specifically for test262, and we only insert the directive if
    # it's in strict only mode (when testing we would have to test both strict
    # and non-strict mode if none of them is specified).
    if StrictMode.STRICT in test_case.strict_mode:
        print("'use strict';")
    print(test_case.source)


async def main() -> int:
    parser = create_parser()
    args = parser.parse_args()
    validate_args(args)

    # If user wants to dump the preprocessed source for debugging, handle it
    # separately for readability.
    if args.dump_source:
        print_preprocessed_source(args.paths[0])
        return 0

    work_dir: Optional[str] = args.work_dir
    # Hold the reference to the temporary directory (if we do create one), so
    # it won't be deleted until we run all tests.
    tmp_work_dir = None
    if work_dir is None:
        tmp_work_dir = tempfile.TemporaryDirectory(suffix="hermes_workdir")
        work_dir = tmp_work_dir.name
    else:
        # Remove the directory if it exists.
        if os.path.exists(work_dir):
            shutil.rmtree(work_dir)
        os.makedirs(work_dir, exist_ok=False)

    with importlib.resources.as_file(
        RESOURCE_ROOT / "skiplist.json"
    ) as skip_list_cfg_path:
        skipped_paths_features = SkippedPathsOrFeatures(os.fspath(skip_list_cfg_path))

    # Consumes all extra flags.
    extra_compile_vm_args = ExtraCompileVMArgs(
        compile_args=args.extra_compile_args, vm_args=args.extra_vm_args
    )

    exit_code = await run(
        args.paths,
        args.binary_directory,
        skipped_paths_features,
        work_dir,
        args.n_jobs,
        args.test_skiplist,
        args.test_intl,
        args.lazy,
        args.shermes,
        args.opt,
        extra_compile_vm_args,
        args.timeout,
        args.verbose,
    )

    # Explicitly clean up the temporary directory if we created one.
    if tmp_work_dir:
        tmp_work_dir.cleanup()

    return exit_code


if __name__ == "__main__":
    sys.exit(asyncio.run(main()))
