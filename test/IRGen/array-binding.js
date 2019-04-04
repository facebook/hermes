// RUN: %hermesc -dump-ir %s | %FileCheck --match-full-lines %s

var [a, [b = 1, c] = [,2]] = x;

//CHECK:       %2 = TryLoadGlobalPropertyInst globalObject : object, "x" : string
//CHECK-NEXT:  %3 = TryLoadGlobalPropertyInst globalObject : object, "Symbol" : string
//CHECK-NEXT:  %4 = LoadPropertyInst %3, "iterator" : string
//CHECK-NEXT:  %5 = LoadPropertyInst %2, %4
//CHECK-NEXT:  %6 = CallInst %5, %2
//CHECK-NEXT:  %7 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHECK-NEXT:  %8 = LoadPropertyInst %7, "ensureObject" : string
//CHECK-NEXT:  %9 = CallInst %8, undefined : undefined, %6, "iterator is not an object" : string
//CHECK-NEXT:  %10 = LoadPropertyInst %6, "next" : string
//CHECK-NEXT:  %11 = AllocStackInst $?anon_1_iterDone
//CHECK-NEXT:  %12 = StoreStackInst undefined : undefined, %11
//CHECK-NEXT:  %13 = AllocStackInst $?anon_2_iterValue
//CHECK-NEXT:  %14 = StoreStackInst undefined : undefined, %13
//CHECK-NEXT:  %15 = BranchInst %BB1
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %16 = CallInst %10, %6
//CHECK-NEXT:  %17 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHECK-NEXT:  %18 = LoadPropertyInst %17, "ensureObject" : string
//CHECK-NEXT:  %19 = CallInst %18, undefined : undefined, %16, "iterator.next() did not return an object" : string
//CHECK-NEXT:  %20 = LoadPropertyInst %16, "done" : string
//CHECK-NEXT:  %21 = StoreStackInst %20, %11
//CHECK-NEXT:  %22 = CondBranchInst %20, %BB2, %BB3
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  %23 = LoadPropertyInst %16, "value" : string
//CHECK-NEXT:  %24 = StoreStackInst %23, %13
//CHECK-NEXT:  %25 = BranchInst %BB2
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %26 = LoadStackInst %13
//CHECK-NEXT:  %27 = StorePropertyInst %26, globalObject : object, "a" : string
//CHECK-NEXT:  %28 = StoreStackInst undefined : undefined, %13
//CHECK-NEXT:  %29 = LoadStackInst %11
//CHECK-NEXT:  %30 = CondBranchInst %29, %BB4, %BB5
//CHECK-NEXT:%BB5:
//CHECK-NEXT:  %31 = CallInst %10, %6
//CHECK-NEXT:  %32 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHECK-NEXT:  %33 = LoadPropertyInst %32, "ensureObject" : string
//CHECK-NEXT:  %34 = CallInst %33, undefined : undefined, %31, "iterator.next() did not return an object" : string
//CHECK-NEXT:  %35 = LoadPropertyInst %31, "done" : string
//CHECK-NEXT:  %36 = StoreStackInst %35, %11
//CHECK-NEXT:  %37 = CondBranchInst %35, %BB6, %BB7
//CHECK-NEXT:%BB7:
//CHECK-NEXT:  %38 = LoadPropertyInst %31, "value" : string
//CHECK-NEXT:  %39 = StoreStackInst %38, %13
//CHECK-NEXT:  %40 = BranchInst %BB4
//CHECK-NEXT:%BB4:
//CHECK-NEXT:  %41 = LoadStackInst %13
//CHECK-NEXT:  %42 = BinaryOperatorInst '!==', %41, undefined : undefined
//CHECK-NEXT:  %43 = CondBranchInst %42, %BB8, %BB6
//CHECK-NEXT:%BB6:
//CHECK-NEXT:  %44 = AllocArrayInst 2 : number
//CHECK-NEXT:  %45 = StoreOwnPropertyInst 2 : number, %44 : object, 1 : number, true : boolean
//CHECK-NEXT:  %46 = StoreStackInst %44 : object, %13
//CHECK-NEXT:  %47 = BranchInst %BB8
//CHECK-NEXT:%BB8:
//CHECK-NEXT:  %48 = LoadStackInst %13
//CHECK-NEXT:  %49 = TryLoadGlobalPropertyInst globalObject : object, "Symbol" : string
//CHECK-NEXT:  %50 = LoadPropertyInst %49, "iterator" : string
//CHECK-NEXT:  %51 = LoadPropertyInst %48, %50
//CHECK-NEXT:  %52 = CallInst %51, %48
//CHECK-NEXT:  %53 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHECK-NEXT:  %54 = LoadPropertyInst %53, "ensureObject" : string
//CHECK-NEXT:  %55 = CallInst %54, undefined : undefined, %52, "iterator is not an object" : string
//CHECK-NEXT:  %56 = LoadPropertyInst %52, "next" : string
//CHECK-NEXT:  %57 = AllocStackInst $?anon_3_iterDone
//CHECK-NEXT:  %58 = StoreStackInst undefined : undefined, %57
//CHECK-NEXT:  %59 = AllocStackInst $?anon_4_iterValue
//CHECK-NEXT:  %60 = StoreStackInst undefined : undefined, %59
//CHECK-NEXT:  %61 = BranchInst %BB9
//CHECK-NEXT:%BB9:
//CHECK-NEXT:  %62 = CallInst %56, %52
//CHECK-NEXT:  %63 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHECK-NEXT:  %64 = LoadPropertyInst %63, "ensureObject" : string
//CHECK-NEXT:  %65 = CallInst %64, undefined : undefined, %62, "iterator.next() did not return an object" : string
//CHECK-NEXT:  %66 = LoadPropertyInst %62, "done" : string
//CHECK-NEXT:  %67 = StoreStackInst %66, %57
//CHECK-NEXT:  %68 = CondBranchInst %66, %BB10, %BB11
//CHECK-NEXT:%BB11:
//CHECK-NEXT:  %69 = LoadPropertyInst %62, "value" : string
//CHECK-NEXT:  %70 = StoreStackInst %69, %59
//CHECK-NEXT:  %71 = BranchInst %BB12
//CHECK-NEXT:%BB12:
//CHECK-NEXT:  %72 = LoadStackInst %59
//CHECK-NEXT:  %73 = BinaryOperatorInst '!==', %72, undefined : undefined
//CHECK-NEXT:  %74 = CondBranchInst %73, %BB13, %BB10
//CHECK-NEXT:%BB10:
//CHECK-NEXT:  %75 = StoreStackInst 1 : number, %59
//CHECK-NEXT:  %76 = BranchInst %BB13
//CHECK-NEXT:%BB13:
//CHECK-NEXT:  %77 = LoadStackInst %59
//CHECK-NEXT:  %78 = StorePropertyInst %77, globalObject : object, "b" : string
//CHECK-NEXT:  %79 = StoreStackInst undefined : undefined, %59
//CHECK-NEXT:  %80 = LoadStackInst %57
//CHECK-NEXT:  %81 = CondBranchInst %80, %BB14, %BB15
//CHECK-NEXT:%BB15:
//CHECK-NEXT:  %82 = CallInst %56, %52
//CHECK-NEXT:  %83 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHECK-NEXT:  %84 = LoadPropertyInst %83, "ensureObject" : string
//CHECK-NEXT:  %85 = CallInst %84, undefined : undefined, %82, "iterator.next() did not return an object" : string
//CHECK-NEXT:  %86 = LoadPropertyInst %82, "done" : string
//CHECK-NEXT:  %87 = StoreStackInst %86, %57
//CHECK-NEXT:  %88 = CondBranchInst %86, %BB14, %BB16
//CHECK-NEXT:%BB16:
//CHECK-NEXT:  %89 = LoadPropertyInst %82, "value" : string
//CHECK-NEXT:  %90 = StoreStackInst %89, %59
//CHECK-NEXT:  %91 = BranchInst %BB14
//CHECK-NEXT:%BB14:
//CHECK-NEXT:  %92 = LoadStackInst %59
//CHECK-NEXT:  %93 = StorePropertyInst %92, globalObject : object, "c" : string
