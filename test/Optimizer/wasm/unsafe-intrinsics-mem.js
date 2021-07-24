/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -funsafe-intrinsics -target=HBC -dump-postra %s | %FileCheck --match-full-lines --check-prefix=CHKRA %s
// RUN: %hermesc -funsafe-intrinsics -target=HBC -dump-bytecode %s | %FileCheck --match-full-lines --check-prefix=CHKBC %s

// Setup Asm.js/Wasm memory
var buffer = new ArrayBuffer(131072);
var HEAP8 = new Int8Array(buffer);
var HEAP16 = new Int16Array(buffer);
var HEAP32 = new Int32Array(buffer);
var HEAPU8 = new Uint8Array(buffer);
var HEAPU16 = new Uint16Array(buffer);
var HEAPU32 = new Uint32Array(buffer);
var HEAPF32 = new Float32Array(buffer);
var HEAPF64 = new Float64Array(buffer);

for (let i = 0; i < 256; i++) {
  HEAPU8[i] = i;
}

function loads(func) {
  t0 = __uasm.loadi8(HEAP8, 0);
  t1 = __uasm.loadu8(HEAPU8, 0);
  t2 = __uasm.loadi16(HEAP16, 0);
  t3 = __uasm.loadu16(HEAPU16, 0);
  t4 = __uasm.loadi32(HEAP32, 0);
  t5 = __uasm.loadu32(HEAPU32, 0);
}

//CHKRA-LABEL:function loads(func) : undefined
//CHKRA-NEXT:frame = []
//CHKRA-NEXT:%BB0:
//CHKRA-NEXT:  %0 = HBCGetGlobalObjectInst
//CHKRA-NEXT:  %1 = LoadPropertyInst %0 : object, "HEAP8" : string
//CHKRA-NEXT:  %2 = ImplicitMovInst undefined : undefined
//CHKRA-NEXT:  %3 = HBCLoadConstInst 0 : number
//CHKRA-NEXT:  %4 = CallIntrinsicInst [__uasm.loadi8_2] : number, undefined : undefined, %1, %3 : number
//CHKRA-NEXT:  %5 = StorePropertyInst %4, %0 : object, "t0" : string
//CHKRA-NEXT:  %6 = LoadPropertyInst %0 : object, "HEAPU8" : string
//CHKRA-NEXT:  %7 = ImplicitMovInst undefined : undefined
//CHKRA-NEXT:  %8 = CallIntrinsicInst [__uasm.loadu8_2] : number, undefined : undefined, %6, %3 : number
//CHKRA-NEXT:  %9 = StorePropertyInst %8, %0 : object, "t1" : string
//CHKRA-NEXT:  %10 = LoadPropertyInst %0 : object, "HEAP16" : string
//CHKRA-NEXT:  %11 = ImplicitMovInst undefined : undefined
//CHKRA-NEXT:  %12 = CallIntrinsicInst [__uasm.loadi16_2] : number, undefined : undefined, %10, %3 : number
//CHKRA-NEXT:  %13 = StorePropertyInst %12, %0 : object, "t2" : string
//CHKRA-NEXT:  %14 = LoadPropertyInst %0 : object, "HEAPU16" : string
//CHKRA-NEXT:  %15 = ImplicitMovInst undefined : undefined
//CHKRA-NEXT:  %16 = CallIntrinsicInst [__uasm.loadu16_2] : number, undefined : undefined, %14, %3 : number
//CHKRA-NEXT:  %17 = StorePropertyInst %16, %0 : object, "t3" : string
//CHKRA-NEXT:  %18 = LoadPropertyInst %0 : object, "HEAP32" : string
//CHKRA-NEXT:  %19 = ImplicitMovInst undefined : undefined
//CHKRA-NEXT:  %20 = CallIntrinsicInst [__uasm.loadi32_2] : number, undefined : undefined, %18, %3 : number
//CHKRA-NEXT:  %21 = StorePropertyInst %20, %0 : object, "t4" : string
//CHKRA-NEXT:  %22 = LoadPropertyInst %0 : object, "HEAPU32" : string
//CHKRA-NEXT:  %23 = ImplicitMovInst undefined : undefined
//CHKRA-NEXT:  %24 = CallIntrinsicInst [__uasm.loadu32_2] : number, undefined : undefined, %22, %3 : number
//CHKRA-NEXT:  %25 = StorePropertyInst %24, %0 : object, "t5" : string
//CHKRA-NEXT:  %26 = HBCLoadConstInst undefined : undefined
//CHKRA-NEXT:  %27 = ReturnInst %26 : undefined
//CHKRA-NEXT:function_end

