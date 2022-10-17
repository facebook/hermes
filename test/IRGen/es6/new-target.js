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

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [func1, func2, func3, func4]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %func1#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "func1" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %func2#0#1()#3, %0
// CHECK-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "func2" : string
// CHECK-NEXT:  %5 = CreateFunctionInst %func3#0#1()#4, %0
// CHECK-NEXT:  %6 = StorePropertyInst %5 : closure, globalObject : object, "func3" : string
// CHECK-NEXT:  %7 = CreateFunctionInst %func4#0#1()#8, %0
// CHECK-NEXT:  %8 = StorePropertyInst %7 : closure, globalObject : object, "func4" : string
// CHECK-NEXT:  %9 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %10 = StoreStackInst undefined : undefined, %9
// CHECK-NEXT:  %11 = LoadStackInst %9
// CHECK-NEXT:  %12 = ReturnInst %11
// CHECK-NEXT:function_end

// CHECK:function func1#0#1()#2
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{func1#0#1()#2}
// CHECK-NEXT:  %1 = GetNewTargetInst
// CHECK-NEXT:  %2 = ReturnInst %1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function func2#0#1(a)#3
// CHECK-NEXT:frame = [a#3]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{func2#0#1()#3}
// CHECK-NEXT:  %1 = StoreFrameInst %a, [a#3], %0
// CHECK-NEXT:  %2 = LoadFrameInst [a#3], %0
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

// CHECK:function func3#0#1()#4
// CHECK-NEXT:frame = [?anon_0_this#4, ?anon_1_new.target#4, innerArrow1#4, innerFunction#4]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{func3#0#1()#4}
// CHECK-NEXT:  %1 = StoreFrameInst %this, [?anon_0_this#4], %0
// CHECK-NEXT:  %2 = GetNewTargetInst
// CHECK-NEXT:  %3 = StoreFrameInst %2, [?anon_1_new.target#4], %0
// CHECK-NEXT:  %4 = StoreFrameInst undefined : undefined, [innerArrow1#4], %0
// CHECK-NEXT:  %5 = CreateFunctionInst %innerFunction#1#4()#5, %0
// CHECK-NEXT:  %6 = StoreFrameInst %5 : closure, [innerFunction#4], %0
// CHECK-NEXT:  %7 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %8 = GetNewTargetInst
// CHECK-NEXT:  %9 = BinaryOperatorInst '!==', %8, undefined : undefined
// CHECK-NEXT:  %10 = CallInst %7, undefined : undefined, %9
// CHECK-NEXT:  %11 = CreateFunctionInst %innerArrow1#1#4()#6, %0
// CHECK-NEXT:  %12 = StoreFrameInst %11 : closure, [innerArrow1#4], %0
// CHECK-NEXT:  %13 = LoadFrameInst [innerFunction#4], %0
// CHECK-NEXT:  %14 = AllocArrayInst 2 : number
// CHECK-NEXT:  %15 = StoreOwnPropertyInst %13, %14 : object, 0 : number, true : boolean
// CHECK-NEXT:  %16 = LoadFrameInst [innerArrow1#4], %0
// CHECK-NEXT:  %17 = StoreOwnPropertyInst %16, %14 : object, 1 : number, true : boolean
// CHECK-NEXT:  %18 = ReturnInst %14 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %19 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function innerFunction#1#4()#5
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{innerFunction#1#4()#5}
// CHECK-NEXT:  %1 = GetNewTargetInst
// CHECK-NEXT:  %2 = ReturnInst %1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:arrow innerArrow1#1#4()#6
// CHECK-NEXT:frame = [innerArrow2#6]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{innerArrow1#1#4()#6}
// CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [innerArrow2#6], %0
// CHECK-NEXT:  %2 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %3 = LoadFrameInst [?anon_1_new.target#4@func3], %0
// CHECK-NEXT:  %4 = BinaryOperatorInst '!==', %3, undefined : undefined
// CHECK-NEXT:  %5 = CallInst %2, undefined : undefined, %4
// CHECK-NEXT:  %6 = CreateFunctionInst %innerArrow2#4#6()#7, %0
// CHECK-NEXT:  %7 = StoreFrameInst %6 : closure, [innerArrow2#6], %0
// CHECK-NEXT:  %8 = LoadFrameInst [innerArrow2#6], %0
// CHECK-NEXT:  %9 = CallInst %8, undefined : undefined
// CHECK-NEXT:  %10 = ReturnInst %9
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %11 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:arrow innerArrow2#4#6()#7
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{innerArrow2#4#6()#7}
// CHECK-NEXT:  %1 = LoadFrameInst [?anon_1_new.target#4@func3], %0
// CHECK-NEXT:  %2 = ReturnInst %1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function func4#0#1()#8
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{func4#0#1()#8}
// CHECK-NEXT:  %1 = GetNewTargetInst
// CHECK-NEXT:  %2 = LoadPropertyInst %1, "prototype" : string
// CHECK-NEXT:  %3 = ReturnInst %2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
