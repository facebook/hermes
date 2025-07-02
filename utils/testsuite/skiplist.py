# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

import json
import os
import sys
from enum import Enum, unique
from pathlib import Path
from typing import cast, Dict, List, Optional, Pattern, Sequence, Tuple, Union

from .typing_defs import PathT, SkippedPathItem, SkippedPathsOrFeaturesDict
from .utils import list_all_matched_entries, TestCaseResult, TestResultCode


@unique
class SkipCategory(Enum):
    MANUAL_SKIP_LIST = "manual_skip_list"
    SKIP_LIST = "skip_list"
    LAZY_SKIP_LIST = "lazy_skip_list"
    PERMANENT_SKIP_LIST = "permanent_skip_list"
    HANDLESAN_SKIP_LIST = "handlesan_skip_list"
    UNSUPPORTED_FEATURES = "unsupported_features"
    PERMANENT_UNSUPPORTED_FEATURES = "permanent_unsupported_features"
    INTL_TESTS = "intl_tests"
    PLATFORM_SKIP_LIST = "platform_skip_list"


SKIPCAT_TO_RETCODE = {
    SkipCategory.MANUAL_SKIP_LIST: TestResultCode.TEST_SKIPPED,
    SkipCategory.SKIP_LIST: TestResultCode.TEST_SKIPPED,
    SkipCategory.LAZY_SKIP_LIST: TestResultCode.TEST_SKIPPED,
    SkipCategory.PERMANENT_SKIP_LIST: TestResultCode.TEST_PERMANENTLY_SKIPPED,
    SkipCategory.UNSUPPORTED_FEATURES: TestResultCode.TEST_SKIPPED,
    SkipCategory.PERMANENT_UNSUPPORTED_FEATURES: TestResultCode.TEST_PERMANENTLY_SKIPPED,
    SkipCategory.INTL_TESTS: TestResultCode.TEST_SKIPPED,
    SkipCategory.PERMANENT_SKIP_LIST: TestResultCode.TEST_SKIPPED,
    SkipCategory.PLATFORM_SKIP_LIST: TestResultCode.TEST_SKIPPED,
}
"""Mapping from a skip category to result code."""