//CHKBC-LABEL:Function<loads>(2 params, 12 registers, 0 symbols):
//CHKBC-NEXT:Offset in debug table: source 0x00b0, lexical 0x0000
//CHKBC-NEXT:    GetGlobalObject   r1
//CHKBC-NEXT:    GetByIdShort      r4, r1, 1, "HEAP8"
//CHKBC-NEXT:    LoadConstZero     r3
//CHKBC-NEXT:    Loadi8            r0, r4, r3
//CHKBC-NEXT:    PutById           r1, r0, 1, "t0"
//CHKBC-NEXT:    GetByIdShort      r4, r1, 2, "HEAPU8"
//CHKBC-NEXT:    Loadu8            r0, r4, r3
//CHKBC-NEXT:    PutById           r1, r0, 2, "t1"
//CHKBC-NEXT:    GetByIdShort      r4, r1, 3, "HEAP16"
//CHKBC-NEXT:    Loadi16           r0, r4, r3
//CHKBC-NEXT:    PutById           r1, r0, 3, "t2"
//CHKBC-NEXT:    GetByIdShort      r4, r1, 4, "HEAPU16"
//CHKBC-NEXT:    Loadu16           r0, r4, r3
//CHKBC-NEXT:    PutById           r1, r0, 4, "t3"
//CHKBC-NEXT:    GetByIdShort      r4, r1, 5, "HEAP32"
//CHKBC-NEXT:    Loadi32           r0, r4, r3
//CHKBC-NEXT:    PutById           r1, r0, 5, "t4"
//CHKBC-NEXT:    GetByIdShort      r4, r1, 6, "HEAPU32"
//CHKBC-NEXT:    Loadu32           r0, r4, r3
//CHKBC-NEXT:    PutById           r1, r0, 6, "t5"
//CHKBC-NEXT:    LoadConstUndefined r0
//CHKBC-NEXT:    Ret               r0

function stores(func, x) {
  __uasm.store8(HEAP8, 0, x);
  __uasm.store16(HEAP16, 0, x);
  __uasm.store32(HEAP32, 0, x);
}

//CHKRA-LABEL:function stores(func, x) : undefined
//CHKRA-NEXT:frame = []
//CHKRA-NEXT:%BB0:
//CHKRA-NEXT:  %0 = HBCLoadParamInst 2 : number
//CHKRA-NEXT:  %1 = HBCGetGlobalObjectInst
//CHKRA-NEXT:  %2 = LoadPropertyInst %1 : object, "HEAP8" : string
//CHKRA-NEXT:  %3 = ImplicitMovInst undefined : undefined
//CHKRA-NEXT:  %4 = HBCLoadConstInst 0 : number
//CHKRA-NEXT:  %5 = MovInst %0
//CHKRA-NEXT:  %6 = CallIntrinsicInst [__uasm.store8_3] : number, undefined : undefined, %2, %4 : number, %5
//CHKRA-NEXT:  %7 = LoadPropertyInst %1 : object, "HEAP16" : string
//CHKRA-NEXT:  %8 = ImplicitMovInst undefined : undefined
//CHKRA-NEXT:  %9 = MovInst %0
//CHKRA-NEXT:  %10 = CallIntrinsicInst [__uasm.store16_3] : number, undefined : undefined, %7, %4 : number, %9
//CHKRA-NEXT:  %11 = LoadPropertyInst %1 : object, "HEAP32" : string
//CHKRA-NEXT:  %12 = ImplicitMovInst undefined : undefined
//CHKRA-NEXT:  %13 = MovInst %0
//CHKRA-NEXT:  %14 = CallIntrinsicInst [__uasm.store32_3] : number, undefined : undefined, %11, %4 : number, %13
//CHKRA-NEXT:  %15 = HBCLoadConstInst undefined : undefined
//CHKRA-NEXT:  %16 = ReturnInst %15 : undefined
//CHKRA-NEXT:function_end

//CHKBC-LABEL:Function<stores>(3 params, 14 registers, 0 symbols):
//CHKBC-NEXT:Offset in debug table: source 0x00ea, lexical 0x0000
//CHKBC-NEXT:    LoadParam         r2, 2
//CHKBC-NEXT:    GetGlobalObject   r0
//CHKBC-NEXT:    GetByIdShort      r6, r0, 1, "HEAP8"
//CHKBC-NEXT:    LoadConstZero     r5
//CHKBC-NEXT:    Mov               r4, r2
//CHKBC-NEXT:    Store8            r6, r5, r4
//CHKBC-NEXT:    GetByIdShort      r6, r0, 2, "HEAP16"
//CHKBC-NEXT:    Mov               r4, r2
//CHKBC-NEXT:    Store16           r6, r5, r4
//CHKBC-NEXT:    GetByIdShort      r6, r0, 3, "HEAP32"
//CHKBC-NEXT:    Mov               r4, r2
//CHKBC-NEXT:    Store32           r6, r5, r4
//CHKBC-NEXT:    LoadConstUndefined r0
//CHKBC-NEXT:    Ret               r0
