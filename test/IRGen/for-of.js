// RUN: %hermes -dump-ir %s | %FileCheck --match-full-lines %s
// RUN: %hermes -dump-lir -O %s | %FileCheck --match-full-lines --check-prefix=CHKOPT %s

function forof_normal(seq, cb) {
    for(var i of seq)
        cb(i);
}
//CHECK-LABEL:function forof_normal(seq, cb)
//CHECK-NEXT:frame = [i, seq, cb]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [i]
//CHECK-NEXT:  %1 = StoreFrameInst %seq, [seq]
//CHECK-NEXT:  %2 = StoreFrameInst %cb, [cb]
//CHECK-NEXT:  %3 = LoadFrameInst [seq]
//CHECK-NEXT:  %4 = TryLoadGlobalPropertyInst globalObject : object, "Symbol" : string
//CHECK-NEXT:  %5 = LoadPropertyInst %4, "iterator" : string
//CHECK-NEXT:  %6 = LoadPropertyInst %3, %5
//CHECK-NEXT:  %7 = CallInst %6, %3
//CHECK-NEXT:  %8 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHECK-NEXT:  %9 = LoadPropertyInst %8, "ensureObject" : string
//CHECK-NEXT:  %10 = CallInst %9, undefined : undefined, %7, "iterator is not an object" : string
//CHECK-NEXT:  %11 = LoadPropertyInst %7, "next" : string
//CHECK-NEXT:  %12 = BranchInst %BB1
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %13 = CallInst %11, %7
//CHECK-NEXT:  %14 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHECK-NEXT:  %15 = LoadPropertyInst %14, "ensureObject" : string
//CHECK-NEXT:  %16 = CallInst %15, undefined : undefined, %13, "iterator.next() did not return an object" : string
//CHECK-NEXT:  %17 = LoadPropertyInst %13, "done" : string
//CHECK-NEXT:  %18 = CondBranchInst %17, %BB2, %BB3
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  %19 = LoadPropertyInst %13, "value" : string
//CHECK-NEXT:  %20 = StoreFrameInst %19, [i]
//CHECK-NEXT:  %21 = LoadFrameInst [cb]
//CHECK-NEXT:  %22 = LoadFrameInst [i]
//CHECK-NEXT:  %23 = CallInst %21, undefined : undefined, %22
//CHECK-NEXT:  %24 = BranchInst %BB1
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %25 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end

//CHKOPT-LABEL:function forof_normal(seq, cb) : undefined
//CHKOPT-NEXT:frame = []
//CHKOPT-NEXT:%BB0:
//CHKOPT-NEXT:  %0 = HBCLoadParamInst 1 : number
//CHKOPT-NEXT:  %1 = HBCLoadParamInst 2 : number
//CHKOPT-NEXT:  %2 = HBCLoadConstInst "iterator.next() did not return an object" : string
//CHKOPT-NEXT:  %3 = HBCLoadConstInst undefined : undefined
//CHKOPT-NEXT:  %4 = HBCGetGlobalObjectInst
//CHKOPT-NEXT:  %5 = TryLoadGlobalPropertyInst %4 : object, "Symbol" : string
//CHKOPT-NEXT:  %6 = LoadPropertyInst %5, "iterator" : string
//CHKOPT-NEXT:  %7 = LoadPropertyInst %0, %6
//CHKOPT-NEXT:  %8 = HBCCallNInst %7, %0
//CHKOPT-NEXT:  %9 = HBCLoadConstInst "iterator is not an object" : string
//CHKOPT-NEXT:  %10 = HBCCallBuiltinInst [HermesInternal.ensureObject] : number, undefined : undefined, %8, %9 : string
//CHKOPT-NEXT:  %11 = LoadPropertyInst %8, "next" : string
//CHKOPT-NEXT:  %12 = BranchInst %BB1
//CHKOPT-NEXT:%BB1:
//CHKOPT-NEXT:  %13 = HBCCallNInst %11, %8
//CHKOPT-NEXT:  %14 = HBCCallBuiltinInst [HermesInternal.ensureObject] : number, undefined : undefined, %13, %2 : string
//CHKOPT-NEXT:  %15 = LoadPropertyInst %13, "done" : string
//CHKOPT-NEXT:  %16 = CondBranchInst %15, %BB2, %BB3
//CHKOPT-NEXT:%BB3:
//CHKOPT-NEXT:  %17 = LoadPropertyInst %13, "value" : string
//CHKOPT-NEXT:  %18 = HBCCallNInst %1, %3 : undefined, %17
//CHKOPT-NEXT:  %19 = BranchInst %BB1
//CHKOPT-NEXT:%BB2:
//CHKOPT-NEXT:  %20 = ReturnInst %3 : undefined
//CHKOPT-NEXT:function_end

