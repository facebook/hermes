// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hermesc -O -target=HBC -dump-ir %s | %FileCheck --match-full-lines --check-prefix=CHKIR %s

function dummy() {
  return;
}
function emptyString() {
  return dummy``;
}
//CHKIR-LABEL: function emptyString()
//CHKIR-NEXT: frame = []
//CHKIR-NEXT: %BB0:
//CHKIR-NEXT:   %0 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHKIR-NEXT:   %1 = LoadPropertyInst %0, "getTemplateObject" : string
//CHKIR-NEXT:   %2 = CallInst %1, undefined : undefined, 0 : number, true : boolean, "" : string
//CHKIR-NEXT:   %3 = LoadPropertyInst globalObject : object, "dummy" : string
//CHKIR-NEXT:   %4 = CallInst %3, undefined : undefined, %2
//CHKIR-NEXT:   %5 = ReturnInst %4
//CHKIR-NEXT: function_end

function oneString() {
  return dummy`hello`;
}

//CHKIR-LABEL:function oneString()
//CHKIR-NEXT:frame = []
//CHKIR-NEXT:%BB0:
//CHKIR-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHKIR-NEXT:  %1 = LoadPropertyInst %0, "getTemplateObject" : string
//CHKIR-NEXT:  %2 = CallInst %1, undefined : undefined, 1 : number, true : boolean, "hello" : string
//CHKIR-NEXT:  %3 = LoadPropertyInst globalObject : object, "dummy" : string
//CHKIR-NEXT:  %4 = CallInst %3, undefined : undefined, %2
//CHKIR-NEXT:  %5 = ReturnInst %4
//CHKIR-NEXT:function_end

function oneSub() {
  return dummy`${666}`;
}

//CHKIR-LABEL:function oneSub()
//CHKIR-NEXT:frame = []
//CHKIR-NEXT:%BB0:
//CHKIR-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHKIR-NEXT:  %1 = LoadPropertyInst %0, "getTemplateObject" : string
//CHKIR-NEXT:  %2 = CallInst %1, undefined : undefined, 2 : number, true : boolean, "" : string, "" : string
//CHKIR-NEXT:  %3 = LoadPropertyInst globalObject : object, "dummy" : string
//CHKIR-NEXT:  %4 = CallInst %3, undefined : undefined, %2, 666 : number
//CHKIR-NEXT:  %5 = ReturnInst %4
//CHKIR-NEXT:function_end

function dup() {
  return dummy`hello world${1 + 2}!!!`;
}

//CHKIR-LABEL:function dup()
//CHKIR-NEXT:frame = []
//CHKIR-NEXT:%BB0:
//CHKIR-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHKIR-NEXT:  %1 = LoadPropertyInst %0, "getTemplateObject" : string
//CHKIR-NEXT:  %2 = CallInst %1, undefined : undefined, 3 : number, true : boolean, "hello world" : string, "!!!" : string
//CHKIR-NEXT:  %3 = LoadPropertyInst globalObject : object, "dummy" : string
//CHKIR-NEXT:  %4 = CallInst %3, undefined : undefined, %2, 3 : number
//CHKIR-NEXT:  %5 = ReturnInst %4
//CHKIR-NEXT:function_end

function notDup() {
  return dummy`hello\nworld${1 + 2}!!!`;
}

//CHKIR-LABEL:function notDup()
//CHKIR-NEXT:frame = []
//CHKIR-NEXT:%BB0:
//CHKIR-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHKIR-NEXT:  %1 = LoadPropertyInst %0, "getTemplateObject" : string
//CHKIR-NEXT:  %2 = CallInst %1, undefined : undefined, 4 : number, false : boolean, "hello\\\\nworld" : string, "!!!" : string, "hello\\nworld" : string, "!!!" : string
//CHKIR-NEXT:  %3 = LoadPropertyInst globalObject : object, "dummy" : string
//CHKIR-NEXT:  %4 = CallInst %3, undefined : undefined, %2, 3 : number
//CHKIR-NEXT:  %5 = ReturnInst %4
//CHKIR-NEXT:function_end

function memberExpr() {
  var obj = {func: dummy};
  return obj.func`hello world!`;
}

