/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O -funsafe-intrinsics -target=HBC -dump-ir -commonjs %s | %FileCheck --match-full-lines %s
// REQUIRES: run_wasm

// C++ source code:
// #include <stdint.h>
//
// void rgb_to_gray(uint8_t *image, uint8_t* output, uint32_t size) {
//   for (uint32_t i = 0; i < size; i++) {
//     uint32_t R = image[3*i];
//     uint32_t G = image[3*i+1];
//     uint32_t B = image[3*i+2];
//     uint32_t Gray = ((299 * R + 587 * G + 114 * B) / 1000);
//     output[i] = (uint8_t)Gray;
//   }
//   return;
// }
//
// int main() {
//   uint8_t image[1024*1024*4];
//   uint32_t size = 1024*1024;
//   for (uint32_t i = 0; i < size; i++) {
//     image[3 * i] = i % 256;
//     image[3 * i + 1] = i * (i + 1) % 256;
//     image[3 * i + 2] = i * (i % 5) % 256;
//   }
//   rgb_to_gray(image, image+1024*1024*3, size);
//   return 0;
// }

function Table(ret) {
  // grow method not included; table is not growable
  ret.set = function(i, func) {
    this[i] = func;
  };
  ret.get = function(i) {
    return this[i];
  };
  return ret;
}

