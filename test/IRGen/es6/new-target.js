/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

function func1() {
    return new.target;
}

function func2(a) {
    if (a)
        return new.target;
    print(new.target !== undefined);
}

function func3() {
    print(new.target !== undefined);
    function innerFunction() {
        return new.target;
    }
    var innerArrow1 = () => {
        print(new.target !== undefined);
        var innerArrow2 = () => { return new.target;}
        return innerArrow2();
    }

    return [innerFunction, innerArrow1];
}

function func4() {
    return new.target.prototype;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global()
// CHECK-NEXT:frame = [], globals = [func1, func2, func3, func4]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %func1()
// CHECK-NEXT:  %1 = StorePropertyLooseInst %0 : closure, globalObject : object, "func1" : string
// CHECK-NEXT:  %2 = CreateFunctionInst %func2()
// CHECK-NEXT:  %3 = StorePropertyLooseInst %2 : closure, globalObject : object, "func2" : string
// CHECK-NEXT:  %4 = CreateFunctionInst %func3()
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4 : closure, globalObject : object, "func3" : string
// CHECK-NEXT:  %6 = CreateFunctionInst %func4()
// CHECK-NEXT:  %7 = StorePropertyLooseInst %6 : closure, globalObject : object, "func4" : string
// CHECK-NEXT:  %8 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %9 = StoreStackInst undefined : undefined, %8
// CHECK-NEXT:  %10 = LoadStackInst %8
// CHECK-NEXT:  %11 = ReturnInst %10
// CHECK-NEXT:function_end

// CHECK:function func1()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetNewTargetInst
// CHECK-NEXT:  %1 = ReturnInst %0
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function func2(a)
// CHECK-NEXT:frame = [a]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %a, [a]
// CHECK-NEXT:  %1 = LoadFrameInst [a]
// CHECK-NEXT:  %2 = CondBranchInst %1, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = GetNewTargetInst
// CHECK-NEXT:  %4 = ReturnInst %3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %6 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %7 = GetNewTargetInst
// CHECK-NEXT:  %8 = BinaryOperatorInst '!==', %7, undefined : undefined
// CHECK-NEXT:  %9 = CallInst %6, undefined : undefined, %8
// CHECK-NEXT:  %10 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %11 = BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:function func3()
// CHECK-NEXT:frame = [innerArrow1, innerFunction, ?anon_0_this, ?anon_1_new.target]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [innerArrow1]
// CHECK-NEXT:  %1 = StoreFrameInst %this, [?anon_0_this]
// CHECK-NEXT:  %2 = GetNewTargetInst
// CHECK-NEXT:  %3 = StoreFrameInst %2, [?anon_1_new.target]
// CHECK-NEXT:  %4 = CreateFunctionInst %innerFunction()
// CHECK-NEXT:  %5 = StoreFrameInst %4 : closure, [innerFunction]
// CHECK-NEXT:  %6 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %7 = GetNewTargetInst
// CHECK-NEXT:  %8 = BinaryOperatorInst '!==', %7, undefined : undefined
// CHECK-NEXT:  %9 = CallInst %6, undefined : undefined, %8
// CHECK-NEXT:  %10 = CreateFunctionInst %innerArrow1()
// CHECK-NEXT:  %11 = StoreFrameInst %10 : closure, [innerArrow1]
// CHECK-NEXT:  %12 = LoadFrameInst [innerFunction]
// CHECK-NEXT:  %13 = AllocArrayInst 2 : number
// CHECK-NEXT:  %14 = StoreOwnPropertyInst %12, %13 : object, 0 : number, true : boolean
// CHECK-NEXT:  %15 = LoadFrameInst [innerArrow1]
// CHECK-NEXT:  %16 = StoreOwnPropertyInst %15, %13 : object, 1 : number, true : boolean
// CHECK-NEXT:  %17 = ReturnInst %13 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %18 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function innerFunction()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetNewTargetInst
// CHECK-NEXT:  %1 = ReturnInst %0
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:arrow innerArrow1()
// CHECK-NEXT:frame = [innerArrow2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [innerArrow2]
// CHECK-NEXT:  %1 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %2 = LoadFrameInst [?anon_1_new.target@func3]
// CHECK-NEXT:  %3 = BinaryOperatorInst '!==', %2, undefined : undefined
// CHECK-NEXT:  %4 = CallInst %1, undefined : undefined, %3
// CHECK-NEXT:  %5 = CreateFunctionInst %innerArrow2()
// CHECK-NEXT:  %6 = StoreFrameInst %5 : closure, [innerArrow2]
// CHECK-NEXT:  %7 = LoadFrameInst [innerArrow2]
// CHECK-NEXT:  %8 = CallInst %7, undefined : undefined
// CHECK-NEXT:  %9 = ReturnInst %8
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %10 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:arrow innerArrow2()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst [?anon_1_new.target@func3]
// CHECK-NEXT:  %1 = ReturnInst %0
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function func4()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetNewTargetInst
// CHECK-NEXT:  %1 = LoadPropertyInst %0, "prototype" : string
// CHECK-NEXT:  %2 = ReturnInst %1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
