/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheck %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O

function sink() {}
var x;

//CHECK-LABEL:function delete_parameter(p)
//CHECK-NEXT:frame = [p]
//CHECK-NEXT:  %BB0:
//CHECK-NEXT:    %0 = StoreFrameInst %p, [p]
//CHECK-NEXT:    %1 = ReturnInst false : boolean
//CHECK-NEXT:  %BB1:
//CHECK-NEXT:    %2 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function delete_parameter(p) {
  return (delete p);
}

//CHECK-LABEL:function delete_literal()
//CHECK-NEXT:frame = []
//CHECK-NEXT:  %BB0:
//CHECK-NEXT:    %0 = ReturnInst true : boolean
//CHECK-NEXT:  %BB1:
//CHECK-NEXT:    %1 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function delete_literal() {
  return (delete 4);
}

//CHECK-LABEL:function delete_variable()
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = DeletePropertyInst globalObject : object, "x" : string
//CHECK-NEXT:  %1 = ReturnInst %0
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %2 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function delete_variable() {
  return (delete x);
}

//CHECK-LABEL:function delete_expr()
//CHECK-NEXT:frame = []
//CHECK-NEXT:  %BB0:
//CHECK-NEXT:    %0 = LoadPropertyInst globalObject : object, "sink" : string
//CHECK-NEXT:    %1 = CallInst %0, undefined : undefined
//CHECK-NEXT:    %2 = ReturnInst true : boolean
//CHECK-NEXT:  %BB1:
//CHECK-NEXT:    %3 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function delete_expr() {
  return (delete sink());
}

