/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 -hermes-parser -dump-ir %s -non-strict 2>&1 | %FileCheck %s --match-full-lines

//CHECK-LABEL: function one()
//CHECK-NEXT: frame = []
//CHECK-NEXT:   %BB0:
//CHECK-NEXT:     %0 = TryLoadGlobalPropertyInst globalObject : object, "s" : string
//CHECK-NEXT:     %1 = ReturnInst %0
//CHECK-NEXT:   %BB1:
//CHECK-NEXT:     %2 = TryLoadGlobalPropertyInst globalObject : object, "s" : string
//CHECK-NEXT:     %3 = ReturnInst %2
//CHECK-NEXT:   %BB2:
//CHECK-NEXT:     %4 = ReturnInst undefined : undefined
//CHECK-NEXT: function_end
function one() { return s; return s; }

//CHECK-LABEL: function two()
//CHECK-NEXT: frame = []
//CHECK-NEXT:   %BB0:
//CHECK-NEXT:     %0 = TryLoadGlobalPropertyInst globalObject : object, "s" : string
//CHECK-NEXT:     %1 = ReturnInst %0
//CHECK-NEXT:   %BB1:
//CHECK-NEXT:     %2 = TryLoadGlobalPropertyInst globalObject : object, "t" : string
//CHECK-NEXT:     %3 = ReturnInst %2
//CHECK-NEXT:   %BB2:
//CHECK-NEXT:     %4 = ReturnInst undefined : undefined
//CHECK-NEXT: function_end
function two() { return s; return t;}

//CHECK-LABEL: function three()
//CHECK-NEXT: frame = []
//CHECK-NEXT:   %BB0:
//CHECK-NEXT:     %0 = TryLoadGlobalPropertyInst globalObject : object, "z" : string
//CHECK-NEXT:     %1 = ReturnInst %0
//CHECK-NEXT:   %BB1:
//CHECK-NEXT:     %2 = TryLoadGlobalPropertyInst globalObject : object, "z" : string
//CHECK-NEXT:     %3 = ReturnInst %2
//CHECK-NEXT:   %BB2:
//CHECK-NEXT:     %4 = ReturnInst undefined : undefined
//CHECK-NEXT: function_end
function three() { return z; return z;}
