# (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.


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

    def __repr__(self):
        return "Benchmark(name={})".format(self.name)


class BenchmarkWithGCConfig(Benchmark):
    """A BenchmarkWithGCConfig is a name, and requirements for its initial heap
    and its max heap."""

    def __init__(self, name, gcMinHeap=None, gcInitHeap=None, gcMaxHeap=None):
        Benchmark.__init__(self, name)
        self.gcMinHeap = gcMinHeap
        self.gcInitHeap = gcInitHeap
        self.gcMaxHeap = gcMaxHeap

    def __repr__(self):
        return "BenchmarkWithGCConfig(Benchmark={}, gcMinHeap = {}, gcInitHeap = {}, gcMaxHeap = {})".format(
            Benchmark.__repr__(self), self.gcMinHeap, self.gcInitHeap, self.gcMaxHeap
        )


class ResourceBenchmark(BenchmarkWithGCConfig):
    """A benchmark that can be built and run with a resource pointing to a JS file"""

    def __init__(self, name, resource, **kwargs):
        BenchmarkWithGCConfig.__init__(self, name, **kwargs)
        self.resource = resource

    def run(self, resourceResolver, runner):
        return runner(
            self.name,
            resourceResolver(self.resource),
            self.gcMinHeap,
            self.gcInitHeap,
            self.gcMaxHeap,
        )

    def __repr__(self):
        return "ResourceBenchmark(BenchmarkWithGCConfig = {}, resource = {})".format(
            BenchmarkWithGCConfig.__repr__(self), self.resource
        )


class SynthBenchmark(BenchmarkWithGCConfig):
    def __init__(self, name, traceResource, bytecodeResources, marker=None, **kwargs):
        BenchmarkWithGCConfig.__init__(self, name, **kwargs)
        self.traceResource = traceResource
        self.bytecodeResources = bytecodeResources
        self.marker = marker

    def run(self, resourceResolver, runner):
        return runner(
            self.name,
            self.gcMinHeap,
            self.gcInitHeap,
            self.gcMaxHeap,
            resourceResolver(self.traceResource),
            [resourceResolver(rsc) for rsc in self.bytecodeResources],
            self.marker,
        )

    def __repr__(self):
        return "SynthBenchmark(BenchmarkWithGCConfig = {}, traceResource = {}, bytecodeResources = {}, marker = {})".format(
            BenchmarkWithGCConfig.__repr__(self),
            self.traceResource,
            self.bytecodeResources,
            self.marker,
        )


# A group of benchmarks
Category = namedtuple("Category", ["name", "benchmarks", "runByDefault"])

_v8 = Category(
    name="v8",
    benchmarks=[
        ResourceBenchmark(
            "v8-crypto", "v8-v6-perf/v8-crypto.js", gcMinHeap="1M", gcMaxHeap="1M"
        ),
        ResourceBenchmark(
            "v8-deltablue", "v8-v6-perf/v8-deltablue.js", gcMinHeap="2M", gcMaxHeap="2M"
        ),
        ResourceBenchmark(
            "v8-raytrace", "v8-v6-perf/v8-raytrace.js", gcMinHeap="1M", gcMaxHeap="1M"
        ),
        ResourceBenchmark(
            "v8-regexp", "v8-v6-perf/v8-regexp.js", gcMinHeap="2M", gcMaxHeap="2M"
        ),
        ResourceBenchmark(
            "v8-richards",
            "v8-v6-perf/v8-richards.js",
            gcMinHeap="512K",
            gcMaxHeap="512K",
        ),
        ResourceBenchmark(
            "v8-splay", "v8-v6-perf/v8-splay.js", gcMinHeap="128M", gcMaxHeap="512M"
        ),
    ],
    runByDefault=True,
)

_tsc = Category(
    name="tsc",
    benchmarks=[
        ResourceBenchmark(
            "tsc-vs-chunk", "tsc/tsc-vs-chunk.js", gcMinHeap="256M", gcMaxHeap="256M"
        )
    ],
    runByDefault=True,
)

