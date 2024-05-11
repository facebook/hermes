# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

import json
from enum import Enum, unique
from typing import List, Optional, Pattern, Union

from progress import TestCaseResult

from typing_defs import PathT, SkippedPathItem, SkippedPathsOrFeaturesDict
from utils import TestResultCode


@unique
class SkipCategory(Enum):
    SKIP_LIST = "skip_list"
    LAZY_SKIP_LIST = "lazy_skip_list"
    PERMANENT_SKIP_LIST = "permanent_skip_list"
    HANDLESAN_SKIP_LIST = "handlesan_skip_list"
    UNSUPPORTED_FEATURES = "unsupported_features"
    PERMANENT_UNSUPPORTED_FEATURES = "permanent_unsupported_features"
    INTL_TESTS = "intl_tests"


SKIPCAT_TO_RETCODE = {
    SkipCategory.SKIP_LIST: TestResultCode.TEST_SKIPPED,
    SkipCategory.LAZY_SKIP_LIST: TestResultCode.TEST_SKIPPED,
    SkipCategory.PERMANENT_SKIP_LIST: TestResultCode.TEST_PERMANENTLY_SKIPPED,
    SkipCategory.UNSUPPORTED_FEATURES: TestResultCode.TEST_SKIPPED,
    SkipCategory.PERMANENT_UNSUPPORTED_FEATURES: TestResultCode.TEST_PERMANENTLY_SKIPPED,
    SkipCategory.INTL_TESTS: TestResultCode.TEST_SKIPPED,
}
"""Mapping from a skip category to result code."""


class SkippedPathsOrFeatures(object):
    """
    Manage skip list and unsupported features for testsuite.
    """

    def __init__(self, cfg_path: PathT):
        with open(cfg_path, "rb") as f:
            self.paths_features: SkippedPathsOrFeaturesDict = json.load(f)

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

        values: List[SkippedPathItem] = self.paths_features.get(skip_cat.value, [])
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
                msg = f"SKIP: {full_test_name} is skipped by {cat.name}"
                return TestCaseResult(full_test_name, SKIPCAT_TO_RETCODE[cat], msg)
        return None
