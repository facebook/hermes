/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -dump-ir -O -fno-inline -non-strict %s | %FileCheck --match-full-lines %s
//
// Ensure that Hermes-specific optimizations (parameter type inference) are performed in non-strict
// mode. We need to disable inlining because it inlines foo() completely and we can't see the
// inference.

function main()  {
  function foo(p1) {
    return "value" + p1;
  }
  foo(2)
}

//CHECK-LABEL: function global() : undefined
//CHECK-NEXT: frame = [], globals = [main]
//CHECK-NEXT: %BB0:
//CHECK-NEXT:   %0 = CreateFunctionInst %main() : undefined
//CHECK-NEXT:   %1 = StorePropertyInst %0 : closure, globalObject : object, "main" : string
//CHECK-NEXT:   %2 = ReturnInst undefined : undefined
//CHECK-NEXT: function_end

//CHECK-LABEL: function main() : undefined
//CHECK-NEXT: frame = []
//CHECK-NEXT: %BB0:
//CHECK-NEXT:   %0 = CreateFunctionInst %foo() : string
//CHECK-NEXT:   %1 = CallInst %0 : closure, undefined : undefined, 2 : number
//CHECK-NEXT:   %2 = ReturnInst undefined : undefined
//CHECK-NEXT: function_end

//CHECK-LABEL: function foo(p1 : number) : string
//CHECK-NEXT: frame = []
//CHECK-NEXT: %BB0:
//CHECK-NEXT:   %0 = BinaryOperatorInst '+', "value" : string, 2 : number
//CHECK-NEXT:   %1 = ReturnInst %0 : string
//CHECK-NEXT: function_end
