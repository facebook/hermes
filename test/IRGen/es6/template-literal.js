/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O -dump-ir %s | %FileCheckOrRegen --match-full-lines --check-prefix=CHKIR %s

function f1() {
  return `hello${1 + 1}world`;
}

function f2() {
  return `world`;
}

function f3() {
  return ``;
}

function f4() {
  return `${666}`;
}

function f5(x) {
  return `${x}`;
}

// Auto-generated content below. Please do not modify manually.

// CHKIR:function global#0()#1 : undefined
// CHKIR-NEXT:frame = [], globals = [f1, f2, f3, f4, f5]
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHKIR-NEXT:  %1 = CreateFunctionInst %f1#0#1()#2, %0
// CHKIR-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "f1" : string
// CHKIR-NEXT:  %3 = CreateFunctionInst %f2#0#1()#3 : string, %0
// CHKIR-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "f2" : string
// CHKIR-NEXT:  %5 = CreateFunctionInst %f3#0#1()#4 : string, %0
// CHKIR-NEXT:  %6 = StorePropertyInst %5 : closure, globalObject : object, "f3" : string
// CHKIR-NEXT:  %7 = CreateFunctionInst %f4#0#1()#5, %0
// CHKIR-NEXT:  %8 = StorePropertyInst %7 : closure, globalObject : object, "f4" : string
// CHKIR-NEXT:  %9 = CreateFunctionInst %f5#0#1()#6, %0
// CHKIR-NEXT:  %10 = StorePropertyInst %9 : closure, globalObject : object, "f5" : string
// CHKIR-NEXT:  %11 = ReturnInst undefined : undefined
// CHKIR-NEXT:function_end

// CHKIR:function f1#0#1()#2
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = CreateScopeInst %S{f1#0#1()#2}
// CHKIR-NEXT:  %1 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
// CHKIR-NEXT:  %2 = LoadPropertyInst %1, "concat" : string
// CHKIR-NEXT:  %3 = CallInst %2, "hello" : string, 2 : number, "world" : string
// CHKIR-NEXT:  %4 = ReturnInst %3
// CHKIR-NEXT:function_end

// CHKIR:function f2#0#1()#3 : string
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = CreateScopeInst %S{f2#0#1()#3}
// CHKIR-NEXT:  %1 = ReturnInst "world" : string
// CHKIR-NEXT:function_end

// CHKIR:function f3#0#1()#4 : string
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = CreateScopeInst %S{f3#0#1()#4}
// CHKIR-NEXT:  %1 = ReturnInst "" : string
// CHKIR-NEXT:function_end

// CHKIR:function f4#0#1()#5
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = CreateScopeInst %S{f4#0#1()#5}
// CHKIR-NEXT:  %1 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
// CHKIR-NEXT:  %2 = LoadPropertyInst %1, "concat" : string
// CHKIR-NEXT:  %3 = CallInst %2, "" : string, 666 : number
// CHKIR-NEXT:  %4 = ReturnInst %3
// CHKIR-NEXT:function_end

// CHKIR:function f5#0#1(x)#6
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = CreateScopeInst %S{f5#0#1()#6}
// CHKIR-NEXT:  %1 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
// CHKIR-NEXT:  %2 = LoadPropertyInst %1, "concat" : string
// CHKIR-NEXT:  %3 = CallInst %2, "" : string, %x
// CHKIR-NEXT:  %4 = ReturnInst %3
// CHKIR-NEXT:function_end
