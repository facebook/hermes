#!/usr/bin/env python2.7

from collections import namedtuple


"""Category

A description of a group of test files, containing:

 - Its category (A name for the group of tests).
 - A list of (file, min size, max size) triples, describing the base file name,
   and the "natural" GC heap size for the benchmark.

"""


class Benchmark:
    """A Benchmark is something that can be run, that has a name"""

    def __init__(self, name):
        self.name = name

    def run(self, runner):
        """Runs the benchmark with the given runtime command"""
        pass


class BenchmarkWithGCConfig(Benchmark):
    """A BenchmarkWithGCConfig is a name, and requirements for its initial heap
    and its max heap."""

    def __init__(self, name, gcMinHeap, gcMaxHeap):
        Benchmark.__init__(self, name)
        self.gcMinHeap = gcMinHeap
        self.gcMaxHeap = gcMaxHeap


class ResourceBenchmark(BenchmarkWithGCConfig):
    """A benchmark that can be built and run with a resource pointing to a JS file"""

    def __init__(self, name, gcMinHeap, gcMaxHeap, resource):
        BenchmarkWithGCConfig.__init__(self, name, gcMinHeap, gcMaxHeap)
        self.resource = resource

    def run(self, resourceResolver, runner):
        return runner(
            self.name, resourceResolver(self.resource), self.gcMinHeap, self.gcMaxHeap
        )

    def __repr__(self):
        return "ResourceBenchmark(name = {}, gcMinHeap = {}, gcMaxHeap = {}, resource = {})".format(
            self.name, self.gcMinHeap, self.gcMaxHeap, self.resource
        )


class SynthBenchmark(BenchmarkWithGCConfig):
    def __init__(self, name, gcMinHeap, gcMaxHeap, traceResource, bytecodeResource):
        BenchmarkWithGCConfig.__init__(self, name, gcMinHeap, gcMaxHeap)
        self.traceResource = traceResource
        self.bytecodeResource = bytecodeResource

    def run(self, resourceResolver, runner):
        return runner(
            self.name,
            self.gcMinHeap,
            self.gcMaxHeap,
            resourceResolver(self.traceResource),
            resourceResolver(self.bytecodeResource),
        )

    def __repr__(self):
        return "SynthBenchmark(name = {}, gcMinHeap = {}, gcMaxHeap = {}, traceResource = {}, bytecodeResource = {})".format(
            self.name,
            self.gcMinHeap,
            self.gcMaxHeap,
            self.traceResource,
            self.bytecodeResource,
        )


class SynthBenchmarkWithMarkerStop(SynthBenchmark):
    def __init__(
        self, name, gcMinHeap, gcMaxHeap, traceResource, bytecodeResource, marker
    ):
        SynthBenchmark.__init__(
            self, name, gcMinHeap, gcMaxHeap, traceResource, bytecodeResource
        )
        self.marker = marker

    def run(self, resourceResolver, runner):
        return runner(
            self.name,
            self.gcMinHeap,
            self.gcMaxHeap,
            resourceResolver(self.traceResource),
            resourceResolver(self.bytecodeResource),
            self.marker,
        )

    def __repr__(self):
        return "SynthBenchmarkWithMarkerStop(name = {}, gcMinHeap = {}, gcMaxHeap = {}, traceResource = {}, bytecodeResource = {}, marker = {})".format(
            self.name,
            self.gcMinHeap,
            self.gcMaxHeap,
            self.traceResource,
            self.bytecodeResource,
            self.marker,
        )


# A group of benchmarks
Category = namedtuple("Category", ["name", "benchmarks", "runByDefault"])

_v8 = Category(
    name="v8",
    benchmarks=[
        ResourceBenchmark("v8-crypto", "1M", "1M", "v8-v6-perf/v8-crypto.js"),
        ResourceBenchmark("v8-deltablue", "2M", "2M", "v8-v6-perf/v8-deltablue.js"),
        ResourceBenchmark("v8-raytrace", "1M", "1M", "v8-v6-perf/v8-raytrace.js"),
        ResourceBenchmark("v8-regexp", "2M", "2M", "v8-v6-perf/v8-regexp.js"),
        ResourceBenchmark("v8-richards", "512K", "512K", "v8-v6-perf/v8-richards.js"),
        ResourceBenchmark("v8-splay", "128M", "512M", "v8-v6-perf/v8-splay.js"),
    ],
    runByDefault=True,
)

