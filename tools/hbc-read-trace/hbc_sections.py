# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

from __future__ import absolute_import, division, print_function, unicode_literals

import re
import subprocess
from bisect import bisect_right
from collections import namedtuple
from operator import attrgetter


_RE_SECT = re.compile(r"(?P<title>.+): \[(?P<start>\d+), (?P<end>\d+)\)")
Section = namedtuple("Section", "start end label")


class HBCSections:
    @classmethod
    def from_bytecode(cls, hbcdump, bytecode):
        return HBCSections(list(raw_sections(hbcdump, bytecode)))

    def __init__(self, sections):
        sections.sort(key=attrgetter("start"))

        last = 0
        gaps = []
        for s in sections:
            assert last <= s.start, "Unexpected overlap"
            if last < s.start:
                gap = Section(last, s.start, "<unknown>")
                gaps.append(gap)
            last = s.end

        sections += gaps
        sections.sort(key=attrgetter("start"))

        self.ends = [s.end for s in sections]
        self.sections = sections

    def labels(self, pageno, pagesz=4096):
        """The labels for sections that overlap with the given page.

        pageno -- The offset of the page (as a number of pages) from the start
            of the bytecode file.
        pagesz -- The size of an individual page in bytes.  Defaults to 4096.

        Returns a 2-tuple containing:

          - The offset into the first section the read occurred in, as a number
            of pages.
          - A list containing the sections that overlap the page.  If there are
            multiple, they will appear in starting address order, and in the
            case of a tie, in the order the sections were output by the hbcdump
            tool.
        """
        first = bisect_right(self.ends, pagesz * pageno)
        last = bisect_right(self.ends, pagesz * (pageno + 1) - 1)

        off = pageno - self.sections[first].start // pagesz

        assert self.sections and last < self.sections[-1].end
        return off, [s.label for s in self.sections[first : last + 1]]


def parse_section(str):
    """Extract a section from a string.

    If the string is in the format of a section as output by hbcdump, it is
    extracted and returned, otherwise None is returned.
    """
    m = _RE_SECT.match(str)
    if not m:
        return

    title, start, end = m.group("title", "start", "end")
    return Section(int(start), int(end), title_to_label(title))


def raw_sections(hbcdump, bytecode):
    """Sequence of sections identified in bytecode.

    hbcdump  -- Path to the hbcdump binary to use to read the bytecode file.
    bytecode -- Path to the bytecode file to extract sections from.

    Returns the sequence of sections in the order they are output by hbcdump.
    """
    ranges = subprocess.check_output([hbcdump, "-show-section-ranges", bytecode])

    for line in ranges.decode().split("\n"):
        sect = parse_section(line)
        if sect:
            yield sect


_RE_WS = re.compile(r"\s+")


def title_to_label(title):
    """Convert an arbitrary bytecode section title to a label.

    Lowercases it, removes leading and trailing whitespace, and replaces
    connecting whitespace with a single -.  The section name "CommonJS module
    table" becomes "commonjs-module-table".
    """
    return _RE_WS.sub("-", title.strip().lower())
