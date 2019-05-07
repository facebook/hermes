// RUN: %hermesc -Xflow-parser -dump-ir %s | %FileCheck %s --match-full-lines
// REQUIRES: flowparser

function* simple() {
  yield 1;
}

// CHECK-LABEL: function simple()
// CHECK-NEXT: frame = []
// CHECK-NEXT: %BB0:
// CHECK-NEXT:   %0 = CreateGeneratorInst %?anon_0_simple()
// CHECK-NEXT:   %1 = ReturnInst %0 : object
// CHECK-NEXT: function_end

// CHECK-LABEL: function ?anon_0_simple()
// CHECK-NEXT: frame = []
// CHECK-NEXT: %BB0:
// CHECK-NEXT:   %0 = StartGeneratorInst
// CHECK-NEXT:   %1 = AllocStackInst $?anon_0_isReturn
// CHECK-NEXT:   %2 = ResumeGeneratorInst %1
// CHECK-NEXT:   %3 = LoadStackInst %1
// CHECK-NEXT:   %4 = CondBranchInst %3, %BB1, %BB2
// CHECK-NEXT: %BB2:
// CHECK-NEXT:   %5 = AllocStackInst $?anon_1_isReturn
// CHECK-NEXT:   %6 = SaveAndYieldInst 1 : number, %BB3
// CHECK-NEXT: %BB1:
// CHECK-NEXT:   %7 = ReturnInst %2
// CHECK-NEXT: %BB3:
// CHECK-NEXT:   %8 = ResumeGeneratorInst %5
// CHECK-NEXT:   %9 = LoadStackInst %5
// CHECK-NEXT:   %10 = CondBranchInst %9, %BB4, %BB5
// CHECK-NEXT: %BB5:
// CHECK-NEXT:   %11 = ReturnInst undefined : undefined
// CHECK-NEXT: %BB4:
// CHECK-NEXT:   %12 = ReturnInst %8
// CHECK-NEXT: function_end

function *useResult() {
  var x = yield 1;
}

// CHECK-LABEL: function useResult()
// CHECK-NEXT: frame = [x]
// CHECK-NEXT: %BB0:
// CHECK-NEXT:   %0 = StoreFrameInst undefined : undefined, [x]
// CHECK-NEXT:   %1 = CreateGeneratorInst %?anon_1_useResult()
// CHECK-NEXT:   %2 = ReturnInst %1 : object
// CHECK-NEXT: function_end

// CHECK-LABEL: function ?anon_1_useResult()
// CHECK-NEXT: frame = [x]
// CHECK-NEXT: %BB0:
// CHECK-NEXT:   %0 = StartGeneratorInst
// CHECK-NEXT:   %1 = AllocStackInst $?anon_0_isReturn
// CHECK-NEXT:   %2 = ResumeGeneratorInst %1
// CHECK-NEXT:   %3 = LoadStackInst %1
// CHECK-NEXT:   %4 = CondBranchInst %3, %BB1, %BB2
// CHECK-NEXT: %BB2:
// CHECK-NEXT:   %5 = StoreFrameInst undefined : undefined, [x]
// CHECK-NEXT:   %6 = AllocStackInst $?anon_1_isReturn
// CHECK-NEXT:   %7 = SaveAndYieldInst 1 : number, %BB3
// CHECK-NEXT: %BB1:
// CHECK-NEXT:   %8 = ReturnInst %2
// CHECK-NEXT: %BB3:
// CHECK-NEXT:   %9 = ResumeGeneratorInst %6
// CHECK-NEXT:   %10 = LoadStackInst %6
// CHECK-NEXT:   %11 = CondBranchInst %10, %BB4, %BB5
// CHECK-NEXT: %BB5:
// CHECK-NEXT:   %12 = StoreFrameInst %9, [x]
// CHECK-NEXT:   %13 = ReturnInst undefined : undefined
// CHECK-NEXT: %BB4:
// CHECK-NEXT:   %14 = ReturnInst %9
// CHECK-NEXT: function_end

function *loop(x) {
  var i = 0;
  while (true) {
    yield x[i++];
  }
}


// CHECK-LABEL: function loop(x)
// CHECK-NEXT: frame = [i, x]
// CHECK-NEXT: %BB0:
// CHECK-NEXT:   %0 = StoreFrameInst undefined : undefined, [i]
// CHECK-NEXT:   %1 = StoreFrameInst %x, [x]
// CHECK-NEXT:   %2 = CreateGeneratorInst %?anon_2_loop()
// CHECK-NEXT:   %3 = ReturnInst %2 : object
// CHECK-NEXT: function_end

