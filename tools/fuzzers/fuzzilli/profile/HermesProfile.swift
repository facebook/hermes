/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

import Fuzzilli

let hermesProfile = Profile(
    getProcessArguments:  { (randomizingArguments: Bool) -> [String] in
      var args = ["--reprl"]

      guard randomizingArguments else { return args }

      if probability(0.5) { args.append("--compile") }
      if probability(0.5) { args.append("--lazy-compilation") }
      if probability(0.5) { args.append("--eager-compilation") }
      if probability(0.5) { args.append("--optimize") }
      if probability(0.5) { args.append("--async-break") }
      if probability(0.5) { args.append("--block-scoping") }
      if probability(0.5) { args.append("--random-mem-layout") }

      return args
    },

    processEnv: ["UBSAN_OPTIONS": "handle_segv=0"],

    maxExecsBeforeRespawn: 1000,

    timeout: 2000,

    codePrefix: """
                function main(){
                """,

    codeSuffix: """
                }; main();
                """,

    ecmaVersion: ECMAScriptVersion.es6,

    crashTests: ["fuzzilli('FUZZILLI_CRASH', 0)", "fuzzilli('FUZZILLI_CRASH', 1)", "fuzzilli('FUZZILLI_CRASH', 2)"],

    additionalCodeGenerators: [],

    additionalProgramTemplates: WeightedList<ProgramTemplate>([]),

    disabledCodeGenerators: ["AsyncArrowFunctionGenerator", "AsyncGeneratorFunctionGenerator", "ClassGenerator", "WithStatementGenerator", "JITFunctionGenerator", "GrowableSharedArrayBufferGenerator"],

    additionalBuiltins: [
        "gc"                                             : .function([] => .undefined),
        "print"                                          : .function([.plain(.anything)] => .undefined),
        "HermesInternal.enqueueJob"                      : .function([.plain(.function())] => .undefined),
        "HermesInternal.setPromiseRejectionTrackingHook" : .function([.plain(.function())] => .undefined),
        "HermesInternal.enablePromiseRejectionTracker"   : .function([.plain(.anything)] => .anything),
        "HermesInternal.getEpilogues"                    : .function([] => .iterable),
        "HermesInternal.getInstrumentedStats"            : .function([] => .object(ofGroup: "Object", withProperties: ["js_VMExperiments", "js_numGCs", "js_gcCPUTime", "js_gcTime", "js_totalAllocatedBytes", "js_allocatedBytes", "js_heapSize", "js_mallocSizeEstimate", "js_vaSize", "js_markStackOverflows"])),
        "HermesInternal.getRuntimeProperties"            : .function([] => .object(ofGroup: "Object", withProperties: ["Snapshot VM", "Bytecode Version", "Builtins Frozen", "VM Experiments", "Build", "GC", "OSS Release Version", "CommonJS Modules"])),
        "HermesInternal.ttiReached"                      : .function([] => .undefined),
        "HermesInternal.getFunctionLocation"             : .function([.plain(.function())] => .object(ofGroup: "Object", withProperties: ["isNative", "lineNumber", "columnNumber", "fileName"])),

        // The methods below are disabled since they are not very interesting to fuzz
        // "HermesInternal.hasPromise"                   : .function([] => .boolean),
        // "HermesInternal.useEngineQueue"               : .function([] => .boolean),
        // "HermesInternal.ttrcReached"                  : .function([] => .undefined),
    ]
)
