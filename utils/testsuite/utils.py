# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

from __future__ import annotations

import os
import re
import sys
from dataclasses import dataclass
from enum import auto, Enum, unique
from functools import cache
from pathlib import Path
from typing import List, Set

from .typing_defs import PathT


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


# Ignore non-JS files and fixture files in test262.
def is_test(f: PathT) -> bool:
    isFixture = "test262" in f and f.endswith("_FIXTURE.js")
    return f.endswith(".js") and not isFixture


def list_all_files(paths: List[PathT]) -> List[PathT]:
    """
    Recursively list all files in the given paths.
    """

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


def list_entries_non_recursive(dir_path: PathT) -> List[PathT]:
    """
    List all files and directories in the given path (non-recursively).
    """

    entries = []
    for entry in os.listdir(dir_path):
        path = os.path.join(dir_path, entry)
        if os.path.isdir(path):
            entries.append(path)
        elif os.path.isfile(path):
            if is_test(path):
                entries.append(path)
        else:
            print(f"Invalid path: {path}")
            sys.exit(1)

    return entries


def list_all_matched_entries(pattern: str) -> List[PathT]:
    """
    List all files/directories matching the given pattern, which could be a
    directory path, or a partial path that is prefix of other file paths.

    Unlike list_all_files(), when the pattern is a directory path, we only list
    the files/directories directly under it.
    """

    if os.path.isdir(pattern):
        return list_entries_non_recursive(pattern)

    # In skiplist config, we have partial path like
    # test262/test/language/statements/for-of/head-let. We first get its
    # parent directory, then list all files that are under it, and return those
    # match the partial path.
    parent = os.fspath(Path(pattern).parent)
    # In case the partial path matches directories, we recursively list all
    # files.
    files = list_all_files([parent])
    return [f for f in files if f.startswith(pattern)]


def check_hermes_exe(binary_dir: PathT, shermes: bool) -> None:
    """Check that needed Hermes executables exist, terminate the execution if not."""

    # TODO(zhaogang): Check the executable names on Windows if we run testsuites on it.
    exe = os.path.join(binary_dir, "hermes")
    if shermes:
        exe = os.path.join(binary_dir, "shermes")

    if not os.path.isfile(exe):
        print(f"Error: {exe} not found.")
        sys.exit(1)


HERMES_FEATURES_MATCHER = re.compile(
    r"\s*Features:\s*\n(.*\n)", re.MULTILINE | re.DOTALL
)
"""Match the "Features" output of `hermes --version`."""


CONFIGURABLE_HERMES_FEATURES = {
    "Unicode RegExp Property Escapes": "regexp-unicode-property-escapes",
}
"""Mapping of Hermes features to test262 features, for use in dynamic feature
skipping."""


@cache
def get_hermes_features(binary_dir: PathT) -> List[str]:
    """
    Run `hermes --version` and extract the features from the output.
    """

    import subprocess

    exe = os.path.join(binary_dir, "hermes")
    try:
        output = subprocess.check_output([exe, "--version"], stderr=subprocess.STDOUT)
        features = HERMES_FEATURES_MATCHER.search(output.decode("utf-8"))
        if features is not None:
            return [feature.strip() for feature in features.group(1).splitlines()]
        else:
            return []
    except subprocess.CalledProcessError as e:
        print(
            "Failed to run hermes --version: " + e.output.decode("utf-8"),
            file=sys.stderr,
        )
        return []


@cache
def get_hermes_supported_test262_features(binary_dir: PathT) -> List[str]:
    """
    Get hermes supported test262 feature from the feature list of the built
    hermes binary.
    """
    hermes_features = get_hermes_features(binary_dir)
    return [
        CONFIGURABLE_HERMES_FEATURES[hermes_feature]
        for hermes_feature in hermes_features
        if hermes_feature in CONFIGURABLE_HERMES_FEATURES
    ]
