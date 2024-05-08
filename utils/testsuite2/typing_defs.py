#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

from typing import Optional, TypedDict


# PathT = Union[str, bytes, os.PathLike]
# Currently, paths passed between functions are mostly str, so use it
# to aovid unncessary conversions.
PathT = str


class Negative(TypedDict):
    phase: str
    type: str


OptNegative = Optional[Negative]
