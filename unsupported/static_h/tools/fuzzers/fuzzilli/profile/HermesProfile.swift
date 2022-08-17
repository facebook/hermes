/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

import Fuzzilli

let hermesProfile = Profile(
    processArguments: ["--replr"],

    processEnv: ["UBSAN_OPTIONS": "handle_segv=0"],

    codePrefix: """
                function main(){
                """,

    codeSuffix: """
                }; main();
                """,

    ecmaVersion: ECMAScriptVersion.es6,

    crashTests: ["fuzzilli('FUZZILLI_CRASH', 0)", "fuzzilli('FUZZILLI_CRASH', 1)"],

    additionalCodeGenerators: WeightedList<CodeGenerator>([]),

    additionalProgramTemplates: WeightedList<ProgramTemplate>([]),

    disabledCodeGenerators: [],

    additionalBuiltins: [
        "gc"                : .function([] => .undefined),
        "print"             : .function([] => .undefined),
    ]
)
