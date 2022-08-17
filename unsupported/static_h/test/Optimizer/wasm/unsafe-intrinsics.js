/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -funsafe-intrinsics -target=HBC -dump-postra %s | %FileCheck --match-full-lines --check-prefix=CHKRA %s
// RUN: %hermesc -funsafe-intrinsics -target=HBC -dump-bytecode %s | %FileCheck --match-full-lines --check-prefix=CHKBC %s
// REQUIRES: run_wasm

// Instrinsics that are defined should not cause any error. The call sequence is
// lowered to a CallIntrinsicInst.
function unsafeIntrinsics(func) {
  t0 = __uasm.add32(1, 2);
  t1 = __uasm.mul32(42, 7);
  return t0 + t1;
}
//CHKRA-LABEL:function unsafeIntrinsics(func) : string|number
//CHKRA-NEXT:frame = []
//CHKRA-NEXT:%BB0:
//CHKRA-NEXT:  %0 = HBCLoadConstInst 1 : number
//CHKRA-NEXT:  %1 = HBCLoadConstInst 2 : number
//CHKRA-NEXT:  %2 = CallIntrinsicInst [__uasm.add32_2] : number, %0 : number, %1 : number
//CHKRA-NEXT:  %3 = HBCGetGlobalObjectInst
//CHKRA-NEXT:  %4 = StorePropertyInst %2, %3 : object, "t0" : string
//CHKRA-NEXT:  %5 = HBCLoadConstInst 42 : number
//CHKRA-NEXT:  %6 = HBCLoadConstInst 7 : number
//CHKRA-NEXT:  %7 = CallIntrinsicInst [__uasm.mul32_2] : number, %5 : number, %6 : number
//CHKRA-NEXT:  %8 = StorePropertyInst %7, %3 : object, "t1" : string
//CHKRA-NEXT:  %9 = TryLoadGlobalPropertyInst %3 : object, "t0" : string
//CHKRA-NEXT:  %10 = TryLoadGlobalPropertyInst %3 : object, "t1" : string
//CHKRA-NEXT:  %11 = BinaryOperatorInst '+', %9, %10
//CHKRA-NEXT:  %12 = ReturnInst %11 : string|number
//CHKRA-NEXT:function_end

//CHKBC-LABEL:Function<unsafeIntrinsics>(2 params, 3 registers, 0 symbols):
//CHKBC-NEXT:Offset in debug table: source 0x000a, lexical 0x0000
//CHKBC-NEXT:    LoadConstUInt8    r1, 1
//CHKBC-NEXT:    LoadConstUInt8    r0, 2
//CHKBC-NEXT:    Add32             r1, r1, r0
//CHKBC-NEXT:    GetGlobalObject   r0
//CHKBC-NEXT:    PutById           r0, r1, 1, "t0"
//CHKBC-NEXT:    LoadConstUInt8    r2, 42
//CHKBC-NEXT:    LoadConstUInt8    r1, 7
//CHKBC-NEXT:    Mul32             r1, r2, r1
//CHKBC-NEXT:    PutById           r0, r1, 2, "t1"
//CHKBC-NEXT:    TryGetById        r1, r0, 1, "t0"
//CHKBC-NEXT:    TryGetById        r0, r0, 2, "t1"
//CHKBC-NEXT:    Add               r0, r1, r0
//CHKBC-NEXT:    Ret               r0

// If namespace is not "__uasm", CallIntrinsicInst should not be emitted.
function incorrectNamespace(func) {
  return uasm.add32(20, 21);
}
//CHKRA-LABEL:function incorrectNamespace(func)
//CHKRA-NEXT:frame = []
//CHKRA-NEXT:%BB0:
//CHKRA-NEXT:  %0 = HBCGetGlobalObjectInst
//CHKRA-NEXT:  %1 = TryLoadGlobalPropertyInst %0 : object, "uasm" : string
//CHKRA-NEXT:  %2 = LoadPropertyInst %1, "add32" : string
//CHKRA-NEXT:  %3 = HBCLoadConstInst 20 : number
//CHKRA-NEXT:  %4 = HBCLoadConstInst 21 : number
//CHKRA-NEXT:  %5 = ImplicitMovInst %1
//CHKRA-NEXT:  %6 = ImplicitMovInst %3 : number
//CHKRA-NEXT:  %7 = ImplicitMovInst %4 : number
//CHKRA-NEXT:  %8 = HBCCallNInst %2, %1, %3 : number, %4 : number
//CHKRA-NEXT:  %9 = ReturnInst %8
//CHKRA-NEXT:function_end