//CHKIR-LABEL: function memberExpr()
//CHKIR-NEXT: frame = []
//CHKIR-NEXT: %BB0:
//CHKIR-NEXT:   %0 = AllocObjectInst 1 : number, empty
//CHKIR-NEXT:   %1 = LoadPropertyInst globalObject : object, "dummy" : string
//CHKIR-NEXT:   %2 = StoreNewOwnPropertyInst %1, %0 : object, "func" : string, true : boolean
//CHKIR-NEXT:   %3 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHKIR-NEXT:   %4 = LoadPropertyInst %3, "getTemplateObject" : string
//CHKIR-NEXT:   %5 = CallInst %4, undefined : undefined, 5 : number, true : boolean, "hello world!" : string
//CHKIR-NEXT:   %6 = LoadPropertyInst %0 : object, "func" : string
//CHKIR-NEXT:   %7 = CallInst %6, %0 : object, %5
//CHKIR-NEXT:   %8 = ReturnInst %7
//CHKIR-NEXT: function_end

function callExpr() {
  function func() {
    return function() {
      return;
    }
  }
  return func()`hello world!`;
}

//CHKIR-LABEL: function callExpr()
//CHKIR-NEXT: frame = []
//CHKIR-NEXT: %BB0:
//CHKIR-NEXT:   %0 = CreateFunctionInst %func() : closure
//CHKIR-NEXT:   %1 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHKIR-NEXT:   %2 = LoadPropertyInst %1, "getTemplateObject" : string
//CHKIR-NEXT:   %3 = CallInst %2, undefined : undefined, 5 : number, true : boolean, "hello world!" : string
//CHKIR-NEXT:   %4 = CallInst %0 : closure, undefined : undefined
//CHKIR-NEXT:   %5 = CallInst %4 : closure, undefined : undefined, %3
//CHKIR-NEXT:   %6 = ReturnInst %5
//CHKIR-NEXT: function_end

/// Some more test cases to check template object id.

function dup2() {
  return dummy`hello world${1 + 2}!!!`;
}

//CHKIR-LABEL: function dup2()
//CHKIR-NEXT: frame = []
//CHKIR-NEXT: %BB0:
//CHKIR-NEXT:   %0 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHKIR-NEXT:   %1 = LoadPropertyInst %0, "getTemplateObject" : string
//CHKIR-NEXT:   %2 = CallInst %1, undefined : undefined, 3 : number, true : boolean, "hello world" : string, "!!!" : string
//CHKIR-NEXT:   %3 = LoadPropertyInst globalObject : object, "dummy" : string
//CHKIR-NEXT:   %4 = CallInst %3, undefined : undefined, %2, 3 : number
//CHKIR-NEXT:   %5 = ReturnInst %4
//CHKIR-NEXT: function_end

function dup3() {
  return dummy`hello world${7}!!!`;
}

//CHKIR-LABEL: function dup3()
//CHKIR-NEXT: frame = []
//CHKIR-NEXT: %BB0:
//CHKIR-NEXT:   %0 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHKIR-NEXT:   %1 = LoadPropertyInst %0, "getTemplateObject" : string
//CHKIR-NEXT:   %2 = CallInst %1, undefined : undefined, 3 : number, true : boolean, "hello world" : string, "!!!" : string
//CHKIR-NEXT:   %3 = LoadPropertyInst globalObject : object, "dummy" : string
//CHKIR-NEXT:   %4 = CallInst %3, undefined : undefined, %2, 7 : number
//CHKIR-NEXT:   %5 = ReturnInst %4
//CHKIR-NEXT: function_end

function helloWorld() {
  return dummy`hello${0} world!!!`;
}

//CHKIR-LABEL: function helloWorld()
//CHKIR-NEXT: frame = []
//CHKIR-NEXT: %BB0:
//CHKIR-NEXT:   %0 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHKIR-NEXT:   %1 = LoadPropertyInst %0, "getTemplateObject" : string
//CHKIR-NEXT:   %2 = CallInst %1, undefined : undefined, 6 : number, true : boolean, "hello" : string, " world!!!" : string
//CHKIR-NEXT:   %3 = LoadPropertyInst globalObject : object, "dummy" : string
//CHKIR-NEXT:   %4 = CallInst %3, undefined : undefined, %2, 0 : number
//CHKIR-NEXT:   %5 = ReturnInst %4
//CHKIR-NEXT: function_end
