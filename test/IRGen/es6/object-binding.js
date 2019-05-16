// RUN: %hermesc -dump-ir %s | %FileCheck %s --match-full-lines

//CHECK-LABEL:function global()
//CHECK-NEXT:frame = [], globals = [a, b, d, e]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = AllocStackInst $?anon_0_ret
//CHECK-NEXT:  %1 = StoreStackInst undefined : undefined, %0

var {a} = x;
//CHECK-NEXT:  %2 = TryLoadGlobalPropertyInst globalObject : object, "x" : string
//CHECK-NEXT:  %3 = LoadPropertyInst %2, "a" : string
//CHECK-NEXT:  %4 = StorePropertyInst %3, globalObject : object, "a" : string

var {a, b} = x;
//CHECK-NEXT:  %5 = TryLoadGlobalPropertyInst globalObject : object, "x" : string
//CHECK-NEXT:  %6 = LoadPropertyInst %5, "a" : string
//CHECK-NEXT:  %7 = StorePropertyInst %6, globalObject : object, "a" : string
//CHECK-NEXT:  %8 = LoadPropertyInst %5, "b" : string
//CHECK-NEXT:  %9 = StorePropertyInst %8, globalObject : object, "b" : string

var {a: b, c: d, } = x;
//CHECK-NEXT:  %10 = TryLoadGlobalPropertyInst globalObject : object, "x" : string
//CHECK-NEXT:  %11 = LoadPropertyInst %10, "a" : string
//CHECK-NEXT:  %12 = StorePropertyInst %11, globalObject : object, "b" : string
//CHECK-NEXT:  %13 = LoadPropertyInst %10, "c" : string
//CHECK-NEXT:  %14 = StorePropertyInst %13, globalObject : object, "d" : string

var {a: b = g} = x;
//CHECK-NEXT:  %15 = TryLoadGlobalPropertyInst globalObject : object, "x" : string
//CHECK-NEXT:  %16 = LoadPropertyInst %15, "a" : string
//CHECK-NEXT:  %17 = BinaryOperatorInst '!==', %16, undefined : undefined
//CHECK-NEXT:  %18 = CondBranchInst %17, %BB1, %BB2
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %19 = TryLoadGlobalPropertyInst globalObject : object, "g" : string
//CHECK-NEXT:  %20 = BranchInst %BB1
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %21 = PhiInst %16, %BB0, %19, %BB2
//CHECK-NEXT:  %22 = StorePropertyInst %21, globalObject : object, "b" : string

var {a: [b = 1, e] = g} = x;
//CHECK-NEXT:  %23 = TryLoadGlobalPropertyInst globalObject : object, "x" : string
//CHECK-NEXT:  %24 = LoadPropertyInst %23, "a" : string
//CHECK-NEXT:  %25 = BinaryOperatorInst '!==', %24, undefined : undefined
//CHECK-NEXT:  %26 = CondBranchInst %25, %BB3, %BB4
//CHECK-NEXT:%BB4:
//CHECK-NEXT:  %27 = TryLoadGlobalPropertyInst globalObject : object, "g" : string
//CHECK-NEXT:  %28 = BranchInst %BB3
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  %29 = PhiInst %24, %BB1, %27, %BB4
//CHECK-NEXT:  %30 = TryLoadGlobalPropertyInst globalObject : object, "Symbol" : string
//CHECK-NEXT:  %31 = LoadPropertyInst %30, "iterator" : string
//CHECK-NEXT:  %32 = LoadPropertyInst %29, %31
//CHECK-NEXT:  %33 = CallInst %32, %29
//CHECK-NEXT:  %34 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHECK-NEXT:  %35 = LoadPropertyInst %34, "ensureObject" : string
//CHECK-NEXT:  %36 = CallInst %35, undefined : undefined, %33, "iterator is not an object" : string
//CHECK-NEXT:  %37 = LoadPropertyInst %33, "next" : string
//CHECK-NEXT:  %38 = AllocStackInst $?anon_1_iterDone
//CHECK-NEXT:  %39 = StoreStackInst undefined : undefined, %38
//CHECK-NEXT:  %40 = AllocStackInst $?anon_2_iterValue
//CHECK-NEXT:  %41 = StoreStackInst undefined : undefined, %40
//CHECK-NEXT:  %42 = BranchInst %BB5
//CHECK-NEXT:%BB5:
//CHECK-NEXT:  %43 = CallInst %37, %33
//CHECK-NEXT:  %44 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHECK-NEXT:  %45 = LoadPropertyInst %44, "ensureObject" : string
//CHECK-NEXT:  %46 = CallInst %45, undefined : undefined, %43, "iterator.next() did not return an object" : string
//CHECK-NEXT:  %47 = LoadPropertyInst %43, "done" : string
//CHECK-NEXT:  %48 = StoreStackInst %47, %38
//CHECK-NEXT:  %49 = CondBranchInst %47, %BB6, %BB7
//CHECK-NEXT:%BB7:
//CHECK-NEXT:  %50 = LoadPropertyInst %43, "value" : string
//CHECK-NEXT:  %51 = StoreStackInst %50, %40
//CHECK-NEXT:  %52 = BranchInst %BB8
//CHECK-NEXT:%BB8:
//CHECK-NEXT:  %53 = LoadStackInst %40
//CHECK-NEXT:  %54 = BinaryOperatorInst '!==', %53, undefined : undefined
//CHECK-NEXT:  %55 = CondBranchInst %54, %BB9, %BB6
//CHECK-NEXT:%BB6:
//CHECK-NEXT:  %56 = StoreStackInst 1 : number, %40
//CHECK-NEXT:  %57 = BranchInst %BB9
//CHECK-NEXT:%BB9:
//CHECK-NEXT:  %58 = LoadStackInst %40
//CHECK-NEXT:  %59 = StorePropertyInst %58, globalObject : object, "b" : string
//CHECK-NEXT:  %60 = StoreStackInst undefined : undefined, %40
//CHECK-NEXT:  %61 = LoadStackInst %38
//CHECK-NEXT:  %62 = CondBranchInst %61, %BB10, %BB11
//CHECK-NEXT:%BB11:
//CHECK-NEXT:  %63 = CallInst %37, %33
//CHECK-NEXT:  %64 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHECK-NEXT:  %65 = LoadPropertyInst %64, "ensureObject" : string
//CHECK-NEXT:  %66 = CallInst %65, undefined : undefined, %63, "iterator.next() did not return an object" : string
//CHECK-NEXT:  %67 = LoadPropertyInst %63, "done" : string
//CHECK-NEXT:  %68 = StoreStackInst %67, %38
//CHECK-NEXT:  %69 = CondBranchInst %67, %BB10, %BB12
//CHECK-NEXT:%BB12:
//CHECK-NEXT:  %70 = LoadPropertyInst %63, "value" : string
//CHECK-NEXT:  %71 = StoreStackInst %70, %40
//CHECK-NEXT:  %72 = BranchInst %BB10
//CHECK-NEXT:%BB10:
//CHECK-NEXT:  %73 = LoadStackInst %40
//CHECK-NEXT:  %74 = StorePropertyInst %73, globalObject : object, "e" : string
