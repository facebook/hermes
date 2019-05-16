#!/usr/bin/env python
# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the LICENSE
# file in the root directory of this source tree.

from __future__ import absolute_import, division, print_function, unicode_literals

import argparse
from collections import namedtuple
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

    args = parser.parse_args()
    sections = HBCSections.from_bytecode(args.hbcdump, args.bytecode)

    accesses = list(section_access_info(sections))
    label_lens = (len(a[0]) for a in accesses)
    pad = max(l for l in label_lens if l < 25)

    for label, time in accesses:
        if len(label) <= pad:
            print("{} -- {}us".format(label.ljust(pad), time))
        else:
            print(label)
            print("{} -- {}us".format("".ljust(pad), time))
