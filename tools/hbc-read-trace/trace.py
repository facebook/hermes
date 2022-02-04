#!/usr/bin/env python
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

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
        offset, names = sections.labels(access.page)
        yield ("/".join(names), offset, access.time)


def grouped_section_access_info(sections):
    """Consecutive accesses with the same label are grouped together.

    This function produces a list of tuples, each containing a string, and
    a list of access times.
    """
    groups = []
    info = section_access_info(sections)
    for label, group in groupby(info, key=lambda a: a[0]):
        groups.append((label, [(offset, time) for _, offset, time in group]))
    return groups


MAX_LABEL_WIDTH = 25


def display_brief(grouped_accesses, display_offsets, display_time):
    FMT = "{} -- \u03a3 = {}us"

    def print_fmt(label, total):
        print(FMT.format(label.ljust(MAX_LABEL_WIDTH), total).encode("utf-8"))

    for label, accesses in grouped_accesses:
        count = len(accesses)
        total = sum(time for _, time in accesses)

        if count > 1:
            label += " x{}".format(count)

        if display_offsets:
            label += " @ " + ", ".join(str(offset) for offset, _ in accesses)

        if not display_time:
            print(label)
            continue

        if len(label) <= MAX_LABEL_WIDTH:
            print_fmt(label, total)
        else:
            print(label)
            print_fmt("", total)


def display_all(grouped_accesses, display_offsets, display_time):
    FMT = "{} -- {}us"
    for group_label, accesses in grouped_accesses:
        for offset, time in accesses:
            if display_offsets:
                label = "{} @ {}".format(group_label, offset)
            else:
                label = group_label

            if not display_time:
                print(label)
                continue

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
        "-o",
        "--offsets",
        action="store_true",
        dest="display_offsets",
        help=(
            "Output the offsets at which each access occurred.  Offsets are "
            "given in increments of pages from the beginning of the section "
            "being read.  If there are multiple sections in a page, the first "
            "section is taken to be the one being read for the purposes of "
            "this calculation."
        ),
    )

    parser.add_argument(
        "-T",
        "--no-time",
        action="store_false",
        dest="display_time",
        help="Suppress output of the time taken.",
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
    display = display_all if args.verbose else display_brief

    display(accesses, args.display_offsets, args.display_time)
