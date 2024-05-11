#!/usr/bin/env python3

# Copyright 2011 by Google, Inc.  All rights reserved.
# This code is governed by the BSD license found in the LICENSE file.

# @lint-ignore-every LICENSELINT

# Format of test262:
# https://chromium.googlesource.com/external/github.com/tc39/test262/+/HEAD/CONTRIBUTING.md

import re
from typing import Optional

from ._monkeyYaml import load as yamlLoad


# Matches trailing whitespace and any following blank lines.
_BLANK_LINES = r"([ \t]*[\r\n]{1,2})*"

# Matches the YAML frontmatter block.
_YAML_PATTERN = re.compile(r"/\*---(.*)---\*/" + _BLANK_LINES, re.DOTALL)

# Matches all known variants for the license block.
# https://github.com/tc39/test262/blob/705d78299cf786c84fa4df473eff98374de7135a/tools/lint/lib/checks/license.py
_LICENSE_PATTERN = re.compile(
    r"// Copyright( \([C]\))? (\w+) .+\. {1,2}All rights reserved\.[\r\n]{1,2}"
    + r"("
    + r"// This code is governed by the( BSD)? license found in the LICENSE file\."
    + r"|"
    + r"// See LICENSE for details."
    + r"|"
    + r"// Use of this source code is governed by a BSD-style license that can be[\r\n]{1,2}"
    + r"// found in the LICENSE file\."
    + r"|"
    + r"// See LICENSE or https://github\.com/tc39/test262/blob/(master|HEAD)/LICENSE"
    + r")"
    + _BLANK_LINES,
    re.IGNORECASE,
)


def yamlAttrParser(testRecord: dict, attrs: str, test_name: str) -> None:
    parsed = yamlLoad(attrs)
    if parsed is None:
        print(f"Failed to parse yaml in name {test_name}")
        return

    for key in parsed:
        value = parsed[key]
        if key == "info":
            key = "commentary"
        testRecord[key] = value


def findLicense(src: str) -> Optional[str]:
    match = _LICENSE_PATTERN.search(src)
    if not match:
        return None

    return match.group(0)


def findAttrs(src: str) -> tuple[Optional[str], Optional[str]]:
    match = _YAML_PATTERN.search(src)
    if not match:
        return (None, None)

    return (match.group(0), match.group(1).strip())


def print_verbose(s: str, verbose: bool) -> None:
    if verbose:
        print(s)


def parseTestRecord(src: str, test_name: str, verbose: bool = True) -> dict:
    # Find the license block.
    header = findLicense(src)

    # Find the YAML frontmatter.
    (frontmatter, attrs) = findAttrs(src)

    # YAML frontmatter is required for all tests.
    if frontmatter is None:
        print_verbose(f"Missing frontmatter: {test_name}", verbose)

    # The license shuold be placed before the frontmatter and there shouldn't be
    # any extra content between the license and the frontmatter.
    if header is not None and frontmatter is not None:
        headerIdx = src.index(header)
        frontmatterIdx = src.index(frontmatter)
        if headerIdx > frontmatterIdx:
            print_verbose(f"Unexpected license after frontmatter: {test_name}", verbose)

        # Search for any extra test content, but ignore whitespace only or comment lines.
        extra = src[headerIdx + len(header) : frontmatterIdx]
        if extra and any(
            line.strip() and not line.lstrip().startswith("//")
            for line in extra.split("\n")
        ):
            print_verbose(
                f"Unexpected test content between license and frontmatter: {test_name}",
                verbose,
            )

    # Remove the license and YAML parts from the actual test content.
    test = src
    if frontmatter is not None:
        test = test.replace(frontmatter, "")
    if header is not None:
        test = test.replace(header, "")

    testRecord = {}
    testRecord["src"] = test

    if attrs:
        yamlAttrParser(testRecord, attrs, test_name)

    # Report if the license block is missing in non-generated tests.
    if (
        header is None
        and "generated" not in testRecord.get("flags", "")
        and "hashbang" not in test_name
    ):
        print_verbose(f"No license found in: {test_name}", verbose)

    return testRecord