function asmFunc(env) {
 var buffer = new ArrayBuffer(131072);
 var HEAP8 = new Int8Array(buffer);
 var HEAP16 = new Int16Array(buffer);
 var HEAP32 = new Int32Array(buffer);
 var HEAPU8 = new Uint8Array(buffer);
 var HEAPU16 = new Uint16Array(buffer);
 var HEAPU32 = new Uint32Array(buffer);
 var HEAPF32 = new Float32Array(buffer);
 var HEAPF64 = new Float64Array(buffer);
 var Math_imul = Math.imul;
 var Math_fround = Math.fround;
 var Math_abs = Math.abs;
 var Math_clz32 = Math.clz32;
 var Math_min = Math.min;
 var Math_max = Math.max;
 var Math_floor = Math.floor;
 var Math_ceil = Math.ceil;
 var Math_trunc = Math.trunc;
 var Math_sqrt = Math.sqrt;
 var abort = env.abort;
 var nan = NaN;
 var infinity = Infinity;
 var global$0 = 4259840;
 var global$1 = 1024;
 var global$2 = 1024;
 var global$3 = 1024;
 var global$4 = 66560;
 var global$5 = 0;
 var global$6 = 1;
 function $0() {

 }

 function $1($0_1, $1_1, $2_1) {
  $0_1 = $0_1 | 0;
  $1_1 = $1_1 | 0;
  $2_1 = $2_1 | 0;
  if ($2_1) {
   label$2 : while (1) {
    HEAP8[$1_1 >> 0] = (((Math_imul(HEAPU8[($0_1 + 1 | 0) >> 0] | 0, 587) + Math_imul(HEAPU8[$0_1 >> 0] | 0, 299) | 0) + Math_imul(HEAPU8[($0_1 + 2 | 0) >> 0] | 0, 114) | 0) >>> 0) / (1e3 >>> 0) | 0;
    // print(HEAP8[$1_1 >> 0]);
    $0_1 = $0_1 + 3 | 0;
    $1_1 = $1_1 + 1 | 0;
    $2_1 = $2_1 - 1 | 0;
    if ($2_1) {
     continue label$2
    }
    break label$2;
   }
  }
 }

//CHECK-LABEL: function $1($0_1, $1_1, $2_1) : undefined
//CHECK-NEXT: frame = []
//CHECK-NEXT: %BB0:
//CHECK-NEXT:   %0 = AsInt32Inst %$0_1
//CHECK-NEXT:   %1 = AsInt32Inst %$1_1
//CHECK-NEXT:   %2 = AsInt32Inst %$2_1
//CHECK-NEXT:   %3 = CondBranchInst %2 : number, %BB1, %BB2
//CHECK-NEXT: %BB2:
//CHECK-NEXT:   %4 = ReturnInst undefined : undefined
//CHECK-NEXT: %BB1:
//CHECK-NEXT:   %5 = PhiInst %22 : number, %BB1, %0 : number, %BB0
//CHECK-NEXT:   %6 = PhiInst %23 : number, %BB1, %1 : number, %BB0
//CHECK-NEXT:   %7 = PhiInst %24 : number, %BB1, %2 : number, %BB0
//CHECK-NEXT:   %8 = LoadFrameInst [HEAP8@asmFunc] : undefined|object
//CHECK-NEXT:   %9 = LoadFrameInst [HEAPU8@asmFunc] : undefined|object
//CHECK-NEXT:   %10 = CallIntrinsicInst [__uasm.add32_2] : number, %5 : number, 1 : number
//CHECK-NEXT:   %11 = CallIntrinsicInst [__uasm.loadu8_2] : number, %9 : undefined|object, %10 : number
//CHECK-NEXT:   %12 = CallIntrinsicInst [__uasm.mul32_2] : number, %11 : number, 587 : number
//CHECK-NEXT:   %13 = CallIntrinsicInst [__uasm.loadu8_2] : number, %9 : undefined|object, %5 : number
//CHECK-NEXT:   %14 = CallIntrinsicInst [__uasm.mul32_2] : number, %13 : number, 299 : number
//CHECK-NEXT:   %15 = CallIntrinsicInst [__uasm.add32_2] : number, %12 : number, %14 : number
//CHECK-NEXT:   %16 = CallIntrinsicInst [__uasm.add32_2] : number, %5 : number, 2 : number
//CHECK-NEXT:   %17 = CallIntrinsicInst [__uasm.loadu8_2] : number, %9 : undefined|object, %16 : number
//CHECK-NEXT:   %18 = CallIntrinsicInst [__uasm.mul32_2] : number, %17 : number, 114 : number
//CHECK-NEXT:   %19 = CallIntrinsicInst [__uasm.add32_2] : number, %15 : number, %18 : number
//CHECK-NEXT:   %20 = CallIntrinsicInst [__uasm.divu32_2] : number, %19 : number, 1000 : number
//CHECK-NEXT:   %21 = CallIntrinsicInst [__uasm.store8_3] : number, %8 : undefined|object, %6 : number, %20 : number
//CHECK-NEXT:   %22 = CallIntrinsicInst [__uasm.add32_2] : number, %5 : number, 3 : number
//CHECK-NEXT:   %23 = CallIntrinsicInst [__uasm.add32_2] : number, %6 : number, 1 : number
//CHECK-NEXT:   %24 = CallIntrinsicInst [__uasm.sub32_2] : number, %7 : number, 1 : number
//CHECK-NEXT:   %25 = CondBranchInst %24 : number, %BB1, %BB2
//CHECK-NEXT: function_end

 function $2() {
  // const start2 = Date.now();
  var $0_1 = 0, $2_1 = 0, $1_1 = 0, $3_1 = 0;
  $2_1 = global$0 - 4194304 | 0;
  global$0 = $2_1;
  $1_1 = $2_1;
  label$1 : while (1) {
   HEAP8[$1_1 >> 0] = $0_1;
   $3_1 = $0_1 + 1 | 0;
   HEAP8[($1_1 + 1 | 0) >> 0] = Math_imul($3_1, $0_1);
   HEAP8[($1_1 + 2 | 0) >> 0] = Math_imul($0_1 + Math_imul(($0_1 >>> 0) / (5 >>> 0) | 0, -5) | 0, $0_1);
   $1_1 = $1_1 + 3 | 0;
   $0_1 = $3_1;
   if (($0_1 | 0) != (1048576 | 0)) {
    continue label$1
   }
   break label$1;
  };
  $1($2_1 | 0, $2_1 + 3145728 | 0 | 0, 1048576 | 0);
  global$0 = $2_1 + 4194304 | 0;
  // const end2 = Date.now();
  // print("init + compute");
  // print(end2 - start2);
  return 0 | 0;
 }

 function $3($0_1, $1_1) {
  $0_1 = $0_1 | 0;
  $1_1 = $1_1 | 0;
  return $2() | 0 | 0;
 }

 var FUNCTION_TABLE = Table([]);
 function __wasm_memory_size() {
  return buffer.byteLength / 65536 | 0;
 }

 function __wasm_memory_grow(pagesToAdd) {
  pagesToAdd = pagesToAdd | 0;
  var oldPages = __wasm_memory_size() | 0;
  var newPages = oldPages + pagesToAdd | 0;
  if ((oldPages < newPages) && (newPages < 65536)) {
   var newBuffer = new ArrayBuffer(Math_imul(newPages, 65536));
   var newHEAP8 = new Int8Array(newBuffer);
   newHEAP8.set(HEAP8);
   HEAP8 = new Int8Array(newBuffer);
   HEAP16 = new Int16Array(newBuffer);
   HEAP32 = new Int32Array(newBuffer);
   HEAPU8 = new Uint8Array(newBuffer);
   HEAPU16 = new Uint16Array(newBuffer);
   HEAPU32 = new Uint32Array(newBuffer);
   HEAPF32 = new Float32Array(newBuffer);
   HEAPF64 = new Float64Array(newBuffer);
   buffer = newBuffer;
  }
  return oldPages;
 }

 return {
  "memory": Object.create(Object.prototype, {
   "grow": {
    "value": __wasm_memory_grow
   },
   "buffer": {
    "get": function () {
     return buffer;
    }

   }
  }),
  "__wasm_call_ctors": $0,
  "_Z11rgb_to_grayPhS_j": $1,
  "__original_main": $2,
  "main": $3,
  "__main_void": $2,
  "__indirect_function_table": FUNCTION_TABLE,
  "__dso_handle": global$1,
  "__data_end": global$2,
  "__global_base": global$3,
  "__heap_base": global$4,
  "__memory_base": global$5,
  "__table_base": global$6
 };
}

var retasmFunc = asmFunc(  { abort: function() { throw new Error('abort'); }
  });
export var memory = retasmFunc.memory;
export var __wasm_call_ctors = retasmFunc.__wasm_call_ctors;
export var _Z11rgb_to_grayPhS_j = retasmFunc._Z11rgb_to_grayPhS_j;
export var __original_main = retasmFunc.__original_main;
export var main = retasmFunc.main;
export var __main_void = retasmFunc.__main_void;

// Manually grow the memory.
memory.grow(64);
// Invoke.
main(0,0);