// CHECK-LABEL: function ?anon_2_loop(x)
// CHECK-NEXT: frame = [i, x]
// CHECK-NEXT: %BB0:
// CHECK-NEXT:   %0 = StartGeneratorInst
// CHECK-NEXT:   %1 = AllocStackInst $?anon_0_isReturn
// CHECK-NEXT:   %2 = ResumeGeneratorInst %1
// CHECK-NEXT:   %3 = LoadStackInst %1
// CHECK-NEXT:   %4 = CondBranchInst %3, %BB1, %BB2
// CHECK-NEXT: %BB2:
// CHECK-NEXT:   %5 = StoreFrameInst undefined : undefined, [i]
// CHECK-NEXT:   %6 = StoreFrameInst %x, [x]
// CHECK-NEXT:   %7 = StoreFrameInst 0 : number, [i]
// CHECK-NEXT:   %8 = BranchInst %BB3
// CHECK-NEXT: %BB1:
// CHECK-NEXT:   %9 = ReturnInst %2
// CHECK-NEXT: %BB4:
// CHECK-NEXT:   %10 = LoadFrameInst [x]
// CHECK-NEXT:   %11 = LoadFrameInst [i]
// CHECK-NEXT:   %12 = AsNumberInst %11
// CHECK-NEXT:   %13 = BinaryOperatorInst '+', %12 : number, 1 : number
// CHECK-NEXT:   %14 = StoreFrameInst %13, [i]
// CHECK-NEXT:   %15 = LoadPropertyInst %10, %12 : number
// CHECK-NEXT:   %16 = AllocStackInst $?anon_1_isReturn
// CHECK-NEXT:   %17 = SaveAndYieldInst %15, %BB5
// CHECK-NEXT: %BB6:
// CHECK-NEXT:   %18 = ReturnInst undefined : undefined
// CHECK-NEXT: %BB3:
// CHECK-NEXT:   %19 = CondBranchInst true : boolean, %BB4, %BB6
// CHECK-NEXT: %BB7:
// CHECK-NEXT:   %20 = CondBranchInst true : boolean, %BB4, %BB6
// CHECK-NEXT: %BB8:
// CHECK-NEXT:   %21 = BranchInst %BB7
// CHECK-NEXT: %BB5:
// CHECK-NEXT:   %22 = ResumeGeneratorInst %16
// CHECK-NEXT:   %23 = LoadStackInst %16
// CHECK-NEXT:   %24 = CondBranchInst %23, %BB9, %BB10
// CHECK-NEXT: %BB10:
// CHECK-NEXT:   %25 = BranchInst %BB8
// CHECK-NEXT: %BB9:
// CHECK-NEXT:   %26 = ReturnInst %22
// CHECK-NEXT: function_end

// Test generation of function expressions.
var simple2 = function*() {
  yield 1;
}

// CHECK-LABEL: function simple2()
// CHECK-NEXT: frame = []
// CHECK-NEXT: %BB0:
// CHECK-NEXT:   %0 = CreateGeneratorInst %?anon_4_simple2()
// CHECK-NEXT:   %1 = ReturnInst %0 : object
// CHECK-NEXT: function_end

// CHECK-LABEL: function ?anon_4_simple2()
// CHECK-NEXT: frame = []
// CHECK-NEXT: %BB0:
// CHECK-NEXT:   %0 = StartGeneratorInst
// CHECK-NEXT:   %1 = AllocStackInst $?anon_0_isReturn
// CHECK-NEXT:   %2 = ResumeGeneratorInst %1
// CHECK-NEXT:   %3 = LoadStackInst %1
// CHECK-NEXT:   %4 = CondBranchInst %3, %BB1, %BB2
// CHECK-NEXT: %BB2:
// CHECK-NEXT:   %5 = AllocStackInst $?anon_1_isReturn
// CHECK-NEXT:   %6 = SaveAndYieldInst 1 : number, %BB3
// CHECK-NEXT: %BB1:
// CHECK-NEXT:   %7 = ReturnInst %2
// CHECK-NEXT: %BB3:
// CHECK-NEXT:   %8 = ResumeGeneratorInst %5
// CHECK-NEXT:   %9 = LoadStackInst %5
// CHECK-NEXT:   %10 = CondBranchInst %9, %BB4, %BB5
// CHECK-NEXT: %BB5:
// CHECK-NEXT:   %11 = ReturnInst undefined : undefined
// CHECK-NEXT: %BB4:
// CHECK-NEXT:   %12 = ReturnInst %8
// CHECK-NEXT: function_end
