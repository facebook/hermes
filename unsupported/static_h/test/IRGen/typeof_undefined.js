/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 -hermes-parser -dump-ir -strict %s 2>&1 | %FileCheck %s --match-full-lines

var x = typeof foo;


//CHECK:      {{.*}}typeof_undefined.js:10:16: warning: the variable "foo" was not declared in function "global"

//CHECK-NEXT: var x = typeof foo;
//CHECK-NEXT:                ^~~

//CHECK-NEXT: function global()
//CHECK-NEXT: frame = [], globals = [x]
//CHECK-NEXT:   %BB0:
//CHECK-NEXT:     %0 = AllocStackInst $?anon_0_ret
//CHECK-NEXT:     %1 = StoreStackInst undefined : undefined, %0
//CHECK-NEXT:     %2 = LoadPropertyInst globalObject : object, "foo" : string
//CHECK-NEXT:     %3 = UnaryOperatorInst 'typeof', %2
//CHECK-NEXT:     %4 = StorePropertyInst %3, globalObject : object, "x" : string
//CHECK-NEXT:     %5 = LoadStackInst %0
//CHECK-NEXT:     %6 = ReturnInst %5
//CHECK-NEXT: function_end
