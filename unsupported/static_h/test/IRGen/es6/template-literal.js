/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O -dump-ir %s | %FileCheck --match-full-lines --check-prefix=CHKIR %s

function f1() {
  return `hello${1 + 1}world`;
}
//CHKIR-LABEL:function f1()
//CHKIR-NEXT:frame = []
//CHKIR-NEXT:%BB0:
//CHKIR-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHKIR-NEXT:  %1 = LoadPropertyInst %0, "concat" : string
//CHKIR-NEXT:  %2 = CallInst %1, "hello" : string, 2 : number, "world" : string
//CHKIR-NEXT:  %3 = ReturnInst %2
//CHKIR-NEXT:function_end

function f2() {
  return `world`;
}
//CHKIR-LABEL:function f2() : string
//CHKIR-NEXT:frame = []
//CHKIR-NEXT:%BB0:
//CHKIR-NEXT:  %0 = ReturnInst "world" : string
//CHKIR-NEXT:function_end

function f3() {
  return ``;
}
//CHKIR-LABEL:function f3() : string
//CHKIR-NEXT:frame = []
//CHKIR-NEXT:%BB0:
//CHKIR-NEXT:  %0 = ReturnInst "" : string
//CHKIR-NEXT:function_end

function f4() {
  return `${666}`;
}
//CHKIR-LABEL:function f4()
//CHKIR-NEXT:frame = []
//CHKIR-NEXT:%BB0:
//CHKIR-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHKIR-NEXT:  %1 = LoadPropertyInst %0, "concat" : string
//CHKIR-NEXT:  %2 = CallInst %1, "" : string, 666 : number
//CHKIR-NEXT:  %3 = ReturnInst %2
//CHKIR-NEXT:function_end

function f5(x) {
  return `${x}`;
}
//CHKIR-LABEL:function f5(x)
//CHKIR-NEXT:frame = []
//CHKIR-NEXT:%BB0:
//CHKIR-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHKIR-NEXT:  %1 = LoadPropertyInst %0, "concat" : string
//CHKIR-NEXT:  %2 = CallInst %1, "" : string, %x
//CHKIR-NEXT:  %3 = ReturnInst %2
//CHKIR-NEXT:function_end
