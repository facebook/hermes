/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermesc -hermes-parser -dump-ir %s -O

function simple_test0() {
  var re = /(\w+)\s(\w+)/;
  var str = 'John Smith';
  var newstr = str.replace(re, '$2, $1');
}

function simple_test1() {
  var re0 = /\w+\s/g;
  var re1 = /\w+\s/g;
  var re2 = /\w+/;
  var re3 = /\w+/g;
  var re4 = / /;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "simple_test0": string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "simple_test1": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %simple_test0(): any
// CHECK-NEXT:  %3 = StorePropertyLooseInst %2: object, globalObject: object, "simple_test0": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %simple_test1(): any
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4: object, globalObject: object, "simple_test1": string
// CHECK-NEXT:  %6 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %7 = StoreStackInst undefined: undefined, %6: any
// CHECK-NEXT:  %8 = LoadStackInst (:any) %6: any
// CHECK-NEXT:  %9 = ReturnInst %8: any
// CHECK-NEXT:function_end

// CHECK:function simple_test0(): any
// CHECK-NEXT:frame = [re: any, str: any, newstr: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined: undefined, [re]: any
// CHECK-NEXT:  %1 = StoreFrameInst undefined: undefined, [str]: any
// CHECK-NEXT:  %2 = StoreFrameInst undefined: undefined, [newstr]: any
// CHECK-NEXT:  %3 = CreateRegExpInst (:object) "(\\\\w+)\\\\s(\\\\w+)": string, "": string
// CHECK-NEXT:  %4 = StoreFrameInst %3: object, [re]: any
// CHECK-NEXT:  %5 = StoreFrameInst "John Smith": string, [str]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) [str]: any
// CHECK-NEXT:  %7 = LoadPropertyInst (:any) %6: any, "replace": string
// CHECK-NEXT:  %8 = LoadFrameInst (:any) [re]: any
// CHECK-NEXT:  %9 = CallInst (:any) %7: any, empty: any, empty: any, undefined: undefined, %6: any, %8: any, "$2, $1": string
// CHECK-NEXT:  %10 = StoreFrameInst %9: any, [newstr]: any
// CHECK-NEXT:  %11 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function simple_test1(): any
// CHECK-NEXT:frame = [re0: any, re1: any, re2: any, re3: any, re4: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined: undefined, [re0]: any
// CHECK-NEXT:  %1 = StoreFrameInst undefined: undefined, [re1]: any
// CHECK-NEXT:  %2 = StoreFrameInst undefined: undefined, [re2]: any
// CHECK-NEXT:  %3 = StoreFrameInst undefined: undefined, [re3]: any
// CHECK-NEXT:  %4 = StoreFrameInst undefined: undefined, [re4]: any
// CHECK-NEXT:  %5 = CreateRegExpInst (:object) "\\\\w+\\\\s": string, "g": string
// CHECK-NEXT:  %6 = StoreFrameInst %5: object, [re0]: any
// CHECK-NEXT:  %7 = CreateRegExpInst (:object) "\\\\w+\\\\s": string, "g": string
// CHECK-NEXT:  %8 = StoreFrameInst %7: object, [re1]: any
// CHECK-NEXT:  %9 = CreateRegExpInst (:object) "\\\\w+": string, "": string
// CHECK-NEXT:  %10 = StoreFrameInst %9: object, [re2]: any
// CHECK-NEXT:  %11 = CreateRegExpInst (:object) "\\\\w+": string, "g": string
// CHECK-NEXT:  %12 = StoreFrameInst %11: object, [re3]: any
// CHECK-NEXT:  %13 = CreateRegExpInst (:object) " ": string, "": string
// CHECK-NEXT:  %14 = StoreFrameInst %13: object, [re4]: any
// CHECK-NEXT:  %15 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end
