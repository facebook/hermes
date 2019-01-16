#!/usr/bin/env python2.7

from __future__ import print_function

import gc_bench.derived_metrics as derived_metrics


# Formats for time and space units we use.
_size_fmt = "{:,.1f}"
_percentage_fmt = "{:4.1f}%"
_seconds_bounds_fmt = "{} +- " + _percentage_fmt
_size_bounds_fmt = _size_fmt + " +- " + _percentage_fmt


def _sizeConverter(size):
    sizeFlt = float(size)
    if size > 1e9:
        # greater than a billion, use gigabytes
        return _size_fmt.format(sizeFlt / 1e9) + " GB"
    elif size > 1e6:
        # greater than a million, use megabytes
        return _size_fmt.format(sizeFlt / 1e6) + " MB"
    elif size > 1e3:
        # greater than a thousand, use kilobytes
        return _size_fmt.format(sizeFlt / 1e3) + " KB"
    else:
        return _size_fmt.format(size)


def _secondsConverter(seconds):
    fmt = "{:.1f}"
    if seconds < 0.001:
        # less than one millisecond, use microseconds
        return fmt.format(seconds * 1000000) + " us"
    elif seconds < 1:
        # less than one second, use microseconds
        return fmt.format(seconds * 1000) + " ms"
    else:
        # extra space to align in the columns
        return fmt.format(seconds) + "  s"


def _secondsAndPercent(seconds, pct):
    return "{} ({})".format(_secondsConverter(seconds), _percentage_fmt.format(pct))


def asciiTable(title, columns, units, rows):
    # Should be max row/column width but that's complicated, just increase
    # manually if it's too small
    col_width = 28
    str_fmt = "{:>{width}s}"
    row_fmt = str_fmt * len(columns)

    print(title, ":", sep="")
    if not rows:
        # If there are no rows to display, simply output "No data"
        print("No data")
        return
    # Header line
    print(row_fmt.format(*columns, width=col_width))
    # Unit line
    unit_line = row_fmt.format(*units, width=col_width)
    print(unit_line)
    print("-" * len(unit_line))
    for r in rows:
        print(row_fmt.format(*r, width=col_width))
    print()


def tabSeparated(title, columns, units, rows):
    """Print the tabular data in tab-separated format.  This format ignores the
    title, and forms the header row by combining the column names with their
    respective units.
    """

    def printRow(row):
        print("\t".join(row))

    print(title)
    printRow(" ".join(header) for header in zip(columns, units))

    for row in rows:
        printRow(row)

    print()


def processResults(results, formatter):
    """Expects elements for results to be of the form
    (benchmark, results, bounds).
    """
    fmt = "{} +- " + _percentage_fmt

    def sizeTimeBound(stat, bound):
        return fmt.format(_sizeConverter(stat * 1024) + "ms", bound)

    rows = []
    for name, stats, bounds in results:
        if stats:
            # Only add rows to print if process data exists.
            rss_bound = sizeTimeBound(stats["Integral of RSS kBms"], bounds["rss"])
            va_bound = sizeTimeBound(stats["Integral of VA kBms"], bounds["va"])
            rows.append([name, rss_bound, va_bound])

    formatter(
        "Process Stats",
        ["Benchmark", "Integral of RSS", "Integral of VA"],
        ["", "(size * time)", "(size * time)"],
        rows,
    )


def generalResults(results, formatter):
    """Expects elements for results to be
    tuple (benchmark, results description object, bounds)
    """
    rows = []
    totalAllocRows = []
    for name, stats, bounds in results:
        derived_metrics.updateGeneralStats(stats)
        derived_metrics.updateJitStats(stats)

        total_time_bounds = _seconds_bounds_fmt.format(
            _secondsConverter(stats["totalTime"]), bounds["total"]
        )
        mut_time_bounds = _seconds_bounds_fmt.format(
            _secondsConverter(stats["mutTime"]), bounds["mut"]
        )
        gc_time_bounds = _seconds_bounds_fmt.format(
            _secondsConverter(stats["totalGCTime"]), bounds["gc"]
        )
        num_colls_bounds = _size_bounds_fmt.format(
            stats["numCollections"], bounds["gcNum"]
        )

        rows.append(
            [
                name,
                total_time_bounds,
                mut_time_bounds,
                gc_time_bounds,
                _percentage_fmt.format(stats["gcPct"]),
                _percentage_fmt.format(stats["jitGcPct"]),
                num_colls_bounds,
                _secondsConverter(stats["maxGCPause"]),
                _sizeConverter(stats["finalHeapSize"]),
            ]
        )

        totalAllocRows.append([name, _sizeConverter(stats["totalAllocatedBytes"])])

    formatter(
        "General Stats",
        [
            "Benchmark",
            "total",
            "mut",
            "gc",
            "gc%",
            "jitx gc%",
            "num gcs",
            "max gc pause",
            "final heap size",
        ],
        [
            "",
            "(time)",
            "(time)",
            "(time)",
            "(percent)",
            "(percent)",
            "(avg)",
            "(time)",
            "(bytes)",
        ],
        rows,
    )
    formatter(
        "Total Allocated Bytes",
        ["Benchmark", "total alloc"],
        ["", "(bytes)"],
        totalAllocRows,
    )


