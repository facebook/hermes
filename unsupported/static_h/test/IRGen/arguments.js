/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir -non-strict %s -O0 | %FileCheck %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir -non-strict %s -O

// Code that doesn't require creation of the expensive object.
function cheap() {
    return arguments.length + arguments[0];
}
//CHECK: function cheap()
//CHECK-NEXT: frame = []
//CHECK-NEXT:   %BB0:
//CHECK-NEXT:     %0 = CreateArgumentsInst
//CHECK-NEXT:     %1 = LoadPropertyInst %0 : object, "length" : string
//CHECK-NEXT:     %2 = LoadPropertyInst %0 : object, 0 : number
//CHECK-NEXT:     %3 = BinaryOperatorInst '+', %1, %2
//CHECK-NEXT:     %4 = ReturnInst %3
//CHECK-NEXT:   %BB1:
//CHECK-NEXT:     %5 = ReturnInst undefined : undefined
//CHECK-NEXT: function_end

// Code that requires creation of the expensive object.
function expensive() {
    return arguments.length + cheap(arguments) + arguments[0];
}
//CHECK: function expensive()
//CHECK-NEXT: frame = []
//CHECK-NEXT:   %BB0:
//CHECK-NEXT:     %0 = CreateArgumentsInst
//CHECK-NEXT:     %1 = LoadPropertyInst %0 : object, "length" : string
//CHECK-NEXT:     %2 = LoadPropertyInst globalObject : object, "cheap" : string
//CHECK-NEXT:     %3 = CallInst %2, undefined : undefined, %0 : object
//CHECK-NEXT:     %4 = BinaryOperatorInst '+', %1, %3
//CHECK-NEXT:     %5 = LoadPropertyInst %0 : object, 0 : number
//CHECK-NEXT:     %6 = BinaryOperatorInst '+', %4, %5
//CHECK-NEXT:     %7 = ReturnInst %6
//CHECK-NEXT:   %BB1:
//CHECK-NEXT:     %8 = ReturnInst undefined : undefined
//CHECK-NEXT: function_end

// Something with more than one BB
function cond() {
    if (arguments.length)
        return arguments[1];
}
//CHECK: function cond()
//CHECK-NEXT: frame = []
//CHECK-NEXT:   %BB0:
//CHECK-NEXT:     %0 = CreateArgumentsInst
//CHECK-NEXT:     %1 = LoadPropertyInst %0 : object, "length" : string
//CHECK-NEXT:     %2 = CondBranchInst %1, %BB1, %BB2
//CHECK-NEXT:   %BB1:
//CHECK-NEXT:     %3 = LoadPropertyInst %0 : object, 1 : number
//CHECK-NEXT:     %4 = ReturnInst %3
//CHECK-NEXT:   %BB2:
//CHECK-NEXT:     %5 = BranchInst %BB3
//CHECK-NEXT:   %BB3:
//CHECK-NEXT:     %6 = ReturnInst undefined : undefined
//CHECK-NEXT:   %BB4:
//CHECK-NEXT:     %7 = BranchInst %BB3
//CHECK-NEXT: function_end

function shadow(arguments) {
    return arguments.length;
}
//CHECK-LABEL:function shadow(arguments)
//CHECK-NEXT:frame = [arguments]
//CHECK-NEXT:  %BB0:
//CHECK-NEXT:    %0 = StoreFrameInst %arguments, [arguments]
//CHECK-NEXT:    %1 = LoadFrameInst [arguments]
//CHECK-NEXT:    %2 = LoadPropertyInst %1, "length" : string
//CHECK-NEXT:    %3 = ReturnInst %2
//CHECK-NEXT:  %BB1:
//CHECK-NEXT:    %4 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end

