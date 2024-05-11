# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

from __future__ import annotations

import os
import sys
from dataclasses import dataclass
from enum import auto, Enum, unique
from typing import List

from typing_defs import PathT


@unique
class TestResultCode(Enum):
    TEST_PASSED = auto()
    TEST_FAILED = auto()
    """Only use TEST_FAILED for unclassified failure, e.g., JSON error."""
    TEST_SKIPPED = auto()
    TEST_PERMANENTLY_SKIPPED = auto()
    COMPILE_FAILED = auto()
    COMPILE_TIMEOUT = auto()
    EXECUTE_FAILED = auto()
    EXECUTE_TIMEOUT = auto()

    @property
    def is_failure(self) -> bool:
        """Return whether this code indicates a test failure."""
        return (
            self != TestResultCode.TEST_PASSED
            and self != TestResultCode.TEST_SKIPPED
            and self != TestResultCode.TEST_PERMANENTLY_SKIPPED
        )


@dataclass
class TestCaseResult(object):
    # Test name
    test_name: str
    code: TestResultCode
    # A short message
    msg: str = ""
    # Output details of this test run
    output: str = ""


# Colors for stdout.
@unique
class Color(Enum):
    RESET = auto()
    RED = auto()
    GREEN = auto()

    def __str__(self):
        if not sys.stdout.isatty():
            return ""
        return {
            Color.RESET.value: "\033[0m",
            Color.RED.value: "\033[31m",
            Color.GREEN.value: "\033[32m",
        }[self.value]


def list_all_files(paths: List[PathT]) -> List[PathT]:
    """
    Recursively list all files in the given paths.
    """

    # Ignore non-JS files and fixture files in test262.
    def is_test(f: PathT):
        isFixture = "test262" in f and f.endswith("_FIXTURE.js")
        return f.endswith(".js") and not isFixture

    files = []
    for p in paths:
        if os.path.isdir(p):
            for root, _, filenames in os.walk(p):
                for filename in filenames:
                    full_filename = os.path.join(root, filename)
                    if is_test(full_filename):
                        files.append(full_filename)
        elif os.path.isfile(p):
            if is_test(p):
                files.append(p)
        else:
            print(f"Invalid path: {p}")
            sys.exit(1)

    return files


def check_hermes_exe(binary_path: PathT) -> None:
    """Check that hermes executable exists, terminate the execution if not."""
    hermes_exe = os.path.join(binary_path, "hermes")
    if not os.path.isfile(hermes_exe):
        print(f"{hermes_exe} not found.")
        sys.exit(1)
