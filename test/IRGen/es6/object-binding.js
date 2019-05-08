// RUN: %hermesc -dump-ir %s | %FileCheck %s --match-full-lines

//CHECK-LABEL: function global()
//CHECK-NEXT: frame = [], globals = [a, b, d, e]
//CHECK-NEXT: %BB0:
//CHECK-NEXT:   %0 = BranchInst %BB1
//CHECK-NEXT: %BB1:
//CHECK-NEXT:   %1 = AllocStackInst $?anon_0_ret
//CHECK-NEXT:   %2 = StoreStackInst undefined : undefined, %1

var {a} = x;
//CHECK-NEXT:   %3 = TryLoadGlobalPropertyInst globalObject : object, "x" : string
//CHECK-NEXT:   %4 = LoadPropertyInst %3, "a" : string
//CHECK-NEXT:   %5 = StorePropertyInst %4, globalObject : object, "a" : string

var {a, b} = x;
//CHECK-NEXT:   %6 = TryLoadGlobalPropertyInst globalObject : object, "x" : string
//CHECK-NEXT:   %7 = LoadPropertyInst %6, "a" : string
//CHECK-NEXT:   %8 = StorePropertyInst %7, globalObject : object, "a" : string
//CHECK-NEXT:   %9 = LoadPropertyInst %6, "b" : string
//CHECK-NEXT:   %10 = StorePropertyInst %9, globalObject : object, "b" : string

var {a: b, c: d, } = x;
//CHECK-NEXT:   %11 = TryLoadGlobalPropertyInst globalObject : object, "x" : string
//CHECK-NEXT:   %12 = LoadPropertyInst %11, "a" : string
//CHECK-NEXT:   %13 = StorePropertyInst %12, globalObject : object, "b" : string
//CHECK-NEXT:   %14 = LoadPropertyInst %11, "c" : string
//CHECK-NEXT:   %15 = StorePropertyInst %14, globalObject : object, "d" : string

var {a: b = g} = x;
//CHECK-NEXT:  %16 = TryLoadGlobalPropertyInst globalObject : object, "x" : string
//CHECK-NEXT:  %17 = LoadPropertyInst %16, "a" : string
//CHECK-NEXT:  %18 = BinaryOperatorInst '!==', %17, undefined : undefined
//CHECK-NEXT:  %19 = CondBranchInst %18, %BB2, %BB3
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  %20 = TryLoadGlobalPropertyInst globalObject : object, "g" : string
//CHECK-NEXT:  %21 = BranchInst %BB2
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %22 = PhiInst %17, %BB1, %20, %BB3
//CHECK-NEXT:  %23 = StorePropertyInst %22, globalObject : object, "b" : string

var {a: [b = 1, e] = g} = x;
//CHECK-NEXT:  %24 = TryLoadGlobalPropertyInst globalObject : object, "x" : string
//CHECK-NEXT:  %25 = LoadPropertyInst %24, "a" : string
//CHECK-NEXT:  %26 = BinaryOperatorInst '!==', %25, undefined : undefined
//CHECK-NEXT:  %27 = CondBranchInst %26, %BB4, %BB5
//CHECK-NEXT:%BB5:
//CHECK-NEXT:  %28 = TryLoadGlobalPropertyInst globalObject : object, "g" : string
//CHECK-NEXT:  %29 = BranchInst %BB4
//CHECK-NEXT:%BB4:
//CHECK-NEXT:  %30 = PhiInst %25, %BB2, %28, %BB5
//CHECK-NEXT:  %31 = TryLoadGlobalPropertyInst globalObject : object, "Symbol" : string
//CHECK-NEXT:  %32 = LoadPropertyInst %31, "iterator" : string
//CHECK-NEXT:  %33 = LoadPropertyInst %30, %32
//CHECK-NEXT:  %34 = CallInst %33, %30
//CHECK-NEXT:  %35 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHECK-NEXT:  %36 = LoadPropertyInst %35, "ensureObject" : string
//CHECK-NEXT:  %37 = CallInst %36, undefined : undefined, %34, "iterator is not an object" : string
//CHECK-NEXT:  %38 = LoadPropertyInst %34, "next" : string
//CHECK-NEXT:  %39 = AllocStackInst $?anon_1_iterDone
//CHECK-NEXT:  %40 = StoreStackInst undefined : undefined, %39
//CHECK-NEXT:  %41 = AllocStackInst $?anon_2_iterValue
//CHECK-NEXT:  %42 = StoreStackInst undefined : undefined, %41
//CHECK-NEXT:  %43 = BranchInst %BB6
//CHECK-NEXT:%BB6:
//CHECK-NEXT:  %44 = CallInst %38, %34
//CHECK-NEXT:  %45 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHECK-NEXT:  %46 = LoadPropertyInst %45, "ensureObject" : string
//CHECK-NEXT:  %47 = CallInst %46, undefined : undefined, %44, "iterator.next() did not return an object" : string
//CHECK-NEXT:  %48 = LoadPropertyInst %44, "done" : string
//CHECK-NEXT:  %49 = StoreStackInst %48, %39
//CHECK-NEXT:  %50 = CondBranchInst %48, %BB7, %BB8
//CHECK-NEXT:%BB8:
//CHECK-NEXT:  %51 = LoadPropertyInst %44, "value" : string
//CHECK-NEXT:  %52 = StoreStackInst %51, %41
//CHECK-NEXT:  %53 = BranchInst %BB9
//CHECK-NEXT:%BB9:
//CHECK-NEXT:  %54 = LoadStackInst %41
//CHECK-NEXT:  %55 = BinaryOperatorInst '!==', %54, undefined : undefined
//CHECK-NEXT:  %56 = CondBranchInst %55, %BB10, %BB7
//CHECK-NEXT:%BB7:
//CHECK-NEXT:  %57 = StoreStackInst 1 : number, %41
//CHECK-NEXT:  %58 = BranchInst %BB10
//CHECK-NEXT:%BB10:
//CHECK-NEXT:  %59 = LoadStackInst %41
//CHECK-NEXT:  %60 = StorePropertyInst %59, globalObject : object, "b" : string
//CHECK-NEXT:  %61 = StoreStackInst undefined : undefined, %41
//CHECK-NEXT:  %62 = LoadStackInst %39
//CHECK-NEXT:  %63 = CondBranchInst %62, %BB11, %BB12
//CHECK-NEXT:%BB12:
//CHECK-NEXT:  %64 = CallInst %38, %34
//CHECK-NEXT:  %65 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHECK-NEXT:  %66 = LoadPropertyInst %65, "ensureObject" : string
//CHECK-NEXT:  %67 = CallInst %66, undefined : undefined, %64, "iterator.next() did not return an object" : string
//CHECK-NEXT:  %68 = LoadPropertyInst %64, "done" : string
//CHECK-NEXT:  %69 = StoreStackInst %68, %39
//CHECK-NEXT:  %70 = CondBranchInst %68, %BB11, %BB13
//CHECK-NEXT:%BB13:
//CHECK-NEXT:  %71 = LoadPropertyInst %64, "value" : string
//CHECK-NEXT:  %72 = StoreStackInst %71, %41
//CHECK-NEXT:  %73 = BranchInst %BB11
//CHECK-NEXT:%BB11:
//CHECK-NEXT:  %74 = LoadStackInst %41
//CHECK-NEXT:  %75 = StorePropertyInst %74, globalObject : object, "e" : string
