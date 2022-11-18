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

// CHKIR:function global() : undefined
// CHKIR-NEXT:frame = [], globals = [f1, f2, f3, f4, f5]
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = CreateFunctionInst %f1()
// CHKIR-NEXT:  %1 = StorePropertyLooseInst %0 : closure, globalObject : object, "f1" : string
// CHKIR-NEXT:  %2 = CreateFunctionInst %f2() : string
// CHKIR-NEXT:  %3 = StorePropertyLooseInst %2 : closure, globalObject : object, "f2" : string
// CHKIR-NEXT:  %4 = CreateFunctionInst %f3() : string
// CHKIR-NEXT:  %5 = StorePropertyLooseInst %4 : closure, globalObject : object, "f3" : string
// CHKIR-NEXT:  %6 = CreateFunctionInst %f4()
// CHKIR-NEXT:  %7 = StorePropertyLooseInst %6 : closure, globalObject : object, "f4" : string
// CHKIR-NEXT:  %8 = CreateFunctionInst %f5()
// CHKIR-NEXT:  %9 = StorePropertyLooseInst %8 : closure, globalObject : object, "f5" : string
// CHKIR-NEXT:  %10 = ReturnInst undefined : undefined
// CHKIR-NEXT:function_end

// CHKIR:function f1()
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
// CHKIR-NEXT:  %1 = LoadPropertyInst %0, "concat" : string
// CHKIR-NEXT:  %2 = CallInst %1, "hello" : string, 2 : number, "world" : string
// CHKIR-NEXT:  %3 = ReturnInst %2
// CHKIR-NEXT:function_end

// CHKIR:function f2() : string
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = ReturnInst "world" : string
// CHKIR-NEXT:function_end

// CHKIR:function f3() : string
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = ReturnInst "" : string
// CHKIR-NEXT:function_end

// CHKIR:function f4()
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
// CHKIR-NEXT:  %1 = LoadPropertyInst %0, "concat" : string
// CHKIR-NEXT:  %2 = CallInst %1, "" : string, 666 : number
// CHKIR-NEXT:  %3 = ReturnInst %2
// CHKIR-NEXT:function_end

// CHKIR:function f5(x)
// CHKIR-NEXT:frame = []
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
// CHKIR-NEXT:  %1 = LoadPropertyInst %0, "concat" : string
// CHKIR-NEXT:  %2 = CallInst %1, "" : string, %x
// CHKIR-NEXT:  %3 = ReturnInst %2
// CHKIR-NEXT:function_end
