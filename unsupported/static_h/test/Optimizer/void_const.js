/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s     -O | %FileCheck %s --match-full-lines

//CHECK-LABEL:function global()
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:

//CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
//CHECK-NEXT:  %1 = CallInst %0, undefined : undefined, undefined : undefined
print(void 0);

//CHECK-NEXT:  %2 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
//CHECK-NEXT:  %3 = CallInst %2, undefined : undefined, undefined : undefined
print(void "x");

//CHECK-NEXT:  %4 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
//CHECK-NEXT:  %5 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
//CHECK-NEXT:  %6 = UnaryOperatorInst 'void', %5
//CHECK-NEXT:  %7 = CallInst %4, undefined : undefined, %6 : undefined
print(void print);

//CHECK-NEXT:  %8 = ReturnInst %7
//CHECK-NEXT:function_end

