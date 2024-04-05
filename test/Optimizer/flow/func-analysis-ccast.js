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

// CHECK:scope %VS0 [exports: any, foo: any, bar: any]

// CHECK:scope %VS1 []

// CHECK:function bar(): any [allCallsitesKnownInStrictMode,unreachable]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:  %2 = ResolveScopeInst (:environment) %VS0: any, %1: environment
// CHECK-NEXT:  %3 = LoadFrameInst (:any) %2: environment, [%VS0.foo]: any
// CHECK-NEXT:  %4 = CheckedTypeCastInst (:object) %3: any, type(object)
// CHECK-NEXT:  %5 = CallInst [njsf] (:any) %4: object, %foo(): functionCode, %2: environment, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %6 = CheckedTypeCastInst (:undefined) %5: any, type(undefined)
// CHECK-NEXT:       ReturnInst %6: undefined
// CHECK-NEXT:function_end
