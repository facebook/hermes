#!/usr/bin/python
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# -*- coding: utf-8 -*-

""" Executable section size analysis.

This script analyzes the size of an executable by section.
The 'measure' command may be given to print size metrics about a single executable,
as JSON.
Alternatively, the 'compare' command may be given to print size differences two files,
which may be either an executable, or JSON output.
"""

from __future__ import absolute_import, division, print_function, unicode_literals

import argparse
import json
import math
import os
import re
import subprocess
import sys
from collections import OrderedDict


JSON_INDENT = 2
METRIC_KEYS = ["filesize", "rw_data", "ro_data", "code", "bss", "symbols", "unwind"]


def metric_key_for_section_name(secname):
    """Returns an attribute name for the given section name, or None
    Note that not all sections are represented.
    """
    if secname in ["text", "init_array", "fini_array"]:
        return "code"
    elif secname in ["const", "cstring", "ustring"]:
        return "ro_data"
    elif secname in ["data", "thread_vars", "data.rel.ro"]:
        return "rw_data"
    elif secname in ["bss", "thread_bss", "tbss", "dynamic"]:
        return "bss"
    elif secname in [
        "stub_helper",
        "stubs",
        "nl_symbol_ptr",
        "la_symbol_ptr",
        "gnu.hash",
        "dynsym",
        "dynstr",
        "rela.dyn",
        "rela.plt",
        "got",
        "got.plt",
    ]:
        return "symbols"
    elif secname in ["eh_frame_hdr", "eh_frame", "unwind_info"]:
        return "unwind"
    else:
        return None


def measure_executable(path):
    """Invoke `size` and parse its output, returning a dictionary.

    On Darwin, the interesting lines look like:
      .rodata               366252   5048448
    On Linux:
      __const              25808   4295726400
    """
    metrics = {key: 0 for key in METRIC_KEYS}
    reg = re.compile(
        r"""[._]+         # Leading period or underscore
                         ([a-z_]+)\s+  # Section name
                         (\d+)\s+      # Section size
                         (\d+)\s*      # Section address
                     """,
        re.X,
    )
    text = subprocess.check_output(["size", "-A", "-d", path])
    for line in text.split("\n"):
        match = reg.match(line)
        if not match:
            continue
        secname, secsize = match.group(1, 2)
        key = metric_key_for_section_name(secname)
        if key:
            metrics[key] += int(secsize)
    # Add __LINKEDIT on Darwin, available through size -m
    if sys.platform == "darwin":
        m_output = subprocess.check_output(["size", "-m", "-d", path])
        linkedit_match = re.search(r"__LINKEDIT: (\d+)", m_output)
        if linkedit_match:
            metrics["symbols"] += int(linkedit_match.group(1))

    # Humanize and order the resulting metrics
    result = OrderedDict()
    for (key, size) in metrics.items():
        result[key] = human_readable_size(size)
    result["name"] = os.path.basename(path)
    result["sha1"] = subprocess.check_output(["openssl", "sha1", path]).split()[1]
    result["filesize"] = human_readable_size(os.path.getsize(path))
    return result


def human_readable_size(size, show_sign=False):
    """Return a human-readable string for a given byte size.
    If show_sign is set, write positive numbers with a leading + sign.
    """
    if size == 0:
        return "0"
    units = ["B", "KiB", "MiB", "GiB", "TiB", "PiB", "EiB", "ZiB", "YiB"]
    p = math.floor(math.log(abs(size), 2) / 10)
    maybe_plus = "+" if size > 0 and show_sign else ""
    amt = size / math.pow(1024, p)
    return "%d (%s%.1f %s)" % (size, maybe_plus, amt, units[int(p)])


def compare_metrics(before_metrics, after_metrics):
    """Given before and after metrics, return a dictionary describing
    their differences."""
    result = OrderedDict()
    for key in METRIC_KEYS:
        # Extract the leading byte count from the humanized string
        before = int(before_metrics[key].split()[0])
        after = int(after_metrics[key].split()[0])
        result[key] = human_readable_size(after - before, show_sign=True)
    return result


def read_or_measure_metrics(path):
    """Given a path (which may be to a JSON file or an executable),
    return the metrics.
    """
    try:
        with open(path) as path_fd:
            return json.load(path_fd)
    except ValueError:
        return measure_executable(path)


def subcommand_compare(args):
    """Implementation of compare subcommand
    Compares two files (JSON or executables) specified in args.FILES
    """
    before_path, after_path = args.FILES
    before_metrics = read_or_measure_metrics(before_path)
    after_metrics = read_or_measure_metrics(after_path)
    diffs = compare_metrics(before_metrics, after_metrics)
    print(json.dumps(diffs, indent=JSON_INDENT))


def subcommand_measure(args):
    """Implementation of measure subcommand
    Collects metrics about the files specified in args.FILES
    """
    for argfile in args.FILES:
        metrics = measure_executable(argfile)
        print(json.dumps(metrics, indent=JSON_INDENT))


USAGE = r"""summarize_sizes.py measure FILE [FILE...]
       summarize_sizes.py compare BEFORE_FILE AFTER_FILE

  In its first form, output a JSON description of metrics of the given file.

  In its second form, output a JSON description of the differences of the two
  files (which may be JSON descriptions from previous runs, or executables).
"""

if __name__ == "__main__":
    parser = argparse.ArgumentParser(usage=USAGE)
    subparsers = parser.add_subparsers()
    measure_parser = subparsers.add_parser("measure")
    measure_parser.add_argument("FILES", nargs="+")
    measure_parser.set_defaults(func=subcommand_measure)
    compare_parser = subparsers.add_parser("compare")
    compare_parser.add_argument("FILES", nargs=2)
    compare_parser.set_defaults(func=subcommand_compare)
    pargs = parser.parse_args()
    pargs.func(pargs)
