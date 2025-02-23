# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

from __future__ import annotations

from typing import Dict, List, Optional, TypedDict, Union


# PathT = Union[str, bytes, os.PathLike]
# Currently, paths passed between functions are mostly str, so use it
# to aovid unncessary conversions.
PathT = str


class ExpectedFailure(TypedDict):
    phase: str
    type: str


OptExpectedFailure = Optional[ExpectedFailure]


class PathsWithComment(TypedDict):
    """
    A list of paths (to be skipped), with a comment explaining why we skip them.
    """

    # TODO(zhaogang): we can make this recursive to have nested comments on
    # paths.
    paths: List[str]
    comment: str


SkippedPathItem = Union[PathsWithComment, str]


class SkippedPathsOrFeaturesDict(TypedDict):
    skip_list: List[SkippedPathItem]
    lazy_skip_list: List[SkippedPathItem]
    permant_skip_list: List[SkippedPathItem]
    handlesan_skip_list: List[SkippedPathItem]
    unsupported_features: List[str]
    permant_unsupported_features: List[str]
    intl_tests: List[SkippedPathItem]
    platform_skip_list: Dict[str, List[SkippedPathItem]]


# Use | with Python 3.10+
# This is a recursive type, requiring support of type checker.
JSON = Union[int, float, str, bool, List["JSON"], Dict[str, "JSON"], None]
