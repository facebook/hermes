#!/usr/bin/env python2.7
# Copyright (c) 2019-present, Facebook, Inc.
#
# This source code is licensed under the MIT license found in the LICENSE
# file in the root directory of this source tree.

"""Process Statistics Coming from the Benchmark

These operations calculate derived metrics from those given by the benchmark.
"""

from __future__ import division


def _assertNotIn(d, *keys):
    for key in keys:
        assert key not in d, "Dictionary already contains '{}'" % key


def _pct(num, denom):
    """Calculate percentage

    Calculates the fraction `num/denom` as a percentage. If `denom` is `0`, this
    function will also return `0`.

    """
    return num * 100 / denom if denom else 0


def updateGeneralStats(stats):
    _assertNotIn(stats, "mutTime", "gcPct")

    totalTime = stats["totalTime"]
    gcTime = stats["totalGCTime"]
    mutTime = totalTime - gcTime
    gcPct = _pct(gcTime, totalTime)

    stats.update(mutTime=mutTime, gcPct=gcPct)


def updateJitStats(stats):
    _assertNotIn(stats, "jitGcPct")

    # Estimate the mutator and total time if we had a JIT that sped mutator
    # ops up by 10x
    gcTime = stats["totalGCTime"]
    jitMutTime = stats["mutTime"] / 10.0
    jitTotTime = jitMutTime + gcTime

    stats.update(jitGcPct=_pct(gcTime, jitTotTime))


def updateGenerationalStats(totalGCTime, stats):
    YG_PHASES = [
        "ygMarkOldToYoung",
        "ygMarkRoots",
        "ygScanTransitive",
        "ygUpdateWeakRefs",
        "ygFinalizers",
    ]
    FULL_PHASES = [
        "fullMarkRoots",
        "fullMarkTransitive",
        "fullSweep",
        "fullUpdateRefs",
        "fullCompact",
    ]

    def key(phase):
        return phase + "Time"

    def time(phase):
        return stats[key(phase)]

    _assertNotIn(stats, *(key(phase) + "Pct" for phase in YG_PHASES))
    _assertNotIn(stats, *(key(phase) + "Pct" for phase in FULL_PHASES))

    _assertNotIn(stats, "ygOtherGCTime", "fullOtherGCTime")
    _assertNotIn(stats, "ygGCTimePct", "fullGCTimePct")

    ygTotalGCTime = stats["ygTotalGCTime"]
    fullTotalGCTime = stats["fullTotalGCTime"]

    # Good form to make sure almost everything is accounted for...
    ygOtherGCTime = ygTotalGCTime - sum(time(phase) for phase in YG_PHASES)
    fullOtherGCTime = fullTotalGCTime - sum(time(phase) for phase in FULL_PHASES)

    stats.update(
        {key(phase) + "Pct": _pct(time(phase), ygTotalGCTime) for phase in YG_PHASES}
    )

    stats.update(
        {
            key(phase) + "Pct": _pct(time(phase), fullTotalGCTime)
            for phase in FULL_PHASES
        }
    )

    stats.update(
        ygOtherGCTime=ygOtherGCTime, ygOtherGCTimePct=_pct(ygOtherGCTime, ygTotalGCTime)
    )

    stats.update(
        fullOtherGCTime=fullOtherGCTime,
        fullOtherGCTimePct=_pct(fullOtherGCTime, fullTotalGCTime),
    )

    stats.update(
        ygGCTimePct=_pct(ygTotalGCTime, totalGCTime),
        fullGCTimePct=_pct(fullTotalGCTime, totalGCTime),
    )


def updateRuntimeStats(stats):
    def key(root):
        return "MarkRoots_" + root

    def time(root):
        return stats[key(root) + "Time"]

    INDIVIDUAL_ROOTS = [
        "Registers",
        "RuntimeModules",
        "CharStrings",
        "IdentifierTable",
        "GCScopes",
    ]

    # We predict that several of the phases will make trivial contributions.  If
    # that becomes false, we can expand them out into individual roots.
    OTHER_ROOTS = ["RuntimeInstanceVars", "Prototypes", "DebugEnvironment", "Custom"]

    _assertNotIn(stats, *(key(root) + "Pct" for root in INDIVIDUAL_ROOTS))
    _assertNotIn(stats, "MarkRoots_otherTime", "MarkRoots_otherTimePct")

    totalTime = stats["totalMarkRootsTime"]
    otherTime = sum(time(root) for root in OTHER_ROOTS)

    stats.update(
        {key(root) + "Pct": _pct(time(root), totalTime) for root in INDIVIDUAL_ROOTS}
    )

    stats.update(
        MarkRoots_otherTime=otherTime, MarkRoots_otherTimePct=_pct(otherTime, totalTime)
    )