_octane = Category(
    name="octane",
    benchmarks=[
        ResourceBenchmark("box2d", "octane/box2d.js", gcMinHeap="32M", gcMaxHeap="32M"),
        ResourceBenchmark(
            "earley-boyer", "octane/earley-boyer.js", gcMinHeap="64M", gcMaxHeap="64M"
        ),
        ResourceBenchmark(
            "navier-stokes", "octane/navier-stokes.js", gcMinHeap="4M", gcMaxHeap="4M"
        ),
        ResourceBenchmark(
            "pdfjs", "octane/pdfjs.js", gcMinHeap="128M", gcMaxHeap="128M"
        ),
        ResourceBenchmark("gbemu", "octane/gbemu.js", gcMinHeap="32M", gcMaxHeap="32M"),
        ResourceBenchmark(
            "code-load", "octane/code-load.js", gcMinHeap="32M", gcMaxHeap="32M"
        ),
        ResourceBenchmark(
            "typescript", "octane/typescript.js", gcMinHeap="512M", gcMaxHeap="512M"
        ),
    ],
    runByDefault=True,
)

_micros = Category(
    name="micros",
    benchmarks=[
        ResourceBenchmark(
            "simpleSum", "hvm-bench/simpleSum.js", gcMinHeap="1M", gcMaxHeap="1M"
        ),
        ResourceBenchmark(
            "propAccess", "hvm-bench/propAccess.js", gcMinHeap="1M", gcMaxHeap="1M"
        ),
        ResourceBenchmark(
            "allocObj", "hvm-bench/allocObj.js", gcMinHeap="1M", gcMaxHeap="1M"
        ),
        ResourceBenchmark(
            "allocObjLit", "hvm-bench/allocObjLit.js", gcMinHeap="1M", gcMaxHeap="1M"
        ),
        ResourceBenchmark(
            "allocNewObj", "hvm-bench/allocNewObj.js", gcMinHeap="1M", gcMaxHeap="1M"
        ),
        ResourceBenchmark(
            "allocArray", "hvm-bench/allocArray.js", gcMinHeap="1M", gcMaxHeap="1M"
        ),
        ResourceBenchmark(
            "allocNewArray",
            "hvm-bench/allocNewArray.js",
            gcMinHeap="1M",
            gcMaxHeap="1M",
        ),
        ResourceBenchmark(
            "arrayRead", "hvm-bench/arrayRead.js", gcMinHeap="1M", gcMaxHeap="1M"
        ),
        ResourceBenchmark(
            "largeArrayRead",
            "hvm-bench/largeArrayRead.js",
            gcMinHeap="1M",
            gcMaxHeap="1M",
        ),
        ResourceBenchmark(
            "arrayWrite", "hvm-bench/arrayWrite.js", gcMinHeap="1M", gcMaxHeap="1M"
        ),
        ResourceBenchmark(
            "largeArrayWrite",
            "hvm-bench/largeArrayWrite.js",
            gcMinHeap="1M",
            gcMaxHeap="1M",
        ),
        ResourceBenchmark(
            "interp-dispatch",
            "hvm-bench/interp-dispatch.js",
            gcMinHeap="1M",
            gcMaxHeap="1M",
        ),
        ResourceBenchmark(
            "wb-perf", "hvm-bench/wb-perf.js", gcMinHeap="1M", gcMaxHeap="1M"
        ),
        ResourceBenchmark(
            "arrayReverse", "hvm-bench/arrayReverse.js", gcMinHeap="1M", gcMaxHeap="1M"
        ),
        ResourceBenchmark(
            "arrayMap", "hvm-bench/arrayMap.js", gcMinHeap="1M", gcMaxHeap="1M"
        ),
        ResourceBenchmark(
            "arrayIndexOf", "hvm-bench/arrayIndexOf.js", gcMinHeap="1M", gcMaxHeap="1M"
        ),
        ResourceBenchmark(
            "arrayLastIndexOf",
            "hvm-bench/arrayLastIndexOf.js",
            gcMinHeap="1M",
            gcMaxHeap="1M",
        ),
        ResourceBenchmark(
            "arrayEvery", "hvm-bench/arrayEvery.js", gcMinHeap="1M", gcMaxHeap="1M"
        ),
        ResourceBenchmark(
            "arraySome", "hvm-bench/arraySome.js", gcMinHeap="1M", gcMaxHeap="1M"
        ),
        ResourceBenchmark(
            "arrayFill", "hvm-bench/arrayFill.js", gcMinHeap="1M", gcMaxHeap="1M"
        ),
        ResourceBenchmark(
            "arrayFilter", "hvm-bench/arrayFilter.js", gcMinHeap="1M", gcMaxHeap="1M"
        ),
        ResourceBenchmark(
            "arrayFind", "hvm-bench/arrayFind.js", gcMinHeap="1M", gcMaxHeap="1M"
        ),
        ResourceBenchmark(
            "arrayFindIndex",
            "hvm-bench/arrayFindIndex.js",
            gcMinHeap="1M",
            gcMaxHeap="1M",
        ),
        ResourceBenchmark(
            "arrayPop", "hvm-bench/arrayPop.js", gcMinHeap="1M", gcMaxHeap="1M"
        ),
        ResourceBenchmark(
            "arrayReduce", "hvm-bench/arrayReduce.js", gcMinHeap="1M", gcMaxHeap="1M"
        ),
        ResourceBenchmark(
            "arrayReduceRight",
            "hvm-bench/arrayReduceRight.js",
            gcMinHeap="1M",
            gcMaxHeap="1M",
        ),
        ResourceBenchmark(
            "arrayShift", "hvm-bench/arrayShift.js", gcMinHeap="1M", gcMaxHeap="1M"
        ),
        ResourceBenchmark(
            "arrayUnshift", "hvm-bench/arrayUnshift.js", gcMinHeap="1M", gcMaxHeap="1M"
        ),
        ResourceBenchmark(
            "arrayIncludes",
            "hvm-bench/arrayIncludes.js",
            gcMinHeap="1M",
            gcMaxHeap="1M",
        ),
        ResourceBenchmark(
            "arrayFrom", "hvm-bench/arrayFrom.js", gcMinHeap="1M", gcMaxHeap="1M"
        ),
        ResourceBenchmark(
            "arrayCopyWithin",
            "hvm-bench/arrayCopyWithin.js",
            gcMinHeap="1M",
            gcMaxHeap="1M",
        ),
        ResourceBenchmark(
            "stringFromCharCode",
            "hvm-bench/stringFromCharCode.js",
            gcMinHeap="1M",
            gcMaxHeap="1M",
        ),
        ResourceBenchmark(
            "arraySlice", "hvm-bench/arraySlice.js", gcMinHeap="1M", gcMaxHeap="1M"
        ),
        ResourceBenchmark(
            "arraySplice", "hvm-bench/arraySplice.js", gcMinHeap="1M", gcMaxHeap="1M"
        ),
        ResourceBenchmark(
            "arrayOf", "hvm-bench/arrayOf.js", gcMinHeap="1M", gcMaxHeap="1M"
        ),
        ResourceBenchmark(
            "stringCharAt", "hvm-bench/stringCharAt.js", gcMinHeap="1M", gcMaxHeap="1M"
        ),
        ResourceBenchmark(
            "stringMatch", "hvm-bench/stringMatch.js", gcMinHeap="1M", gcMaxHeap="1M"
        ),
        ResourceBenchmark(
            "stringSearch", "hvm-bench/stringSearch.js", gcMinHeap="1M", gcMaxHeap="1M"
        ),
        ResourceBenchmark(
            "stringStartsWith",
            "hvm-bench/stringStartsWith.js",
            gcMinHeap="1M",
            gcMaxHeap="1M",
        ),
        ResourceBenchmark(
            "stringEndsWith",
            "hvm-bench/stringEndsWith.js",
            gcMinHeap="1M",
            gcMaxHeap="1M",
        ),
        ResourceBenchmark(
            "stringIncludes",
            "hvm-bench/stringIncludes.js",
            gcMinHeap="1M",
            gcMaxHeap="1M",
        ),
        ResourceBenchmark(
            "stringIndexOf",
            "hvm-bench/stringIndexOf.js",
            gcMinHeap="1M",
            gcMaxHeap="1M",
        ),
        ResourceBenchmark(
            "stringLastIndexOf",
            "hvm-bench/stringLastIndexOf.js",
            gcMinHeap="1M",
            gcMaxHeap="1M",
        ),
        ResourceBenchmark(
            "stringSplit", "hvm-bench/stringSplit.js", gcMinHeap="1M", gcMaxHeap="1M"
        ),
        ResourceBenchmark(
            "stringSlice", "hvm-bench/stringSlice.js", gcMinHeap="1M", gcMaxHeap="1M"
        ),
        ResourceBenchmark(
            "stringPadStart",
            "hvm-bench/stringPadStart.js",
            gcMinHeap="1M",
            gcMaxHeap="1M",
        ),
        ResourceBenchmark(
            "stringPadEnd", "hvm-bench/stringPadEnd.js", gcMinHeap="1M", gcMaxHeap="1M"
        ),
        ResourceBenchmark(
            "regExpMatch", "hvm-bench/regExpMatch.js", gcMinHeap="1M", gcMaxHeap="1M"
        ),
        ResourceBenchmark(
            "regExpSearch", "hvm-bench/regExpSearch.js", gcMinHeap="1M", gcMaxHeap="1M"
        ),
        ResourceBenchmark(
            "regExpToString",
            "hvm-bench/regExpToString.js",
            gcMinHeap="1M",
            gcMaxHeap="1M",
        ),
        ResourceBenchmark(
            "stringReplace",
            "hvm-bench/stringReplace.js",
            gcMinHeap="1M",
            gcMaxHeap="1M",
        ),
        ResourceBenchmark(
            "regExpReplace",
            "hvm-bench/regExpReplace.js",
            gcMinHeap="1M",
            gcMaxHeap="1M",
        ),
        ResourceBenchmark(
            "regExpFlags", "hvm-bench/regExpFlags.js", gcMinHeap="1M", gcMaxHeap="1M"
        ),
        ResourceBenchmark(
            "regExpSplit", "hvm-bench/regExpSplit.js", gcMinHeap="1M", gcMaxHeap="1M"
        ),
    ],
    runByDefault=False,
)