_tsc = Category(
    name="tsc",
    benchmarks=[
        ResourceBenchmark("tsc-vs-chunk", "256M", "256M", "tsc/tsc-vs-chunk.js")
    ],
    runByDefault=True,
)

_octane = Category(
    name="octane",
    benchmarks=[
        ResourceBenchmark("box2d", "32M", "32M", "octane/box2d.js"),
        ResourceBenchmark("earley-boyer", "64M", "64M", "octane/earley-boyer.js"),
        ResourceBenchmark("navier-stokes", "4M", "4M", "octane/navier-stokes.js"),
        ResourceBenchmark("pdfjs", "128M", "128M", "octane/pdfjs.js"),
        ResourceBenchmark("gbemu", "32M", "32M", "octane/gbemu.js"),
        ResourceBenchmark("code-load", "32M", "32M", "octane/code-load.js"),
        ResourceBenchmark("typescript", "512M", "512M", "octane/typescript.js"),
    ],
    runByDefault=True,
)

_rnBundles = Category(
    name="rn",
    benchmarks=[
        ResourceBenchmark("rn_ig", "16M", "16M", "rn_ig.js"),
        ResourceBenchmark("rn_ig_tti", "16M", "16M", "rn_ig_tti.js"),
        ResourceBenchmark("rn_wilde", "8M", "16M", "rn_wilde.js"),
        ResourceBenchmark("rn_wilde_tti", "8M", "16M", "rn_wilde_tti.js"),
    ],
    runByDefault=True,
)

_ssjs = Category(
    name="ssjs",
    benchmarks=[ResourceBenchmark("ssjs6", "16M", "16M", "ssjs6.js")],
    runByDefault=True,
)

_csBundles = Category(
    name="cs",
    benchmarks=[
        ResourceBenchmark("cs", "1M", "8M", "cs.js"),
        ResourceBenchmark("cs-prepacked", "1M", "8M", "cs-prepacked.js"),
    ],
    runByDefault=True,
)

_micros = Category(
    name="micros",
    benchmarks=[
        ResourceBenchmark("simpleSum", "1M", "1M", "hvm-bench/simpleSum.js"),
        ResourceBenchmark("propAccess", "1M", "1M", "hvm-bench/propAccess.js"),
        ResourceBenchmark("allocObj", "1M", "1M", "hvm-bench/allocObj.js"),
        ResourceBenchmark("allocObjLit", "1M", "1M", "hvm-bench/allocObjLit.js"),
        ResourceBenchmark("allocNewObj", "1M", "1M", "hvm-bench/allocNewObj.js"),
        ResourceBenchmark("allocArray", "1M", "1M", "hvm-bench/allocArray.js"),
        ResourceBenchmark("allocNewArray", "1M", "1M", "hvm-bench/allocNewArray.js"),
        ResourceBenchmark("arrayRead", "1M", "1M", "hvm-bench/arrayRead.js"),
        ResourceBenchmark("largeArrayRead", "1M", "1M", "hvm-bench/largeArrayRead.js"),
        ResourceBenchmark("arrayWrite", "1M", "1M", "hvm-bench/arrayWrite.js"),
        ResourceBenchmark(
            "largeArrayWrite", "1M", "1M", "hvm-bench/largeArrayWrite.js"
        ),
        ResourceBenchmark(
            "interp-dispatch", "1M", "1M", "hvm-bench/interp-dispatch.js"
        ),
        ResourceBenchmark("wb-perf", "1M", "1M", "hvm-bench/wb-perf.js"),
    ],
    runByDefault=False,
)

synths = Category(
    name="synth",
    benchmarks=[
        SynthBenchmark(
            "fb4a-marketplace-big-gc",
            "140M",
            "140M",
            "fb4a_marketplace_big_gc_synth/trace.json",
            "fb4a_marketplace_big_gc_synth/bundle.js.hbc",
        ),
        SynthBenchmarkWithMarkerStop(
            "fb4a-marketplace-tti",
            "32M",
            "32M",
            "fb4a_marketplace_big_gc_synth/trace.json",
            "fb4a_marketplace_big_gc_synth/bundle.js.hbc",
            "tti",
        ),
        SynthBenchmarkWithMarkerStop(
            "fb4a-marketplace-global-code",
            "32M",
            "32M",
            "fb4a_marketplace_big_gc_synth/trace.json",
            "fb4a_marketplace_big_gc_synth/bundle.js.hbc",
            "end_global_code",
        ),
    ],
    runByDefault=True,
)

categories = [_v8, _tsc, _octane, _rnBundles, _csBundles, _micros, _ssjs, synths]