class SkippedPathsOrFeatures(object):
    """
    Manage skip list and unsupported features for testsuite.
    """

    def __init__(self, config_path: PathT):
        with open(config_path, "rb") as f:
            self.paths_features: SkippedPathsOrFeaturesDict = json.load(f)
            self.config_path = config_path

    def get_skiplist_for_cat(self, skip_cat: SkipCategory) -> List[SkippedPathItem]:
        """
        Get the skip list for given category. For PLATFORM_SKIP_LIST, get the
        list for the host platform.
        """
        if skip_cat == SkipCategory.PLATFORM_SKIP_LIST:
            platform_skip_list: Dict[str, List[SkippedPathItem]] = (
                self.paths_features.get(skip_cat.value, {})
            )
            return platform_skip_list.get(sys.platform, [])
        return self.paths_features.get(skip_cat.value, [])

    def should_skip_cat(
        self, test_or_feature: Union[str, Pattern[str]], skip_cat: SkipCategory
    ) -> bool:
        """
        Check if we should skip this test/feature.

        Args:
            test_or_feature: either full path of test file, or the feature
              listed in its metadata.
        """

        def should_skip(test_or_feature: Union[str, Pattern[str]], value: str) -> bool:
            if isinstance(test_or_feature, str):
                # Either filename or directory is specified.
                if value in test_or_feature:
                    return True
            # Do regex matching.
            elif test_or_feature.search(value):
                return True
            return False

        values: List[SkippedPathItem] = self.get_skiplist_for_cat(skip_cat)
        for value in values:
            if isinstance(value, dict):
                for p in value.get("paths", []):
                    if should_skip(test_or_feature, p):
                        return True
            else:
                if should_skip(test_or_feature, value):
                    return True

        return False

    def try_skip(
        self,
        test_or_feature: Union[str, Pattern[str]],
        cats: List[SkipCategory],
        full_test_name: str,
    ) -> Optional[TestCaseResult]:
        """
        Try if test_or_feature can be skipped.

        Args:
            test_or_feature: either full path of a test file, or the feature
              listed in its metadata.
            cats: SkipCategory to try.
            rel_test_path: A relative path of the test file to create error
              message.

        Returns:
            TestCaseResult with mapped result code and message, if skipped,
            None otherwise.
        """

        for cat in cats:
            if self.should_skip_cat(test_or_feature, cat):
                msg = f"SKIP: skipped by {cat.name} ({test_or_feature})"
                return TestCaseResult(full_test_name, SKIPCAT_TO_RETCODE[cat], msg)
        return None

    def get_unfolded_path(
        self, p: str, tests: Dict[str, str]
    ) -> Optional[Tuple[PathT, str]]:
        """
        If path p is a directory, and any passed test is under it, we
        should unfold it. If a path is a partial path, and any passed test
        matches it, we should also unfold it.

        Returns:
        Absolute path of the directory or partial path and the suite
            directory, if we should unfold it.
        None, otherwise.
        """

        # Only directory or partial path should unfold.
        if p.endswith(".js"):
            return None
        # We need the suite path to get the relative path for every file
        # under or matches it.
        for test, suite_dir in tests.items():
            if test.startswith(p):
                return os.fspath(Path(suite_dir).parent.joinpath(p)), suite_dir
        return None

    def unfold_top_level(
        self, tests: Dict[str, str], paths: Sequence[SkippedPathItem]
    ) -> List[SkippedPathItem]:
        """
        Subsitute the top level directory/partial paths (e.g., a SkippedPathItem
        that is a str) with all files under/match them.
        """

        # Use list to keep the original order.
        unfolded = []
        # Each time we unfold a directory by one level, i.e., we replace the
        # directory with files and directories directly under it. Keep unfolding
        # until no directory path matches any passed test. This can save some
        # syscalls compared to listing all files/directories with os.walk().
        # For partial path, we replace it with all files match it at once.
        while True:
            for path in paths:
                # PathsWithComment is handled separately.
                if isinstance(path, dict):
                    unfolded.append(path)
                    continue
                assert isinstance(path, str)
                if dir_path_and_suite_path := self.get_unfolded_path(path, tests):
                    # Assignment expression does not support unpacking so do it here.
                    (dir_path, suite_path) = dir_path_and_suite_path
                    # Adding files/directories under or matches it.
                    unfolded.extend(
                        os.path.relpath(f, Path(suite_path).parent)
                        for f in list_all_matched_entries(dir_path)
                    )
                else:
                    # If this path does not need to unfold, add it back.
                    unfolded.append(path)
            # If the list size is the same, we have done all necessary
            # unfolding.
            if len(unfolded) == len(paths):
                break
            paths = unfolded
            unfolded = []

        return unfolded

    def unfold(
        self, tests: Dict[str, str], skipped_paths: List[SkippedPathItem]
    ) -> List[SkippedPathItem]:
        """
        Some paths in the skiplist are directories. If we want to exclude
        tests under those directories, we need to unfold them first. A few
        paths are neither directories nor files, they are partial paths that
        are prefix to other paths. We need to replace them with all matched
        paths as well.

        For example, if there is a test path "ES6/async" in skiplist, and a test
        "ES6/async/001.js" under that directory passed, we will replace that
        directory with all files (recursively) under it first, then exclude the
        passed test. Another example, there is a test path "ES6/head-let-"
        in the skiplist, and there are two files match it, "ES6/head-let-0.js"
        and "ES6/head-let-1.js", if the first one passed, we will replace the
        partial path with these two file paths, and exclude the first one
        since it's passed.
        """

        # Unfold nested list first.
        for path_item in skipped_paths:
            if isinstance(path_item, str):
                continue
            # Explicit casting to make type checker happy.
            path_item["paths"] = cast(
                List[str], self.unfold_top_level(tests, path_item["paths"])
            )
        # Unfold directory paths directly included in the list.
        return self.unfold_top_level(tests, skipped_paths)

    def remove_tests(self, tests: Dict[str, str]) -> None:
        """Remove given tests from the skip list config."""

        # Only paths included in SKIP_LIST can be removed for now.
        cat = SkipCategory.SKIP_LIST.value
        skipped_paths: List[SkippedPathItem] = self.paths_features.get(cat, [])
        skipped_paths = self.unfold(tests, skipped_paths)
        # Remove plain test paths that also exist in the given list.
        skipped_paths = [
            p for p in skipped_paths if isinstance(p, dict) or p not in tests
        ]
        result = []
        for path_item in skipped_paths:
            # For PathsWithComment, update its "paths" list.
            if isinstance(path_item, dict):
                path_item["paths"] = [p for p in path_item["paths"] if p not in tests]
                # Ignore this item if all its nested paths have been removed.
                if len(path_item["paths"]) > 0:
                    result.append(path_item)
            else:
                result.append(path_item)

        self.paths_features[cat] = result

    def persist(self) -> None:
        """
        Write out the current skiplist to the original path.
        """

        with open(self.config_path, "w") as writer:
            json.dump(self.paths_features, writer, indent=2)