def runtimeGCResults(results, formatter):
    """Expects elements for results to be
    tuple (benchmark, results description object)
    """
    rows = []
    for name, stats in results:
        derived_metrics.updateRuntimeStats(stats)

        rows.append(
            [
                name,
                _secondsConverter(stats["totalMarkRootsTime"]),
                _secondsAndPercent(
                    stats["MarkRoots_RegistersTime"], stats["MarkRoots_RegistersPct"]
                ),
                _secondsAndPercent(
                    stats["MarkRoots_RuntimeModulesTime"],
                    stats["MarkRoots_RuntimeModulesPct"],
                ),
                _secondsAndPercent(
                    stats["MarkRoots_CharStringsTime"],
                    stats["MarkRoots_CharStringsPct"],
                ),
                _secondsAndPercent(
                    stats["MarkRoots_IdentifierTableTime"],
                    stats["MarkRoots_IdentifierTablePct"],
                ),
                _secondsAndPercent(
                    stats["MarkRoots_GCScopesTime"], stats["MarkRoots_GCScopesPct"]
                ),
                _secondsAndPercent(
                    stats["MarkRoots_otherTime"], stats["MarkRoots_otherTimePct"]
                ),
            ]
        )

    formatter(
        "Runtime::markRoots breakdown",
        [
            "Benchmark",
            "total markRoots",
            "Registers",
            "RuntimeModules",
            "CharStrings",
            "IdentifierTable",
            "GCScopes",
            "Other",
        ],
        [
            "",
            "(time)",
            "(time, %)",
            "(time, %)",
            "(time, %)",
            "(time, %)",
            "(time, %)",
            "(time, %)",
        ],
        rows,
    )


def heapInfoResults(results, formatter):
    rows = [[name, _sizeConverter(stats["Malloc size"])] for name, stats in results]

    formatter("Heap Info", ["Benchmark", "Malloc size"], ["", "(bytes)"], rows)


