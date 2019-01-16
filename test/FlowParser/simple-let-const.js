// RUN: %hermesc -Xflow-parser -dump-ir %s | %FileCheck --match-full-lines %s
// REQUIRES: flowparser

function foo() {
    let x = 20;
    const y = 30;
    return x + y;
}

//CHECK-LABEL:function foo()
//CHECK-NEXT:frame = [x, y]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [x]
//CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [y]
//CHECK-NEXT:  %2 = StoreFrameInst 20 : number, [x]
//CHECK-NEXT:  %3 = StoreFrameInst 30 : number, [y]
//CHECK-NEXT:  %4 = LoadFrameInst [x]
//CHECK-NEXT:  %5 = LoadFrameInst [y]
//CHECK-NEXT:  %6 = BinaryOperatorInst '+', %4, %5
//CHECK-NEXT:  %7 = ReturnInst %6
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %8 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
