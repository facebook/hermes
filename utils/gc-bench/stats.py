#!/usr/bin/env python2.7
# Copyright (c) 2019-present, Facebook, Inc.
#
# This source code is licensed under the MIT license found in the LICENSE
# file in the root directory of this source tree.

from __future__ import absolute_import, division, print_function, unicode_literals

import json
import logging
import re
from collections import Counter
from math import sqrt


logger = logging.getLogger(__name__)


def parseStats(label, log):
    """Finds the label in the log output, and parses a JSON object immediately
    following it (if such an object exists).  Returns a python object
    representing the JSON upon success and None otherwise.
    """
    sigil = "{} stats:\n".format(label).encode()
    ix = log.find(sigil)

    if ix == -1:
        # Sigil was not found.
        return None

    post_sigil_log = log[ix + len(sigil) :].strip().decode("utf-8")
    try:
        decoder = json.JSONDecoder()
        return decoder.raw_decode(post_sigil_log)[0]
    except ValueError as e:
        # This could be more specific and catch JSONDecodeError, but that is
        # not defined until python3.5.
        logger.warn("Log could not be parsed. Its contents are printed below.")
        logger.warn(log.decode("utf-8"), e)
        return None


def stdDevPercentage(data):
    """Calculate standard deviation of a list and return it as percentage of the
    mean"""
    mean, sd = meanAndStdDev(data)
    if mean == 0:
        return 0
    return (sd / mean) * 100.0


def meanAndStdDev(data):
    """Calculate and return mean and standard deviation of a list"""
    if len(data) == 0:
        return (0, 0)
    length = float(len(data))
    mean = sum(data) / length
    return (mean, sqrt(sum((v - mean) ** 2 for v in data) / length))


def stdDev(data):
    """Calculate standard deviation of a list"""
    return meanAndStdDev(data)[1]


class HermesGCStatsCollector:
    def __init__(self, runner):
        self.runner = runner

    def run(self, *args):
        processStatsSum = Counter({})
        generalStatsSum = Counter({})
        collSpecificStatsSum = Counter({})
        runtimeGCStatsSum = Counter({})
        heapInfoStatsSum = Counter({})
        processStatsArray = []
        generalStatsArray = []
        skipBenchmark = False

        # Forward the arguments to the runner.
        for output in self.runner.run(*args):
            gcStats = parseStats("GC", output)
            if not gcStats:
                skipBenchmark = True
                break

            generalStats = gcStats["general"]
            if not generalStats:
                raise Exception("GC stats must include general stats")

            collectorSpecificTuple = gcStats["specific"]
            collectorSpecificStats = collectorSpecificTuple["stats"]

            runtimeGCStats = gcStats["runtime"]
            heapInfoStats = gcStats["heapInfo"]

            generalStatsArray.append(generalStats)
            generalStatsSum.update(generalStats)
            collSpecificStatsSum.update(collectorSpecificStats)
            runtimeGCStatsSum.update(runtimeGCStats)
            heapInfoStatsSum.update(heapInfoStats)

            # Process stats can be missing
            processStats = parseStats("Process", output)
            if processStats:
                processStatsArray.append(processStats)
                processStatsSum.update(processStats)

        if skipBenchmark:
            return ()

        numSamples = len(generalStatsArray)
        numProcessSamples = len(processStatsArray)
        procStatsAvg = {k: v / numProcessSamples for k, v in processStatsSum.items()}
        genStatsAvg = {k: v / numSamples for k, v in generalStatsSum.items()}
        collSpecificStatsAvg = {
            k: v / numSamples for k, v in collSpecificStatsSum.items()
        }
        runtimeGCStatsAvg = {k: v / numSamples for k, v in runtimeGCStatsSum.items()}
        heapInfoStatsAvg = {k: v / numSamples for k, v in heapInfoStatsSum.items()}

        # Getting percentage of the difference from average, except for gcNum,
        # which is just the max difference from average
        procBounds = {
            "rss": 2
            * stdDevPercentage(
                [res["Integral of RSS kBms"] for res in processStatsArray]
            ),
            "va": 2
            * stdDevPercentage(
                [res["Integral of VA kBms"] for res in processStatsArray]
            ),
        }

        genBounds = {
            "total": 2
            * stdDevPercentage([res["totalTime"] for res in generalStatsArray]),
            "gcNum": 2 * stdDev([res["numCollections"] for res in generalStatsArray]),
            "mut": 2
            * stdDevPercentage(
                [res["totalTime"] - res["totalGCTime"] for res in generalStatsArray]
            ),
            "gc": 2
            * stdDevPercentage([res["totalGCTime"] for res in generalStatsArray]),
            "gcCPU": 2
            * stdDevPercentage([res["totalGCCPUTime"] for res in generalStatsArray]),
        }

        return (
            (procStatsAvg, procBounds),
            (genStatsAvg, genBounds),
            collSpecificStatsAvg,
            runtimeGCStatsAvg,
            heapInfoStatsAvg,
        )
