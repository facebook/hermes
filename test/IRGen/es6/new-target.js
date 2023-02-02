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
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "func1" : string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "func2" : string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "func3" : string
// CHECK-NEXT:  %3 = DeclareGlobalVarInst "func4" : string
// CHECK-NEXT:  %4 = CreateFunctionInst %func1()
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4 : closure, globalObject : object, "func1" : string
// CHECK-NEXT:  %6 = CreateFunctionInst %func2()
// CHECK-NEXT:  %7 = StorePropertyLooseInst %6 : closure, globalObject : object, "func2" : string
// CHECK-NEXT:  %8 = CreateFunctionInst %func3()
// CHECK-NEXT:  %9 = StorePropertyLooseInst %8 : closure, globalObject : object, "func3" : string
// CHECK-NEXT:  %10 = CreateFunctionInst %func4()
// CHECK-NEXT:  %11 = StorePropertyLooseInst %10 : closure, globalObject : object, "func4" : string
// CHECK-NEXT:  %12 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %13 = StoreStackInst undefined : undefined, %12
// CHECK-NEXT:  %14 = LoadStackInst %12
// CHECK-NEXT:  %15 = ReturnInst %14
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
// CHECK-NEXT:  %0 = LoadParamInst %a
// CHECK-NEXT:  %1 = StoreFrameInst %0, [a]
// CHECK-NEXT:  %2 = LoadFrameInst [a]
// CHECK-NEXT:  %3 = CondBranchInst %2, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = GetNewTargetInst
// CHECK-NEXT:  %5 = ReturnInst %4
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %6 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %7 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %8 = GetNewTargetInst
// CHECK-NEXT:  %9 = BinaryOperatorInst '!==', %8, undefined : undefined
// CHECK-NEXT:  %10 = CallInst %7, undefined : undefined, %9
// CHECK-NEXT:  %11 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %12 = BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:function func3()
// CHECK-NEXT:frame = [?anon_0_this, ?anon_1_new.target, innerFunction, innerArrow1]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %this
// CHECK-NEXT:  %1 = CoerceThisNSInst %0
// CHECK-NEXT:  %2 = StoreFrameInst %1 : object, [?anon_0_this]
// CHECK-NEXT:  %3 = GetNewTargetInst
// CHECK-NEXT:  %4 = StoreFrameInst %3, [?anon_1_new.target]
// CHECK-NEXT:  %5 = StoreFrameInst undefined : undefined, [innerArrow1]
// CHECK-NEXT:  %6 = CreateFunctionInst %innerFunction()
// CHECK-NEXT:  %7 = StoreFrameInst %6 : closure, [innerFunction]
// CHECK-NEXT:  %8 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %9 = GetNewTargetInst
// CHECK-NEXT:  %10 = BinaryOperatorInst '!==', %9, undefined : undefined
// CHECK-NEXT:  %11 = CallInst %8, undefined : undefined, %10
// CHECK-NEXT:  %12 = CreateFunctionInst %innerArrow1()
// CHECK-NEXT:  %13 = StoreFrameInst %12 : closure, [innerArrow1]
// CHECK-NEXT:  %14 = LoadFrameInst [innerFunction]
// CHECK-NEXT:  %15 = AllocArrayInst 2 : number
// CHECK-NEXT:  %16 = StoreOwnPropertyInst %14, %15 : object, 0 : number, true : boolean
// CHECK-NEXT:  %17 = LoadFrameInst [innerArrow1]
// CHECK-NEXT:  %18 = StoreOwnPropertyInst %17, %15 : object, 1 : number, true : boolean
// CHECK-NEXT:  %19 = ReturnInst %15 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %20 = ReturnInst undefined : undefined
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
