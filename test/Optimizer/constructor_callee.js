/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-ir %s -fno-inline | %FileCheckOrRegen %s

"use strict";

function ctor_this_test() {
    function use_this(k) {
        this.k = k
        return this;
    }

    return new use_this(12)
}

function ctor_load_store_test() {
  function use_this(k) {
    this.k = k
  }

  function construct_use_this(){
    return new use_this(12)
  }

  return construct_use_this();
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global() : string
// CHECK-NEXT:frame = [], globals = [ctor_this_test, ctor_load_store_test]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %ctor_this_test() : object
// CHECK-NEXT:  %1 = StorePropertyStrictInst %0 : closure, globalObject : object, "ctor_this_test" : string
// CHECK-NEXT:  %2 = CreateFunctionInst %ctor_load_store_test() : object
// CHECK-NEXT:  %3 = StorePropertyStrictInst %2 : closure, globalObject : object, "ctor_load_store_test" : string
// CHECK-NEXT:  %4 = ReturnInst "use strict" : string
// CHECK-NEXT:function_end

// CHECK:function ctor_this_test() : object
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %use_this() : object
// CHECK-NEXT:  %1 = LoadPropertyInst %0 : closure, "prototype" : string
// CHECK-NEXT:  %2 = CreateThisInst %1, %0 : closure
// CHECK-NEXT:  %3 = ConstructInst %0 : closure, %2 : object, 12 : number
// CHECK-NEXT:  %4 = ReturnInst %3 : object
// CHECK-NEXT:function_end

// CHECK:function use_this(k : number) : object
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %this : object
// CHECK-NEXT:  %1 = StorePropertyStrictInst 12 : number, %0 : object, "k" : string
// CHECK-NEXT:  %2 = ReturnInst %0 : object
// CHECK-NEXT:function_end

// CHECK:function ctor_load_store_test() : object
// CHECK-NEXT:frame = [use_this : closure]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %"use_this 1#"() : undefined
// CHECK-NEXT:  %1 = StoreFrameInst %0 : closure, [use_this] : closure
// CHECK-NEXT:  %2 = CreateFunctionInst %construct_use_this() : object
// CHECK-NEXT:  %3 = CallInst %2 : closure, undefined : undefined
// CHECK-NEXT:  %4 = ReturnInst %3 : object
// CHECK-NEXT:function_end

// CHECK:function "use_this 1#"(k : number) : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %this : object
// CHECK-NEXT:  %1 = StorePropertyStrictInst 12 : number, %0 : object, "k" : string
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function construct_use_this() : object
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst [use_this@ctor_load_store_test] : closure
// CHECK-NEXT:  %1 = LoadPropertyInst %0 : closure, "prototype" : string
// CHECK-NEXT:  %2 = CreateThisInst %1, %0 : closure
// CHECK-NEXT:  %3 = ConstructInst %0 : closure, %2 : object, 12 : number
// CHECK-NEXT:  %4 = ReturnInst %2 : object
// CHECK-NEXT:function_end