synths = Category(
    name="synth",
    benchmarks=[
        SynthBenchmark(
            "fb4a-marketplace-big-gc",
            "fb4a_marketplace_big_gc_synth/trace.json",
            [
                "fb4a_marketplace_big_gc_synth/Fb4aBundle.js.hbc",
                "fb4a_marketplace_big_gc_synth/432.js.hbc",
                "fb4a_marketplace_big_gc_synth/433.js.hbc",
                "fb4a_marketplace_big_gc_synth/434.js.hbc",
                "fb4a_marketplace_big_gc_synth/436.js.hbc",
            ],
        ),
        SynthBenchmark(
            "fb4a-marketplace-extra-gc",
            "fb4a_marketplace_extra_gc_synth/trace.json",
            [
                "fb4a_marketplace_extra_gc_synth/Fb4aBundle.js.hbc",
                "fb4a_marketplace_extra_gc_synth/489.js.hbc",
                "fb4a_marketplace_extra_gc_synth/490.js.hbc",
                "fb4a_marketplace_extra_gc_synth/491.js.hbc",
                "fb4a_marketplace_extra_gc_synth/523.js.hbc",
                "fb4a_marketplace_extra_gc_synth/524.js.hbc",
            ],
        ),
        SynthBenchmark(
            "oculus-reactvr-store",
            "oculus_reactvr_store/trace.json",
            ["oculus_reactvr_store/bundle.js.hbc", "oculus_reactvr_store/intl.js.hbc"],
            gcMinHeap="32M",
            gcMaxHeap="32M",
        ),
        SynthBenchmark(
            "oculus-reactvr-library",
            "oculus_reactvr_library/trace.json",
            ["oculus_reactvr_library/bundle.js.hbc"],
            gcMinHeap="32M",
            gcMaxHeap="32M",
        ),
        SynthBenchmark(
            "oculus-reactvr-settings",
            "oculus_reactvr_settings/trace.json",
            ["oculus_reactvr_settings/bundle.js.hbc"],
            gcMinHeap="32M",
            gcMaxHeap="32M",
        ),
        SynthBenchmark(
            "oculus-reactvr-home-search",
            "oculus_reactvr_home_search/trace.json",
            ["oculus_reactvr_home_search/reactnative.js.hbc"],
        ),
        SynthBenchmark(
            "oculus-reactvr-system-search",
            "oculus_reactvr_system_search/trace.json",
            ["oculus_reactvr_system_search/reactnative.js.hbc"],
        ),
        SynthBenchmark(
            "fb4a-marketplace-tti",
            "fb4a_marketplace_big_gc_synth/trace.json",
            ["fb4a_marketplace_big_gc_synth/bundle.js.hbc"],
            marker="tti",
            gcMinHeap="32M",
            gcMaxHeap="32M",
        ),
        SynthBenchmark(
            "fb4a-marketplace-global-code",
            "fb4a_marketplace_big_gc_synth/trace.json",
            ["fb4a_marketplace_big_gc_synth/bundle.js.hbc"],
            marker="end_global_code",
            gcMinHeap="32M",
            gcMaxHeap="32M",
        ),
        SynthBenchmark(
            "simpleSynth",
            "simpleSynth/trace.json",
            ["simpleSynth/bundle.js.hbc"],
            gcMinHeap="1M",
            gcMaxHeap="1M",
        ),
    ],
    runByDefault=True,
)

categories = [_v8, _tsc, _octane, _micros, synths]
