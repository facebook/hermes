#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

import argparse
import asyncio
import os
import shutil
import tempfile
import time
from asyncio import Semaphore
from collections import defaultdict
from typing import Awaitable, List, Optional

import utils
from progress import (
    ProgressBar,
    SimpleProgressBar,
    TerminalController,
    TestCaseResult,
    TestingProgressDisplay,
)
from skiplist import SkipCategory, SkippedPathsOrFeatures
from test_run_defs import get_suite, TestRunArgs
from typing_defs import PathT
from utils import Color, TestResultCode

N_DEFAULT_CPU = 10


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
        default="./bin/",
        help="Path to hermes built binary directory",
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
    parser.add_argument(
        "paths", type=str, nargs="+", help="Paths to testsuite, can be dir or file"
    )

    return parser


def print_stats(stats: dict):
    """Print the result stats."""
    total = sum(stats.values())
    failed = (
        stats[TestResultCode.COMPILE_FAILED]
        + stats[TestResultCode.COMPILE_TIMEOUT]
        + stats[TestResultCode.EXECUTE_FAILED]
        + stats[TestResultCode.EXECUTE_TIMEOUT]
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
    print("-----------------------------------")
    # fmt: on


def print_failed_tests(tests: dict):
    def print_test_list(cat: TestResultCode, header: str):
        if len(tests[cat]) > 0:
            print("-----------------------------------")
            print(header)
            for test in tests[cat]:
                print(test)
            print("-----------------------------------")

    print("\nDetails:")
    print_test_list(TestResultCode.COMPILE_FAILED, "Compile failed:")
    print_test_list(TestResultCode.COMPILE_TIMEOUT, "Compile timeout:")
    print_test_list(TestResultCode.EXECUTE_FAILED, "Execute failed:")
    print_test_list(TestResultCode.EXECUTE_TIMEOUT, "Execute timeout:")


async def run(
    tests_paths: List[PathT],
    binary_directory: PathT,
    skipped_paths_features: SkippedPathsOrFeatures,
    work_dir: PathT,
    n_jobs: int,
    test_skiplist: bool,
    test_intl: bool,
    verbose: bool,
) -> None:
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

    utils.check_hermes_exe(binary_directory)
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
    for test_file in tests_files:
        suite = get_suite(test_file)
        assert suite, f"Test suite root directory is not found for {test_file}"
        full_test_name = suite.get_full_test_name(test_file)

        # Check if this file should be skipped.
        if not test_skiplist:
            if test_result := skipped_paths_features.try_skip(
                test_file,
                [SkipCategory.SKIP_LIST, SkipCategory.PERMANENT_SKIP_LIST],
                full_test_name,
            ):
                pd.update(test_result)
                stats[test_result.code] += 1
                continue
        if not test_intl:
            if test_result := skipped_paths_features.try_skip(
                test_file, [SkipCategory.INTL_TESTS], full_test_name
            ):
                pd.update(test_result)
                stats[test_result.code] += 1
                continue

        test_run_args = TestRunArgs(
            test_file,
            tests_home,
            work_dir,
            binary_directory,
            skipped_paths_features,
            test_skiplist,
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
    for task in asyncio.as_completed([wrap_task(fut) for fut in tasks]):
        result = await task
        stats[result.code] += 1
        pd.update(result)
        if result.code.is_failure:
            failed_cases[result.code].append(result.test_name)
    pd.finish()
    elapsed = time.time() - start_time
    print(f"Testing time: {elapsed:.2f}")

    # Print result
    print_stats(stats)
    print_failed_tests(failed_cases)


async def main():
    parser = create_parser()
    args = parser.parse_args()

    work_dir: Optional[str] = args.work_dir
    # Hold the reference to the temporary directory (if we do create one), so
    # it won't be deleted until we run all tests.
    tmp_work_dir = None
    if work_dir is None:
        tmp_work_dir = tempfile.TemporaryDirectory()
        work_dir = tmp_work_dir.name
    else:
        # Remove the directory if it exists.
        if os.path.exists(work_dir):
            shutil.rmtree(work_dir)
        os.makedirs(work_dir, exist_ok=False)

    skip_list_cfg_path = os.path.join(os.path.dirname(__file__), "skiplist.json")
    skipped_paths_features = SkippedPathsOrFeatures(skip_list_cfg_path)

    await run(
        args.paths,
        args.binary_directory,
        skipped_paths_features,
        work_dir,
        args.n_jobs,
        args.test_skiplist,
        args.test_intl,
        args.verbose,
    )


if __name__ == "__main__":
    asyncio.run(main())
