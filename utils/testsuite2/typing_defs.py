#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

from typing import List, Optional, TypedDict, Union


# PathT = Union[str, bytes, os.PathLike]
# Currently, paths passed between functions are mostly str, so use it
# to aovid unncessary conversions.
PathT = str


class Negative(TypedDict):
    phase: str
    type: str


OptNegative = Optional[Negative]


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
