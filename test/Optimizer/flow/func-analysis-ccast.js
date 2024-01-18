/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -typed -dump-ir -Xdump-functions=bar -Xcustom-opt=frameloadstoreopts,functionanalysis %s | %FileCheckOrRegen %s --match-full-lines

// Ensure that FunctionAnalysis needs to follow checked casts.
// bar() below uses a checked cast before calling foo().
// The IR output we are checking shows the cast and that CallInst instruction has the
// correct target.

var foo: any = function () {}

function bar() {
    return (foo as () => void)();
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function bar(): any [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst (:any) [foo@""]: any
// CHECK-NEXT:  %1 = CheckedTypeCastInst (:object) %0: any, type(object)
// CHECK-NEXT:  %2 = CallInst [njsf] (:any) %1: object, %foo(): functionCode, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %3 = CheckedTypeCastInst (:undefined) %2: any, type(undefined)
// CHECK-NEXT:       ReturnInst %3: undefined
// CHECK-NEXT:function_end
