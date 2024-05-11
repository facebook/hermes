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
from typing import Optional

import skiplist
import test262

N_DEFAULT_CPU = 10


def create_parser():
    parser = argparse.ArgumentParser(description="Run Hermes testsuite")
    subparsers = parser.add_subparsers(dest="subcommand")

    # Sub-parser for Javascript testsuite.
    test262_parser = subparsers.add_parser(
        "test262", help="Run Javascript testsuite with Hermes"
    )
    test262_parser.add_argument(
        "paths", type=str, nargs="+", help="Paths to testsuite, can be dir or file"
    )
    test262_parser.add_argument(
        "-b",
        "--binary-path",
        dest="binary_path",
        default="./bin/",
        help="Path to hermes built binary directory",
    )
    test262_parser.add_argument(
        "-j",
        "--jobs",
        dest="n_jobs",
        default=os.cpu_count() or N_DEFAULT_CPU,
        type=int,
        help="Number of tests to run simultaneously, default is number of logical CPUs",
    )
    test262_parser.add_argument(
        "--test-skiplist",
        dest="test_skiplist",
        action="store_true",
        help="Run tests in the skiplist and remove them from the list if passed",
    )
    test262_parser.add_argument(
        "--test-intl",
        dest="test_intl",
        action="store_true",
        help="Run supported Intl tests",
    )
    test262_parser.add_argument(
        "--work-dir",
        dest="work_dir",
        default=None,
        type=str,
        help="Specifies work directory where the test files will be generated. "
        "The work directory will be deleted if it is not empty",
    )
    test262_parser.add_argument(
        "-v",
        "--verbose",
        dest="verbose",
        default=False,
        action="store_true",
        help="Show intermediate output",
    )

    # Sub-parser for esprima.
    esprima_parser = subparsers.add_parser(
        "esprima", help="Run AST test fixtures with Hermes"
    )
    esprima_parser.add_argument(
        "path", type=str, help="path to the dir of test fixtures"
    )
    esprima_parser.add_argument(
        "--hermes", dest="hermes", help="path to the hermes binary"
    )

    return parser


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
    skipped_paths_features = skiplist.SkippedPathsOrFeatures(skip_list_cfg_path)

    if args.subcommand == "test262":
        await test262.run(
            args.paths,
            args.binary_path,
            skipped_paths_features,
            work_dir,
            args.n_jobs,
            args.test_skiplist,
            args.test_intl,
            args.verbose,
        )


if __name__ == "__main__":
    asyncio.run(main())