function forof_update(seq) {
    var i = 0, ar = [];
    for(ar[i++] of seq);
    return ar;
}
//CHECK-LABEL:function forof_update(seq)
//CHECK-NEXT:frame = [i, ar, seq]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [i]
//CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [ar]
//CHECK-NEXT:  %2 = StoreFrameInst %seq, [seq]
//CHECK-NEXT:  %3 = StoreFrameInst 0 : number, [i]
//CHECK-NEXT:  %4 = AllocArrayInst 0 : number
//CHECK-NEXT:  %5 = StoreFrameInst %4 : object, [ar]
//CHECK-NEXT:  %6 = LoadFrameInst [seq]
//CHECK-NEXT:  %7 = TryLoadGlobalPropertyInst globalObject : object, "Symbol" : string
//CHECK-NEXT:  %8 = LoadPropertyInst %7, "iterator" : string
//CHECK-NEXT:  %9 = LoadPropertyInst %6, %8
//CHECK-NEXT:  %10 = CallInst %9, %6
//CHECK-NEXT:  %11 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHECK-NEXT:  %12 = LoadPropertyInst %11, "ensureObject" : string
//CHECK-NEXT:  %13 = CallInst %12, undefined : undefined, %10, "iterator is not an object" : string
//CHECK-NEXT:  %14 = LoadPropertyInst %10, "next" : string
//CHECK-NEXT:  %15 = BranchInst %BB1
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %16 = CallInst %14, %10
//CHECK-NEXT:  %17 = TryLoadGlobalPropertyInst globalObject : object, "HermesInternal" : string
//CHECK-NEXT:  %18 = LoadPropertyInst %17, "ensureObject" : string
//CHECK-NEXT:  %19 = CallInst %18, undefined : undefined, %16, "iterator.next() did not return an object" : string
//CHECK-NEXT:  %20 = LoadPropertyInst %16, "done" : string
//CHECK-NEXT:  %21 = CondBranchInst %20, %BB2, %BB3
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  %22 = LoadPropertyInst %16, "value" : string
//CHECK-NEXT:  %23 = LoadFrameInst [ar]
//CHECK-NEXT:  %24 = LoadFrameInst [i]
//CHECK-NEXT:  %25 = AsNumberInst %24
//CHECK-NEXT:  %26 = BinaryOperatorInst '+', %25 : number, 1 : number
//CHECK-NEXT:  %27 = StoreFrameInst %26, [i]
//CHECK-NEXT:  %28 = StorePropertyInst %22, %23, %25 : number
//CHECK-NEXT:  %29 = BranchInst %BB1
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %30 = LoadFrameInst [ar]
//CHECK-NEXT:  %31 = ReturnInst %30
//CHECK-NEXT:%BB4:
//CHECK-NEXT:  %32 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end

//CHKOPT-LABEL:function forof_update(seq) : object
//CHKOPT-NEXT:frame = []
//CHKOPT-NEXT:%BB0:
//CHKOPT-NEXT:  %0 = HBCLoadParamInst 1 : number
//CHKOPT-NEXT:  %1 = HBCLoadConstInst 0 : number
//CHKOPT-NEXT:  %2 = HBCLoadConstInst "iterator.next() did not return an object" : string
//CHKOPT-NEXT:  %3 = HBCLoadConstInst 1 : number
//CHKOPT-NEXT:  %4 = AllocArrayInst 0 : number
//CHKOPT-NEXT:  %5 = HBCGetGlobalObjectInst
//CHKOPT-NEXT:  %6 = TryLoadGlobalPropertyInst %5 : object, "Symbol" : string
//CHKOPT-NEXT:  %7 = LoadPropertyInst %6, "iterator" : string
//CHKOPT-NEXT:  %8 = LoadPropertyInst %0, %7
//CHKOPT-NEXT:  %9 = HBCCallNInst %8, %0
//CHKOPT-NEXT:  %10 = HBCLoadConstInst "iterator is not an object" : string
//CHKOPT-NEXT:  %11 = HBCCallBuiltinInst [HermesInternal.ensureObject] : number, undefined : undefined, %9, %10 : string
//CHKOPT-NEXT:  %12 = LoadPropertyInst %9, "next" : string
//CHKOPT-NEXT:  %13 = BranchInst %BB1
//CHKOPT-NEXT:%BB1:
//CHKOPT-NEXT:  %14 = PhiInst %1 : number, %BB0, %20 : number, %BB2
//CHKOPT-NEXT:  %15 = HBCCallNInst %12, %9
//CHKOPT-NEXT:  %16 = HBCCallBuiltinInst [HermesInternal.ensureObject] : number, undefined : undefined, %15, %2 : string
//CHKOPT-NEXT:  %17 = LoadPropertyInst %15, "done" : string
//CHKOPT-NEXT:  %18 = CondBranchInst %17, %BB3, %BB2
//CHKOPT-NEXT:%BB2:
//CHKOPT-NEXT:  %19 = LoadPropertyInst %15, "value" : string
//CHKOPT-NEXT:  %20 = BinaryOperatorInst '+', %14 : number, %3 : number
//CHKOPT-NEXT:  %21 = StorePropertyInst %19, %4 : object, %14 : number
//CHKOPT-NEXT:  %22 = BranchInst %BB1
//CHKOPT-NEXT:%BB3:
//CHKOPT-NEXT:  %23 = ReturnInst %4 : object
//CHKOPT-NEXT:function_end
