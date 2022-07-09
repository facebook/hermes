/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheck %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O


//CHECK-LABEL:function update_field_test0(o)
//CHECK-NEXT:frame = [o]
//CHECK-NEXT:  %BB0:
//CHECK-NEXT:    %0 = StoreFrameInst %o, [o]
//CHECK-NEXT:    %1 = LoadFrameInst [o]
//CHECK-NEXT:    %2 = LoadPropertyInst %1, "f" : string
//CHECK-NEXT:    %3 = AsNumericInst %2
//CHECK-NEXT:    %4 = UnaryOperatorInst '++', %3 : number|bigint
//CHECK-NEXT:    %5 = StorePropertyInst %4, %1, "f" : string
//CHECK-NEXT:    %6 = ReturnInst %3 : number|bigint
//CHECK-NEXT:  %BB1:
//CHECK-NEXT:    %7 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function update_field_test0(o) { return o.f++; }

//CHECK-LABEL:function update_field_test1(o)
//CHECK-NEXT:frame = [o]
//CHECK-NEXT:  %BB0:
//CHECK-NEXT:    %0 = StoreFrameInst %o, [o]
//CHECK-NEXT:    %1 = LoadFrameInst [o]
//CHECK-NEXT:    %2 = LoadPropertyInst %1, "f" : string
//CHECK-NEXT:    %3 = AsNumericInst %2
//CHECK-NEXT:    %4 = UnaryOperatorInst '--', %3 : number|bigint
//CHECK-NEXT:    %5 = StorePropertyInst %4, %1, "f" : string
//CHECK-NEXT:    %6 = ReturnInst %3 : number|bigint
//CHECK-NEXT:  %BB1:
//CHECK-NEXT:    %7 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function update_field_test1(o) { return o.f--; }

//CHECK-LABEL:function update_field_test2(o)
//CHECK-NEXT:frame = [o]
//CHECK-NEXT:  %BB0:
//CHECK-NEXT:    %0 = StoreFrameInst %o, [o]
//CHECK-NEXT:    %1 = LoadFrameInst [o]
//CHECK-NEXT:    %2 = LoadPropertyInst %1, "f" : string
//CHECK-NEXT:    %3 = UnaryOperatorInst '++', %2
//CHECK-NEXT:    %4 = StorePropertyInst %3, %1, "f" : string
//CHECK-NEXT:    %5 = ReturnInst %3
//CHECK-NEXT:  %BB1:
//CHECK-NEXT:    %6 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function update_field_test2(o) { return ++o.f; }

//CHECK-LABEL:function update_field_test3(o)
//CHECK-NEXT:frame = [o]
//CHECK-NEXT:  %BB0:
//CHECK-NEXT:    %0 = StoreFrameInst %o, [o]
//CHECK-NEXT:    %1 = LoadFrameInst [o]
//CHECK-NEXT:    %2 = LoadPropertyInst %1, "f" : string
//CHECK-NEXT:    %3 = UnaryOperatorInst '--', %2
//CHECK-NEXT:    %4 = StorePropertyInst %3, %1, "f" : string
//CHECK-NEXT:    %5 = ReturnInst %3
//CHECK-NEXT:  %BB1:
//CHECK-NEXT:    %6 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function update_field_test3(o) { return --o.f; }

//CHECK-LABEL:function update_variable_test0(x)
//CHECK-NEXT:frame = [x]
//CHECK-NEXT:  %BB0:
//CHECK-NEXT:    %0 = StoreFrameInst %x, [x]
//CHECK-NEXT:    %1 = LoadFrameInst [x]
//CHECK-NEXT:    %2 = AsNumericInst %1
//CHECK-NEXT:    %3 = UnaryOperatorInst '++', %2 : number|bigint
//CHECK-NEXT:    %4 = StoreFrameInst %3, [x]
//CHECK-NEXT:    %5 = ReturnInst %2 : number|bigint
//CHECK-NEXT:  %BB1:
//CHECK-NEXT:    %6 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function update_variable_test0(x) { return x++; }

//CHECK-LABEL:function update_variable_test1(x)
//CHECK-NEXT:frame = [x]
//CHECK-NEXT:  %BB0:
//CHECK-NEXT:    %0 = StoreFrameInst %x, [x]
//CHECK-NEXT:    %1 = LoadFrameInst [x]
//CHECK-NEXT:    %2 = AsNumericInst %1
//CHECK-NEXT:    %3 = UnaryOperatorInst '--', %2 : number|bigint
//CHECK-NEXT:    %4 = StoreFrameInst %3, [x]
//CHECK-NEXT:    %5 = ReturnInst %2 : number|bigint
//CHECK-NEXT:  %BB1:
//CHECK-NEXT:    %6 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function update_variable_test1(x) { return x--; }

//CHECK-LABEL:function update_variable_test2(x)
//CHECK-NEXT:frame = [x]
//CHECK-NEXT:  %BB0:
//CHECK-NEXT:    %0 = StoreFrameInst %x, [x]
//CHECK-NEXT:    %1 = LoadFrameInst [x]
//CHECK-NEXT:    %2 = UnaryOperatorInst '++', %1
//CHECK-NEXT:    %3 = StoreFrameInst %2, [x]
//CHECK-NEXT:    %4 = ReturnInst %2
//CHECK-NEXT:  %BB1:
//CHECK-NEXT:    %5 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function update_variable_test2(x) { return ++x; }

//CHECK-LABEL:function update_variable_test3(x)
//CHECK-NEXT:frame = [x]
//CHECK-NEXT:  %BB0:
//CHECK-NEXT:    %0 = StoreFrameInst %x, [x]
//CHECK-NEXT:    %1 = LoadFrameInst [x]
//CHECK-NEXT:    %2 = UnaryOperatorInst '--', %1
//CHECK-NEXT:    %3 = StoreFrameInst %2, [x]
//CHECK-NEXT:    %4 = ReturnInst %2
//CHECK-NEXT:  %BB1:
//CHECK-NEXT:    %5 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function update_variable_test3(x) { return --x; }
