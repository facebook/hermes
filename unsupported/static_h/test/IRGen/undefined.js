/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheck --match-full-lines %s
// RUN: %hermes -hermes-parser -dump-ir %s -O

//CHECK-LABEL:function global()
//CHECK-NEXT:frame = [], globals = [foo]
//CHECK-NEXT:  %BB0:
//CHECK-NEXT:    %0 = CreateFunctionInst %foo()
//CHECK-NEXT:    %1 = StorePropertyInst %0 : closure, globalObject : object, "foo" : string
//CHECK-NEXT:    %2 = AllocStackInst $?anon_0_ret
//CHECK-NEXT:    %3 = StoreStackInst undefined : undefined, %2
//CHECK-NEXT:    %4 = StorePropertyInst 5 : number, globalObject : object, "undefined" : string
//CHECK-NEXT:    %5 = LoadPropertyInst globalObject : object, "foo" : string
//CHECK-NEXT:    %6 = CallInst %5, undefined : undefined
//CHECK-NEXT:    %7 = StoreStackInst %6, %2
//CHECK-NEXT:    %8 = StoreStackInst undefined : undefined, %2
//CHECK-NEXT:    %9 = LoadStackInst %2
//CHECK-NEXT:    %10 = ReturnInst %9
//CHECK-NEXT:function_end

//CHECK-LABEL:function foo()
//CHECK-NEXT:frame = [undefined]
//CHECK-NEXT:  %BB0:
//CHECK-NEXT:    %0 = StoreFrameInst undefined : undefined, [undefined]
//CHECK-NEXT:    %1 = StoreFrameInst 5 : number, [undefined]
//CHECK-NEXT:    %2 = LoadFrameInst [undefined]
//CHECK-NEXT:    %3 = ReturnInst %2
//CHECK-NEXT:  %BB1:
//CHECK-NEXT:    %4 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end

function foo() {
  var undefined = 5;
  return undefined;
}

var undefined = 5;
foo();
undefined;
