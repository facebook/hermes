#!/usr/bin/env python
# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the LICENSE
# file in the root directory of this source tree.

from __future__ import absolute_import, division, print_function, unicode_literals

import argparse
from collections import namedtuple
from itertools import groupby
from sys import stdin

from hbc_sections import HBCSections


Access = namedtuple("Access", "page time")


def page_accesses():
    it = iter(stdin)
    next(it)  # Discard the header
    for line in it:
        page, time = line.split("\t")
        yield Access(int(page), int(time))


def section_access_info(sections):
    for access in page_accesses():
        labels = "/".join(sections.labels(access.page))
        yield (labels, access.time)


def grouped_section_access_info(sections):
    """Consecutive accesses with the same label are grouped together.

    This function produces a list of tuples, each containing a string, and
    a list of access times.
    """
    groups = []
    info = section_access_info(sections)
    for label, group in groupby(info, key=lambda a: a[0]):
        groups.append((label, [time for _, time in group]))
    return groups


MAX_LABEL_WIDTH = 25


def display_brief(grouped_accesses):
    FMT = "{} -- \u03a3 = {}us"
    for label, times in grouped_accesses:
        count = len(times)
        total = sum(times)

        if count > 1:
            label += " x{}".format(count)

        if len(label) <= MAX_LABEL_WIDTH:
            print(FMT.format(label.ljust(MAX_LABEL_WIDTH), total))
        else:
            print(label)
            print(FMT.format("".ljust(MAX_LABEL_WIDTH), total))


def display_all(grouped_accesses):
    FMT = "{} -- {}us"
    for label, times in grouped_accesses:
        for time in times:
            if len(label) <= MAX_LABEL_WIDTH:
                print(FMT.format(label.ljust(MAX_LABEL_WIDTH), time))
            else:
                print(label)
                print(FMT.format("".ljust(MAX_LABEL_WIDTH), time))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description=(
            "Annotates page access traces with the parts of the bytecode file "
            "they correspond to.  Page access traces are accepted on standard "
            "input as a TSV table.  The table is expected to have a header row "
            "and two columns:  One for the page number and another for the "
            "page access time in microseconds."
        )
    )

    parser.add_argument(
        "-B",
        "--bytecode",
        required=True,
        help="The bytecode file the page access trace refers to.",
    )

    parser.add_argument(
        "-D",
        "--hbcdump",
        required=True,
        help=(
            "HBCDump binary for parsing the provided bytecode file.  Its "
            "version must match the bytecode's"
        ),
    )

    parser.add_argument(
        "-v",
        "--verbose",
        action="store_true",
        help="Verbose output.  Each access is emitted on its own line.",
    )

    args = parser.parse_args()
    sections = HBCSections.from_bytecode(args.hbcdump, args.bytecode)

    accesses = grouped_section_access_info(sections)

    if args.verbose:
        display_all(accesses)
    else:
        display_brief(accesses)
