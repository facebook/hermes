# (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

import derived_metrics
import stats
import sys
import json


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


def secondsAndBounds(stat):
    return _seconds_bounds_fmt.format(_secondsConverter(stat.avg()), stat.bounds())


def _sizeAndBounds(stat):
    return _size_bounds_fmt.format(stat.avg(), stat.bounds())


class Formatter:
    def __init__(self, raw):
        self.raw = raw;


class AsciiTableFormatter(Formatter):
    colWidth = 28
    strFmt = "{:>{width}s}"

    def __init__(self):
        Formatter.__init__(self, False)


    def format(self, outfile, runtime, title, columns, units, rows):
        # Should be max row/column width but that's complicated, just increase
        # manually if it's too small
        row_fmt = self.strFmt * len(columns)

        print("Runtime: ", runtime)
        print(title, ":", sep="", file=outfile)
        if not rows:
            # If there are no rows to display, simply output "No data"
            print("No data", file=outfile)
            return
        # Header line
        print(row_fmt.format(*columns, width=self.colWidth), file=outfile)
        # Unit line
        unit_line = row_fmt.format(*units, width=self.colWidth)
        print(unit_line, file=outfile)
        print("-" * len(unit_line), file=outfile)
        for r in rows:
            print(row_fmt.format(*r, width=self.colWidth), file=outfile)


class TabSeparatedFormatter(Formatter):
    def __init__(self):
        Formatter.__init__(self, False)


    def format(self, outfile, runtime, title, columns, units, rows):
        """Print the tabular data in tab-separated format.  This format ignores the
        title, and forms the header row by combining the column names with their
        respective units.
        """

        def printRow(outfile, row):
            print("\t".join(row), file=outfile)

        print(title, file=outfile)
        printRow(outfile, (" ".join(header) for header in zip(columns, units)))

        for row in rows:
            printRow(outfile, row)

        print(file=outfile)


class JSONFormatter(Formatter):
    def __init__(self):
        Formatter.__init__(self, True)


    def format(self, outfile, runtime, results):
        """Print the tabular data in json format.
        """
        res = {"runtime": runtime, "results": results }
        encoder = stats.MetricEncoder()
        print(encoder.encode(res), file=outfile)


def generalResults(outfileName, runtime, results, formatter):
    if outfileName:
        with open(outfileName, 'w') as outfile:
           generalResultsToFile(outfile, runtime, results, formatter)
    else:
        generalResultsToFile(sys.stdout, runtime, results, formatter)


def generalResultsToFile(outfile, runtime, results, formatter):        
    """Expects elements for results to be
    tuple (benchmark, results description object, bounds)
    """

    if formatter.raw:
        return formatter.format(outfile, runtime, results)
        
    rows = []
    totalAllocRows = []
    for name, stats in results.items():
        derived_metrics.updateGeneralStats(stats)

        total_time_bounds = secondsAndBounds(stats["totalTime"])

        rows.append(
            [
                name,
                total_time_bounds,
            ]
        )

    formatter.format(
        outfile,
        runtime,
        "General Stats",
        [
            "Benchmark",
            "total",
        ],
        [
            "",
            "(time)",
        ],
        rows,
    )
