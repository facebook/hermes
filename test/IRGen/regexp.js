/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O

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

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [simple_test0, simple_test1]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %simple_test0#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "simple_test0" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %simple_test1#0#1()#3, %0
// CHECK-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "simple_test1" : string
// CHECK-NEXT:  %5 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %6 = StoreStackInst undefined : undefined, %5
// CHECK-NEXT:  %7 = LoadStackInst %5
// CHECK-NEXT:  %8 = ReturnInst %7
// CHECK-NEXT:function_end

// CHECK:function simple_test0#0#1()#2
// CHECK-NEXT:frame = [re#2, str#2, newstr#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{simple_test0#0#1()#2}
// CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [re#2], %0
// CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [str#2], %0
// CHECK-NEXT:  %3 = StoreFrameInst undefined : undefined, [newstr#2], %0
// CHECK-NEXT:  %4 = CreateRegExpInst "(\\\\w+)\\\\s(\\\\w+)" : string, "" : string
// CHECK-NEXT:  %5 = StoreFrameInst %4 : regexp, [re#2], %0
// CHECK-NEXT:  %6 = StoreFrameInst "John Smith" : string, [str#2], %0
// CHECK-NEXT:  %7 = LoadFrameInst [str#2], %0
// CHECK-NEXT:  %8 = LoadPropertyInst %7, "replace" : string
// CHECK-NEXT:  %9 = LoadFrameInst [re#2], %0
// CHECK-NEXT:  %10 = CallInst %8, %7, %9, "$2, $1" : string
// CHECK-NEXT:  %11 = StoreFrameInst %10, [newstr#2], %0
// CHECK-NEXT:  %12 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function simple_test1#0#1()#3
// CHECK-NEXT:frame = [re0#3, re1#3, re2#3, re3#3, re4#3]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{simple_test1#0#1()#3}
// CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [re0#3], %0
// CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [re1#3], %0
// CHECK-NEXT:  %3 = StoreFrameInst undefined : undefined, [re2#3], %0
// CHECK-NEXT:  %4 = StoreFrameInst undefined : undefined, [re3#3], %0
// CHECK-NEXT:  %5 = StoreFrameInst undefined : undefined, [re4#3], %0
// CHECK-NEXT:  %6 = CreateRegExpInst "\\\\w+\\\\s" : string, "g" : string
// CHECK-NEXT:  %7 = StoreFrameInst %6 : regexp, [re0#3], %0
// CHECK-NEXT:  %8 = CreateRegExpInst "\\\\w+\\\\s" : string, "g" : string
// CHECK-NEXT:  %9 = StoreFrameInst %8 : regexp, [re1#3], %0
// CHECK-NEXT:  %10 = CreateRegExpInst "\\\\w+" : string, "" : string
// CHECK-NEXT:  %11 = StoreFrameInst %10 : regexp, [re2#3], %0
// CHECK-NEXT:  %12 = CreateRegExpInst "\\\\w+" : string, "g" : string
// CHECK-NEXT:  %13 = StoreFrameInst %12 : regexp, [re3#3], %0
// CHECK-NEXT:  %14 = CreateRegExpInst " " : string, "" : string
// CHECK-NEXT:  %15 = StoreFrameInst %14 : regexp, [re4#3], %0
// CHECK-NEXT:  %16 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
