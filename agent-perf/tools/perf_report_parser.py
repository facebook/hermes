#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

"""Helpers for parsing perf report --stdio symbol rows."""

import re
from typing import Dict, Optional


_PERCENT_RE = re.compile(r"^\d+(?:\.\d+)?%$")
_SYMBOL_TYPE_RE = re.compile(r"\[[^\]]\]\s+")


def parse_perf_report_line(line: str) -> Optional[Dict]:
    """Parse one perf report --stdio symbol row.

    Supports the compact formats used by agent-perf tools:
        12.34%  1234  hermes  libhermes.so  [.] someFunction
        12.34%        hermes  libhermes.so  [.] someFunction

    It also supports perf's default Children/Self layout:
        12.34%  5.67%  hermes  libhermes.so  [.] someFunction

    Returns None for comments, empty lines, headers, and unrecognized rows.
    """
    line = line.rstrip("\n")
    if not line.strip() or line.lstrip().startswith("#"):
        return None

    symbol_match = _SYMBOL_TYPE_RE.search(line)
    if not symbol_match:
        return None

    function = line[symbol_match.end() :].strip()
    if not function:
        return None

    prefix = line[: symbol_match.start()].strip()
    tokens = prefix.split()
    if not tokens:
        return None

    idx = 0
    percentages = []
    while idx < len(tokens) and _PERCENT_RE.match(tokens[idx]):
        percentages.append(float(tokens[idx][:-1]))
        idx += 1

    if not percentages:
        return None

    samples = 0
    if idx < len(tokens) and tokens[idx].isdigit():
        samples = int(tokens[idx])
        idx += 1

    if idx >= len(tokens):
        return None

    idx += 1  # command
    if idx >= len(tokens):
        return None

    return {
        "function": function,
        "module": " ".join(tokens[idx:]),
        "samples": samples,
        "percentage": percentages[-1],
    }
