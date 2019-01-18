#!/usr/bin/env python2.7
# Copyright (c) 2019-present, Facebook, Inc.
#
# This source code is licensed under the MIT license found in the LICENSE
# file in the root directory of this source tree.


from __future__ import print_function

import argparse
import logging
import os
import sys
from resource.lib import resourceResolver

import gc_bench.output as output
from gc_bench.categories import categories, synths
from gc_bench.runner import HermesRunner, SynthBenchmarkRunner
from gc_bench.stats import HermesGCStatsCollector
from gc_bench.tmpdir import TemporaryDirectory


logger = logging.getLogger(__name__)


def progress(*args):
    """Output progress updates, on STDERR to avoid contaminating the GC bench
    output
    """
    print(*args, file=sys.stderr)


HermesName = "hermes"
SynthName = "synth"


def determineRuntime(hermes, synth):
    """
    Choose which Runtime configuration to use, given the flags that were passed
    in. If no flags have been set (i.e. the user made no choice about the
    runtime), default to using the Non-contiguous generational GC.
    """

    runtimes = [(HermesName, hermes), (SynthName, synth)]

    choices = [rt for (rt, chosen) in runtimes if chosen]

    if len(choices) > 1:
        raise Exception("Cannot specify multiple runtimes")

    return next(iter(choices), HermesName)


FORMATTERS = {"ascii": output.asciiTable, "tsv": output.tabSeparated}


def main():
    # Support for command line arguments
    argparser = argparse.ArgumentParser(description="Runs gc benchmarks")
    argparser.add_argument(
        "-b",
        "--binary",
        help="Path to hermes or synth executable",
        required=True,
        action="store",
    )
    argparser.add_argument(
        "--compiler", help="Path to hermes for compiling js", action="store"
    )
    runtimeGroup = argparser.add_mutually_exclusive_group()
    runtimeGroup.add_argument("--hermes", help="Use the Hermes VM", action="store_true")
    runtimeGroup.add_argument(
        "--synth", help="Use the synthetic benchmark interpreter", action="store_true"
    )

    argparser.add_argument(
        "-c",
        "--count",
        type=int,
        default=1,
        help="Runs each test 'count' number of times",
    )

    argparser.add_argument(
        "--benchmarks",
        "--bm",
        dest="benchmarks",
        nargs="+",
        help="Choose one or more benchmarks to run.",
        choices={bm.name for cat in categories for bm in cat.benchmarks},
    )
    argparser.add_argument(
        "--categories",
        "--cats",
        dest="categories",
        nargs="+",
        help="Choose one or more benchmark categories to run.",
        choices={cat.name for cat in categories},
    )
    argparser.add_argument(
        "--output-format",
        "-f",
        dest="output_format",
        help="Format for the statistics output at the end of a GC Bench run.",
        choices=FORMATTERS,
        default="ascii",
    )
    argparser.add_argument(
        "--keep-tmp",
        "-k",
        dest="keep_tmp",
        help="Keep around temporary bytecode files after compilation",
        action="store_true",
    )
    argparser.add_argument(
        "--cpu-times",
        dest="cpu_times",
        help="Output a table containing the CPU time spent on GCs.",
        action="store_true",
    )

    logging_group = argparser.add_mutually_exclusive_group()
    logging_group.add_argument(
        "--verbose",
        "-v",
        help="Log extra messages about each individual operation",
        action="store_const",
        dest="loglevel",
        const=logging.INFO,
    )
    logging_group.add_argument(
        "--debug",
        "-d",
        help="Add debug level of logging for when something goes wrong",
        action="store_const",
        dest="loglevel",
        const=logging.DEBUG,
        default=logging.WARNING,
    )

    args = argparser.parse_args()
    logging.basicConfig(level=args.loglevel)

    runtime = determineRuntime(args.hermes, args.synth)

    formatter = FORMATTERS[args.output_format]

    binary = os.path.abspath(args.binary)

    if not os.path.isfile(binary):
        raise Exception(
            "Binary (at location {}) should be an existing normal file".format(binary)
        )

    if runtime == HermesName:
        # Hermes runtime, use the hermes runner and stat collector
        statCollector = HermesGCStatsCollector(
            HermesRunner(binary, args.count, args.keep_tmp)
        )
    elif runtime == SynthName:
        # Synthetic benchmarks, use the synth runner and hermes stat collector
        statCollector = HermesGCStatsCollector(SynthBenchmarkRunner(binary, args.count))
    else:
        assert False, "Illegal runtime specified"

    # Get the benchmarks to run
    benchmarkNamesToRun = set(args.benchmarks or [])
    if not args.categories and not benchmarkNamesToRun:
        # If there were no benchmarks specified, and no categories specified,
        # then use the default categories
        args.categories = {cat.name for cat in categories if cat.runByDefault}
    if args.categories:
        # Add the benchmarks specified by any categories
        benchmarkNamesToRun |= {
            bm.name
            for cat in categories
            if cat.name in args.categories
            for bm in cat.benchmarks
        }
    benchmarks = [
        bm
        for cat in categories
        for bm in cat.benchmarks
        if bm.name in benchmarkNamesToRun
    ]
    if runtime == SynthName:
        # Remove non-synthetic benchmarks from Synth, since it can't run normal
        # benchmarks
        benchmarks = [bm for bm in benchmarks if bm in synths.benchmarks]
    else:
        # Remove synthetic benchmarks from Hermes, since only Synth can run
        # them.
        benchmarks = [bm for bm in benchmarks if bm not in synths.benchmarks]

    logger.info("Running benchmarks {}".format(benchmarks))
    assert benchmarks, "No benchmarks to run"

    with TemporaryDirectory(args.keep_tmp) as resourceDir:

        def resolver(rsc):
            return resourceResolver(resourceDir, rsc)

        # Run the benchmarks and get the results.
        results = [
            # Comma to force a tuple instead of a string.
            (bm.name,) + bm.run(resolver, statCollector.run)
            for bm in benchmarks
        ]

    # Filter out benchmarks that were skipped.
    results = [r for r in results if len(r) >= 2]
    # Add the benchmark name to each individual result for easier output.
    processResults = [(file[0],) + file[1] for file in results]
    generalResults = [(file[0],) + file[2] for file in results]
    heapInfoResults = [(file[0], file[5]) for file in results]
    specificResults = [(file[0], file[3]) for file in results]
    runtimeGCResults = [(file[0], file[4]) for file in results]

    output.processResults(processResults, formatter)
    output.generalResults(generalResults, formatter)
    output.heapInfoResults(heapInfoResults, formatter)

    output.generationalResults(generalResults, specificResults, formatter)
    output.runtimeGCResults(runtimeGCResults, formatter)

    if args.cpu_times:
        output.cpuTimeResults(generalResults, specificResults, formatter)


if __name__ == "__main__":
    main()