def generationalResults(generalResults, genGCResults, formatter):
    ygRows = []
    ygByPhaseRows = []
    fullRows = []
    fullByPhaseRows = []
    for gres, genres in zip(generalResults, genGCResults):
        benchmark, stats = genres[0:2]
        if not stats:
            # No generational stats output, skip this result
            continue

        derived_metrics.updateGenerationalStats(gres[1]["totalGCTime"], stats)

        ygNumCollections = stats["ygNumCollections"]

        ygRows.append(
            [
                benchmark,
                str(ygNumCollections),
                _secondsAndPercent(stats["ygTotalGCTime"], stats["ygGCTimePct"]),
                _secondsConverter(stats["ygAvgGCPause"]),
                _secondsConverter(stats["ygMaxGCPause"]),
                _percentage_fmt.format(stats["ygSurvivalPct"]),
                _sizeConverter(stats["ygFinalSize"]) if ygNumCollections > 0 else "N/A",
            ]
        )

        ygByPhaseRows.append(
            [
                benchmark,
                _secondsConverter(stats["ygTotalGCTime"]),
                _secondsAndPercent(
                    stats["ygMarkOldToYoungTime"], stats["ygMarkOldToYoungTimePct"]
                ),
                _secondsAndPercent(
                    stats["ygMarkRootsTime"], stats["ygMarkRootsTimePct"]
                ),
                _secondsAndPercent(
                    stats["ygScanTransitiveTime"], stats["ygScanTransitiveTimePct"]
                ),
                _secondsAndPercent(
                    stats["ygUpdateWeakRefsTime"], stats["ygUpdateWeakRefsTimePct"]
                ),
                _secondsAndPercent(
                    stats["ygFinalizersTime"], stats["ygFinalizersTimePct"]
                ),
                _secondsAndPercent(stats["ygOtherGCTime"], stats["ygOtherGCTimePct"]),
            ]
        )

        fullNumCollections = stats["fullNumCollections"]

        fullRows.append(
            [
                benchmark,
                str(fullNumCollections),
                _secondsAndPercent(stats["fullTotalGCTime"], stats["fullGCTimePct"]),
                _secondsConverter(stats["fullAvgGCPause"]),
                _secondsConverter(stats["fullMaxGCPause"]),
                _percentage_fmt.format(stats["fullSurvivalPct"]),
                _sizeConverter(stats["fullFinalSize"])
                if fullNumCollections > 0
                else "N/A",
            ]
        )

        fullByPhaseRows.append(
            [
                benchmark,
                _secondsConverter(stats["fullTotalGCTime"]),
                _secondsAndPercent(
                    stats["fullMarkRootsTime"], stats["fullMarkRootsTimePct"]
                ),
                _secondsAndPercent(
                    stats["fullMarkTransitiveTime"], stats["fullMarkTransitiveTimePct"]
                ),
                _secondsAndPercent(stats["fullSweepTime"], stats["fullSweepTimePct"]),
                _secondsAndPercent(
                    stats["fullUpdateRefsTime"], stats["fullUpdateRefsTimePct"]
                ),
                _secondsAndPercent(
                    stats["fullCompactTime"], stats["fullCompactTimePct"]
                ),
                _secondsAndPercent(
                    stats["fullOtherGCTime"], stats["fullOtherGCTimePct"]
                ),
            ]
        )

    formatter(
        "Young-generation collections",
        [
            "Benchmark",
            "num gcs",
            "gc, % of total",
            "avg pause",
            "max pause",
            "survival",
            "final size",
        ],
        ["", "", "(time, %)", "(time)", "(time)", "(% bytes)", "(bytes)"],
        ygRows,
    )

    formatter(
        "Young-generation collections by phase",
        [
            "Benchmark",
            "total gc time",
            "old-to-young",
            "roots",
            "scan trans",
            "weakRefs",
            "finalizers",
            "other",
        ],
        [
            "",
            "(time)",
            "(time, %)",
            "(time, %)",
            "(time, %)",
            "(time, %)",
            "(time, %)",
            "(time, %)",
        ],
        ygByPhaseRows,
    )

    formatter(
        "Full collections",
        [
            "Benchmark",
            "num gcs",
            "gc, % of total",
            "avg pause",
            "max pause",
            "survival",
            "final size",
        ],
        ["", "", "(time, %)", "(time)", "(time)", "(% bytes)", "(bytes)"],
        fullRows,
    )

    formatter(
        "Full collections by phase",
        [
            "Benchmark",
            "total gc time",
            "mark roots",
            "mark trans",
            "sweep",
            "update refs",
            "compact",
            "other",
        ],
        [
            "",
            "(time)",
            "(time, %)",
            "(time, %)",
            "(time, %)",
            "(time, %)",
            "(time, %)",
            "(time, %)",
        ],
        fullByPhaseRows,
    )


def cpuTimeResults(generalResults, specificResults, formatter):
    rows = []

    hasGenerational = len(specificResults) > 0 and specificResults[0][1]

    for gres, genres in zip(generalResults, specificResults):
        benchmark, genStats, genBounds = gres
        specificStats = genres[1]

        gc_cpu_time_bounds = _seconds_bounds_fmt.format(
            _secondsConverter(genStats["totalGCCPUTime"]), genBounds["gcCPU"]
        )

        row = [
            benchmark,
            gc_cpu_time_bounds,
            _secondsConverter(genStats["maxGCCPUPause"]),
        ]

        if hasGenerational:
            row.extend(
                [
                    _secondsConverter(specificStats["ygAvgGCCPUPause"]),
                    _secondsConverter(specificStats["ygMaxGCCPUPause"]),
                    _secondsConverter(specificStats["fullAvgGCCPUPause"]),
                    _secondsConverter(specificStats["fullMaxGCCPUPause"]),
                ]
            )

        rows.append(row)

    formatter(
        "CPU Times",
        ["Benchmark", "total time", "max time"]
        + (
            ["yg avg time", "yg max time", "full avg time", "full max time"]
            if hasGenerational
            else []
        ),
        ["", "(time +- %)", "(time)"]
        + (["(time)", "(time)", "(time)", "(time)"] if hasGenerational else []),
        rows,
    )
