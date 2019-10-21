/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ir %s | %FileCheck --match-full-lines %s
// RUN: %hermesc -fno-enable-tdz -dump-ir %s | %FileCheck --match-full-lines --check-prefix=CHKDIS %s

function check1() {
    return x + y;
    let x = 10;
    const y = 1;
}
//CHECK-LABEL:function check1()
//CHECK-NEXT:frame = [x, ?anon_0_tdz$x, y, ?anon_1_tdz$y]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [x]
//CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [?anon_0_tdz$x]
//CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [y]
//CHECK-NEXT:  %3 = StoreFrameInst undefined : undefined, [?anon_1_tdz$y]
//CHECK-NEXT:  %4 = LoadFrameInst [?anon_0_tdz$x]
//CHECK-NEXT:  %5 = ThrowIfUndefinedInst %4
//CHECK-NEXT:  %6 = LoadFrameInst [x]
//CHECK-NEXT:  %7 = LoadFrameInst [?anon_1_tdz$y]
//CHECK-NEXT:  %8 = ThrowIfUndefinedInst %7
//CHECK-NEXT:  %9 = LoadFrameInst [y]
//CHECK-NEXT:  %10 = BinaryOperatorInst '+', %6, %9
//CHECK-NEXT:  %11 = ReturnInst %10
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %12 = StoreFrameInst 10 : number, [x]
//CHECK-NEXT:  %13 = StoreFrameInst true : boolean, [?anon_0_tdz$x]
//CHECK-NEXT:  %14 = StoreFrameInst 1 : number, [y]
//CHECK-NEXT:  %15 = StoreFrameInst true : boolean, [?anon_1_tdz$y]
//CHECK-NEXT:  %16 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end

//CHKDIS-LABEL:function check1()
//CHKDIS-NEXT:frame = [x, y]
//CHKDIS-NEXT:%BB0:
//CHKDIS-NEXT:  %0 = StoreFrameInst undefined : undefined, [x]
//CHKDIS-NEXT:  %1 = StoreFrameInst undefined : undefined, [y]
//CHKDIS-NEXT:  %2 = LoadFrameInst [x]
//CHKDIS-NEXT:  %3 = LoadFrameInst [y]
//CHKDIS-NEXT:  %4 = BinaryOperatorInst '+', %2, %3
//CHKDIS-NEXT:  %5 = ReturnInst %4
//CHKDIS-NEXT:%BB1:
//CHKDIS-NEXT:  %6 = StoreFrameInst 10 : number, [x]
//CHKDIS-NEXT:  %7 = StoreFrameInst 1 : number, [y]
//CHKDIS-NEXT:  %8 = ReturnInst undefined : undefined
//CHKDIS-NEXT:function_end
