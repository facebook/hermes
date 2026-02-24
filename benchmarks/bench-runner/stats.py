# (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

import json
import logging
import math
import numbers
from collections import Counter, defaultdict
from math import sqrt


logger = logging.getLogger(__name__)


class MetricEncoder(json.JSONEncoder):
    def default(self, obj):
        if isinstance(obj, Metric):
            return {"mean": obj.avg(), "stdev": obj.stdDev(), "samples": obj.samples}
        # Let the base class default method raise the TypeError
        return json.JSONEncoder.default(self, obj)


# Represents a metric, which consists of samples, averages, std dev, median, etc.
class Metric:
    def __init__(self, samples=None):
        self.samples = samples or []

    def addSample(self, *samples):
        self.samples.extend(samples)

    def avg(self):
        return self._meanAndStdDev()[0]

    def stdDev(self):
        """Calculate standard deviation of a list"""
        return self._meanAndStdDev()[1]

    def bounds(self):
        return 2 * self.stdDevPercentage()

    def percentile(self, pct):
        """Return the sample for which pct% is less than itself"""
        if not self.samples:
            return 0
        # Use ceil so that if the percentile doesn't map evenly, use the next
        # highest number, an overestimate of the percentile, rather than an
        # underestimate.
        pct_as_len = math.ceil(pct * len(self.samples))
        samples_copy = list(self.samples)
        samples_copy.sort()
        return samples_copy[pct_as_len - 1]

    def stdDevPercentage(self):
        """Calculate standard deviation of a list and return it as percentage of the
        mean"""
        mean, sd = self._meanAndStdDev()
        if mean == 0:
            return 0
        return (sd / mean) * 100.0

    def _meanAndStdDev(self):
        """Calculate and return mean and standard deviation of a list"""
        if not self.samples:
            return (0, 0)
        length = float(len(self.samples))
        mean = sum(self.samples) / length
        return (mean, sqrt(sum((v - mean) ** 2 for v in self.samples) / length))

    def __len__(self):
        return len(self.samples)

    def __add__(self, other):
        if not isinstance(other, Metric):
            return Metric([sample + other for sample in self.samples])
        if len(self.samples) == 0:
            return other
        elif len(other.samples) == 0:
            return self
        assert len(self.samples) == len(
            other.samples
        ), "Cannot add two metrics with different sample numbers"
        return Metric([a + b for a, b in zip(self.samples, other.samples)])

    def __sub__(self, other):
        if not isinstance(other, Metric):
            return Metric([sample - other for sample in self.samples])
        if len(self.samples) == 0:
            return other
        elif len(other.samples) == 0:
            return self
        assert len(self.samples) == len(
            other.samples
        ), "Cannot subtract two metrics with different sample numbers"
        return Metric([a - b for a, b in zip(self.samples, other.samples)])

    def __mul__(self, other):
        if not isinstance(other, Metric):
            return Metric([sample * other for sample in self.samples])
        if len(self.samples) == 0:
            return other
        elif len(other.samples) == 0:
            return self
        assert len(self.samples) == len(
            other.samples
        ), "Cannot multiply two metrics with different sample numbers"
        return Metric([a * b for a, b in zip(self.samples, other.samples)])

    def __truediv__(self, other):
        if not isinstance(other, Metric):
            return Metric([sample / other for sample in self.samples])
        if len(self.samples) == 0:
            return other
        elif len(other.samples) == 0:
            return self
        assert len(self.samples) == len(
            other.samples
        ), "Cannot divide two metrics with different sample numbers"
        return Metric([a / b if b else 0 for a, b in zip(self.samples, other.samples)])


class HermesGCStatsCollector:
    def __init__(self, runner):
        self.runner = runner

    def run(self, *args):
        metrics = {}
        metrics["general"] = defaultdict(Metric)
        metrics["runtime"] = defaultdict(Metric)
        metrics["heapInfo"] = defaultdict(Metric)
        metrics["specific"] = defaultdict(Metric)
        metrics["process"] = defaultdict(Metric)
        metrics["collections"] = defaultdict(lambda: defaultdict(Metric))
        skipBenchmark = False

        # Forward the arguments to the runner.
        for gcStats, processStats in self.runner.run(*args):
            if not gcStats:
                skipBenchmark = True
                break

            generalStats = gcStats["general"]
            if not generalStats:
                raise Exception("GC stats must include general stats")
            for stat, val in generalStats.items():
                if isinstance(val, numbers.Number):
                    metrics["general"][stat].addSample(val)
            for stat, val in gcStats["runtime"].items():
                if isinstance(val, numbers.Number):
                    metrics["runtime"][stat].addSample(val)
            for stat, val in gcStats["heapInfo"].items():
                if isinstance(val, numbers.Number):
                    metrics["heapInfo"][stat].addSample(val)
            for stat, val in gcStats["specific"]["stats"].items():
                if isinstance(val, numbers.Number):
                    metrics["specific"][stat].addSample(val)
            metrics["collector"] = gcStats["specific"]["collector"]

            collectionTypeCount = Counter({})
            for coll in gcStats["collections"]:
                # Keep collection type to separate collections.
                collectionType = coll.pop("collectionType", "unknown")
                collectionTypeCount[collectionType] += 1
                for stat, val in coll.items():
                    if isinstance(val, numbers.Number):
                        metrics["collections"][collectionType][stat].addSample(val)
            for collectionType, count in collectionTypeCount.items():
                metrics["collections"][collectionType]["count"].addSample(count)

            # Process stats can be missing
            if processStats:
                for stat, val in processStats.items():
                    if isinstance(val, numbers.Number):
                        metrics["process"][stat].addSample(val)

        if skipBenchmark:
            return None

        return metrics


class StatsCollector:
    def __init__(self, runner):
        self.runner = runner

    def run(self, *args):
        metrics = {}
        metrics["general"] = defaultdict(Metric)
        skipBenchmark = False

        # Forward the arguments to the runner.
        for stats in self.runner.run(*args):
            if not stats:
                skipBenchmark = True
                break

            generalStats = stats["general"]
            if not generalStats:
                raise Exception("GC stats must include general stats")
            for stat, val in generalStats.items():
                if isinstance(val, numbers.Number):
                    metrics["general"][stat].addSample(val)

        if skipBenchmark:
            return None

        return metrics
    
