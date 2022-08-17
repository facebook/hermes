/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O -funsafe-intrinsics -target=HBC -dump-postra %s | %FileCheck --match-full-lines --check-prefix=CHKRA %s
// REQUIRES: run_wasm
function asmFunc(env) {
  var buffer = new ArrayBuffer(131072);
  var HEAP8 = new Int8Array(buffer);
  var HEAP16 = new Int16Array(buffer);
  var HEAP32 = new Int32Array(buffer);
  var HEAPU8 = new Uint8Array(buffer);
  var HEAPU16 = new Uint16Array(buffer);
  var HEAPU32 = new Uint32Array(buffer);
  var Math_imul = Math.imul;
  var Math_abs = Math.abs;

  // int32
  function add32(x, y) {
    x = x | 0;
    y = y | 0;
    var z = x + y;
    return (z + x) | 0;
  }
  //CHKRA-LABEL: function add32(x, y) : number
  //CHKRA-NEXT: frame = []
  //CHKRA-NEXT: %BB0:
  //CHKRA-NEXT:   %0 = HBCLoadParamInst 1 : number
  //CHKRA-NEXT:   %1 = AsInt32Inst %0
  //CHKRA-NEXT:   %2 = HBCLoadParamInst 2 : number
  //CHKRA-NEXT:   %3 = AsInt32Inst %2
  //CHKRA-NEXT:   %4 = CallIntrinsicInst [__uasm.add32_2] : number, %1 : number, %3 : number
  //CHKRA-NEXT:   %5 = CallIntrinsicInst [__uasm.add32_2] : number, %4 : number, %1 : number
  //CHKRA-NEXT:   %6 = ReturnInst %5 : number
  //CHKRA-NEXT: function_end

  // uint32
  function add32u(x, y) {
    x = x >>> 0;
    y = y >>> 0;
    var z = x + y;
    return (z + x) | 0;
  }
  //CHKRA-LABEL: function add32u(x, y) : number
  //CHKRA-NEXT: frame = []
  //CHKRA-NEXT: %BB0:
  //CHKRA-NEXT:   %0 = HBCLoadParamInst 1 : number
  //CHKRA-NEXT:   %1 = HBCLoadConstInst 0 : number
  //CHKRA-NEXT:   %2 = BinaryOperatorInst '>>>', %0, %1 : number
  //CHKRA-NEXT:   %3 = HBCLoadParamInst 2 : number
  //CHKRA-NEXT:   %4 = BinaryOperatorInst '>>>', %3, %1 : number
  //CHKRA-NEXT:   %5 = CallIntrinsicInst [__uasm.add32_2] : number, %2 : number, %4 : number
  //CHKRA-NEXT:   %6 = CallIntrinsicInst [__uasm.add32_2] : number, %5 : number, %2 : number
  //CHKRA-NEXT:   %7 = ReturnInst %6 : number
  //CHKRA-NEXT: function_end

  // int32 adding a literal number in Wasm mode should be treated as int32
  function add32l(x, y) {
    x = x | 0;
    y = y | 0;
    var z = x + y + 42;
    return (10086 + z) | 0;
  }
  //CHKRA-LABEL: function add32l(x, y) : number
  //CHKRA-NEXT: frame = []
  //CHKRA-NEXT: %BB0:
  //CHKRA-NEXT:   %0 = HBCLoadParamInst 1 : number
  //CHKRA-NEXT:   %1 = AsInt32Inst %0
  //CHKRA-NEXT:   %2 = HBCLoadParamInst 2 : number
  //CHKRA-NEXT:   %3 = AsInt32Inst %2
  //CHKRA-NEXT:   %4 = HBCLoadConstInst 10086 : number
  //CHKRA-NEXT:   %5 = CallIntrinsicInst [__uasm.add32_2] : number, %1 : number, %3 : number
  //CHKRA-NEXT:   %6 = HBCLoadConstInst 42 : number
  //CHKRA-NEXT:   %7 = CallIntrinsicInst [__uasm.add32_2] : number, %5 : number, %6 : number
  //CHKRA-NEXT:   %8 = CallIntrinsicInst [__uasm.add32_2] : number, %4 : number, %7 : number
  //CHKRA-NEXT:   %9 = ReturnInst %8 : number
  //CHKRA-NEXT: function_end

  function add32lf(x, y) {
    x = x | 0;
    y = y | 0;
    return (((x + y ) | 0) + 4.2) | 0;
  }
  //CHKRA-LABEL: function add32lf(x, y) : number
  //CHKRA-NEXT: frame = []
  //CHKRA-NEXT: %BB0:
  //CHKRA-NEXT:   %0 = HBCLoadParamInst 1 : number
  //CHKRA-NEXT:   %1 = AsInt32Inst %0
  //CHKRA-NEXT:   %2 = HBCLoadParamInst 2 : number
  //CHKRA-NEXT:   %3 = AsInt32Inst %2
  //CHKRA-NEXT:   %4 = CallIntrinsicInst [__uasm.add32_2] : number, %1 : number, %3 : number
  //CHKRA-NEXT:   %5 = HBCLoadConstInst 4.2 : number
  //CHKRA-NEXT:   %6 = BinaryOperatorInst '+', %4 : number, %5 : number
  //CHKRA-NEXT:   %7 = AsInt32Inst %6 : number
  //CHKRA-NEXT:   %8 = ReturnInst %7 : number
  //CHKRA-NEXT: function_end

  // Regular add
  function add32n(x, y) {
    var z = x + y;
    return (z + x) | 0;
  }
  //CHKRA-LABEL: function add32n(x, y) : number
  //CHKRA-NEXT: frame = []
  //CHKRA-NEXT: %BB0:
  //CHKRA-NEXT:   %0 = HBCLoadParamInst 1 : number
  //CHKRA-NEXT:   %1 = HBCLoadParamInst 2 : number
  //CHKRA-NEXT:   %2 = BinaryOperatorInst '+', %0, %1
  //CHKRA-NEXT:   %3 = BinaryOperatorInst '+', %2 : string|number, %0
  //CHKRA-NEXT:   %4 = AsInt32Inst %3 : string|number
  //CHKRA-NEXT:   %5 = ReturnInst %4 : number
  //CHKRA-NEXT: function_end

  // Phi node and local variable
  function local() {
    var x = 0;
    var i = 0;
    label$1: while (1) {
      x = x + i | 0;
      i = i + 1 | 0;
      if ((i | 0) != (10 | 0)) {
        continue label$1;
      }
      break label$1;
    }
    return x;
  }
  //CHKRA-LABEL: function local() : number
  //CHKRA-NEXT: frame = []
  //CHKRA-NEXT: %BB0:
  //CHKRA-NEXT:   %0 = HBCLoadConstInst 0 : number
  //CHKRA-NEXT:   %1 = HBCLoadConstInst 1 : number
  //CHKRA-NEXT:   %2 = HBCLoadConstInst 10 : number
  //CHKRA-NEXT:   %3 = HBCLoadConstInst 0 : number
  //CHKRA-NEXT:   %4 = BranchInst %BB1
  //CHKRA-NEXT: %BB2:
  //CHKRA-NEXT:   %5 = ReturnInst %8 : number
  //CHKRA-NEXT: %BB1:
  //CHKRA-NEXT:   %6 = PhiInst %0 : number, %BB0, %10 : number, %BB1
  //CHKRA-NEXT:   %7 = PhiInst %3 : number, %BB0, %11 : number, %BB1
  //CHKRA-NEXT:   %8 = CallIntrinsicInst [__uasm.add32_2] : number, %6 : number, %7 : number
  //CHKRA-NEXT:   %9 = CallIntrinsicInst [__uasm.add32_2] : number, %7 : number, %1 : number
  //CHKRA-NEXT:   %10 = MovInst %8 : number
  //CHKRA-NEXT:   %11 = MovInst %9 : number
  //CHKRA-NEXT:   %12 = CompareBranchInst '!==', %11 : number, %2 : number, %BB1, %BB2
  //CHKRA-NEXT: function_end

  // int32 sub
  function sub32(x, y) {
    x = x | 0;
    y = y | 0;
    var z = x - y;
    return (z - x) | 0;
  }
  //CHKRA-LABEL: function sub32(x, y) : number
  //CHKRA-NEXT: frame = []
  //CHKRA-NEXT: %BB0:
  //CHKRA-NEXT:   %0 = HBCLoadParamInst 1 : number
  //CHKRA-NEXT:   %1 = AsInt32Inst %0
  //CHKRA-NEXT:   %2 = HBCLoadParamInst 2 : number
  //CHKRA-NEXT:   %3 = AsInt32Inst %2
  //CHKRA-NEXT:   %4 = CallIntrinsicInst [__uasm.sub32_2] : number, %1 : number, %3 : number
  //CHKRA-NEXT:   %5 = CallIntrinsicInst [__uasm.sub32_2] : number, %4 : number, %1 : number
  //CHKRA-NEXT:   %6 = ReturnInst %5 : number
  //CHKRA-NEXT: function_end

  // divi
  function divi32(x, y) {
    x = x | 0;
    y = y | 0;
    var z = x / y;
    return (z / x) | 0;
  }
  //CHKRA-LABEL: function divi32(x, y) : number
  //CHKRA-NEXT: frame = []
  //CHKRA-NEXT: %BB0:
  //CHKRA-NEXT:   %0 = HBCLoadParamInst 1 : number
  //CHKRA-NEXT:   %1 = AsInt32Inst %0
  //CHKRA-NEXT:   %2 = HBCLoadParamInst 2 : number
  //CHKRA-NEXT:   %3 = AsInt32Inst %2
  //CHKRA-NEXT:   %4 = CallIntrinsicInst [__uasm.divi32_2] : number, %1 : number, %3 : number
  //CHKRA-NEXT:   %5 = CallIntrinsicInst [__uasm.divi32_2] : number, %4 : number, %1 : number
  //CHKRA-NEXT:   %6 = ReturnInst %5 : number
  //CHKRA-NEXT: function_end

  // divu
  function divu32(x, y) {
    x = x | 0;
    y = y | 0;
    return ((x >>> 0) / (y >>> 0)) | 0;
  }
  //CHKRA-LABEL: function divu32(x, y) : number
  //CHKRA-NEXT: frame = []
  //CHKRA-NEXT: %BB0:
  //CHKRA-NEXT:   %0 = HBCLoadParamInst 1 : number
  //CHKRA-NEXT:   %1 = AsInt32Inst %0
  //CHKRA-NEXT:   %2 = HBCLoadParamInst 2 : number
  //CHKRA-NEXT:   %3 = AsInt32Inst %2
  //CHKRA-NEXT:   %4 = CallIntrinsicInst [__uasm.divu32_2] : number, %1 : number, %3 : number
  //CHKRA-NEXT:   %5 = ReturnInst %4 : number
  //CHKRA-NEXT: function_end

  // Math_imul
  function mul32(x, y) {
    x = x | 0;
    y = y | 0;
    return Math_imul(x, y);
  }
  //CHKRA-LABEL: function mul32(x, y) : number
  //CHKRA-NEXT: frame = []
  //CHKRA-NEXT: %BB0:
  //CHKRA-NEXT:   %0 = HBCLoadParamInst 1 : number
  //CHKRA-NEXT:   %1 = AsInt32Inst %0
  //CHKRA-NEXT:   %2 = HBCLoadParamInst 2 : number
  //CHKRA-NEXT:   %3 = AsInt32Inst %2
  //CHKRA-NEXT:   %4 = CallIntrinsicInst [__uasm.mul32_2] : number, %1 : number, %3 : number
  //CHKRA-NEXT:   %5 = ReturnInst %4 : number
  //CHKRA-NEXT: function_end

  // Should not be replaced.
  function math_abs(x) {
    x = x | 0;
    return Math_abs(x);
  }
  //CHKRA-LABEL: function math_abs(x)
  //CHKRA-NEXT: frame = []
  //CHKRA-NEXT: %BB0:
  //CHKRA-NEXT:   %0 = HBCLoadParamInst 1 : number
  //CHKRA-NEXT:   %1 = AsInt32Inst %0
  //CHKRA-NEXT:   %2 = HBCResolveEnvironment %asmFunc()
  //CHKRA-NEXT:   %3 = HBCLoadFromEnvironmentInst %2, [Math_abs@asmFunc]
  //CHKRA-NEXT:   %4 = HBCLoadConstInst undefined : undefined
  //CHKRA-NEXT:   %5 = ImplicitMovInst %4 : undefined
  //CHKRA-NEXT:   %6 = ImplicitMovInst %1 : number
  //CHKRA-NEXT:   %7 = HBCCallNInst %3, %4 : undefined, %1 : number
  //CHKRA-NEXT:   %8 = ReturnInst %7
  //CHKRA-NEXT: function_end

  function loadi8(addr) {
    addr = addr | 0;
    return HEAP8[addr >> 0];
  }
  //CHKRA-LABEL: function loadi8(addr) : number
  //CHKRA-NEXT: frame = []
  //CHKRA-NEXT: %BB0:
  //CHKRA-NEXT:   %0 = HBCLoadParamInst 1 : number
  //CHKRA-NEXT:   %1 = AsInt32Inst %0
  //CHKRA-NEXT:   %2 = HBCResolveEnvironment %asmFunc()
  //CHKRA-NEXT:   %3 = HBCLoadFromEnvironmentInst %2, [HEAP8@asmFunc] : undefined|object
  //CHKRA-NEXT:   %4 = CallIntrinsicInst [__uasm.loadi8_2] : number, %3, %1 : number
  //CHKRA-NEXT:   %5 = ReturnInst %4 : number
  //CHKRA-NEXT: function_end

  function loadu8(addr) {
    addr = addr | 0;
    return HEAPU8[addr >> 0];
  }
  //CHKRA-LABEL: function loadu8(addr) : number
  //CHKRA-NEXT: frame = []
  //CHKRA-NEXT: %BB0:
  //CHKRA-NEXT:   %0 = HBCLoadParamInst 1 : number
  //CHKRA-NEXT:   %1 = AsInt32Inst %0
  //CHKRA-NEXT:   %2 = HBCResolveEnvironment %asmFunc()
  //CHKRA-NEXT:   %3 = HBCLoadFromEnvironmentInst %2, [HEAPU8@asmFunc] : undefined|object
  //CHKRA-NEXT:   %4 = CallIntrinsicInst [__uasm.loadu8_2] : number, %3, %1 : number
  //CHKRA-NEXT:   %5 = ReturnInst %4 : number
  //CHKRA-NEXT: function_end

  function loadi16(addr) {
    addr = addr | 0;
    return HEAP16[addr >> 1];
  }
  //CHKRA-LABEL: function loadi16(addr) : number
  //CHKRA-NEXT: frame = []
  //CHKRA-NEXT: %BB0:
  //CHKRA-NEXT:   %0 = HBCLoadParamInst 1 : number
  //CHKRA-NEXT:   %1 = AsInt32Inst %0
  //CHKRA-NEXT:   %2 = HBCResolveEnvironment %asmFunc()
  //CHKRA-NEXT:   %3 = HBCLoadFromEnvironmentInst %2, [HEAP16@asmFunc] : undefined|object
  //CHKRA-NEXT:   %4 = CallIntrinsicInst [__uasm.loadi16_2] : number, %3, %1 : number
  //CHKRA-NEXT:   %5 = ReturnInst %4 : number
  //CHKRA-NEXT: function_end

  function loadu16(addr) {
    addr = addr | 0;
    return HEAPU16[addr >> 1];
  }
  //CHKRA-LABEL: function loadu16(addr) : number
  //CHKRA-NEXT: frame = []
  //CHKRA-NEXT: %BB0:
  //CHKRA-NEXT:   %0 = HBCLoadParamInst 1 : number
  //CHKRA-NEXT:   %1 = AsInt32Inst %0
  //CHKRA-NEXT:   %2 = HBCResolveEnvironment %asmFunc()
  //CHKRA-NEXT:   %3 = HBCLoadFromEnvironmentInst %2, [HEAPU16@asmFunc] : undefined|object
  //CHKRA-NEXT:   %4 = CallIntrinsicInst [__uasm.loadu16_2] : number, %3, %1 : number
  //CHKRA-NEXT:   %5 = ReturnInst %4 : number
  //CHKRA-NEXT: function_end

  function loadi32(addr) {
    addr = addr | 0;
    return HEAP32[addr >> 2];
  }
  //CHKRA-LABEL: function loadi32(addr) : number
  //CHKRA-NEXT: frame = []
  //CHKRA-NEXT: %BB0:
  //CHKRA-NEXT:   %0 = HBCLoadParamInst 1 : number
  //CHKRA-NEXT:   %1 = AsInt32Inst %0
  //CHKRA-NEXT:   %2 = HBCResolveEnvironment %asmFunc()
  //CHKRA-NEXT:   %3 = HBCLoadFromEnvironmentInst %2, [HEAP32@asmFunc] : undefined|object
  //CHKRA-NEXT:   %4 = CallIntrinsicInst [__uasm.loadi32_2] : number, %3, %1 : number
  //CHKRA-NEXT:   %5 = ReturnInst %4 : number
  //CHKRA-NEXT: function_end

  function loadu32(addr) {
    addr = addr | 0;
    return HEAPU32[addr >> 2];
  }
  //CHKRA-LABEL: function loadu32(addr) : number
  //CHKRA-NEXT: frame = []
  //CHKRA-NEXT: %BB0:
  //CHKRA-NEXT:   %0 = HBCLoadParamInst 1 : number
  //CHKRA-NEXT:   %1 = AsInt32Inst %0
  //CHKRA-NEXT:   %2 = HBCResolveEnvironment %asmFunc()
  //CHKRA-NEXT:   %3 = HBCLoadFromEnvironmentInst %2, [HEAPU32@asmFunc] : undefined|object
  //CHKRA-NEXT:   %4 = CallIntrinsicInst [__uasm.loadu32_2] : number, %3, %1 : number
  //CHKRA-NEXT:   %5 = ReturnInst %4 : number
  //CHKRA-NEXT: function_end

  function store8(addr, data) {
    addr = addr | 0;
    HEAP8[addr >> 0] = data;
    HEAPU8[addr >> 0] = data;
  }
  //CHKRA-LABEL: function store8(addr, data) : undefined
  //CHKRA-NEXT: frame = []
  //CHKRA-NEXT: %BB0:
  //CHKRA-NEXT:   %0 = HBCLoadParamInst 2 : number
  //CHKRA-NEXT:   %1 = HBCLoadParamInst 1 : number
  //CHKRA-NEXT:   %2 = AsInt32Inst %1
  //CHKRA-NEXT:   %3 = HBCResolveEnvironment %asmFunc()
  //CHKRA-NEXT:   %4 = HBCLoadFromEnvironmentInst %3, [HEAP8@asmFunc] : undefined|object
  //CHKRA-NEXT:   %5 = CallIntrinsicInst [__uasm.store8_3] : number, %4, %2 : number, %0
  //CHKRA-NEXT:   %6 = HBCLoadFromEnvironmentInst %3, [HEAPU8@asmFunc] : undefined|object
  //CHKRA-NEXT:   %7 = CallIntrinsicInst [__uasm.store8_3] : number, %6, %2 : number, %0
  //CHKRA-NEXT:   %8 = HBCLoadConstInst undefined : undefined
  //CHKRA-NEXT:   %9 = ReturnInst %8 : undefined
  //CHKRA-NEXT: function_end

  function store16(addr, data) {
    addr = addr | 0;
    HEAP16[addr >> 1] = data;
    HEAPU16[addr >> 1] = data;
  }
  //CHKRA-LABEL: function store16(addr, data) : undefined
  //CHKRA-NEXT: frame = []
  //CHKRA-NEXT: %BB0:
  //CHKRA-NEXT:   %0 = HBCLoadParamInst 2 : number
  //CHKRA-NEXT:   %1 = HBCLoadParamInst 1 : number
  //CHKRA-NEXT:   %2 = AsInt32Inst %1
  //CHKRA-NEXT:   %3 = HBCResolveEnvironment %asmFunc()
  //CHKRA-NEXT:   %4 = HBCLoadFromEnvironmentInst %3, [HEAP16@asmFunc] : undefined|object
  //CHKRA-NEXT:   %5 = CallIntrinsicInst [__uasm.store16_3] : number, %4, %2 : number, %0
  //CHKRA-NEXT:   %6 = HBCLoadFromEnvironmentInst %3, [HEAPU16@asmFunc] : undefined|object
  //CHKRA-NEXT:   %7 = CallIntrinsicInst [__uasm.store16_3] : number, %6, %2 : number, %0
  //CHKRA-NEXT:   %8 = HBCLoadConstInst undefined : undefined
  //CHKRA-NEXT:   %9 = ReturnInst %8 : undefined
  //CHKRA-NEXT: function_end

  function store32(addr, data) {
    addr = addr | 0;
    HEAP32[addr >> 2] = data;
    HEAPU32[addr >> 2] = data;
  }
  //CHKRA-LABEL: function store32(addr, data) : undefined
  //CHKRA-NEXT: frame = []
  //CHKRA-NEXT: %BB0:
  //CHKRA-NEXT:   %0 = HBCLoadParamInst 2 : number
  //CHKRA-NEXT:   %1 = HBCLoadParamInst 1 : number
  //CHKRA-NEXT:   %2 = AsInt32Inst %1
  //CHKRA-NEXT:   %3 = HBCResolveEnvironment %asmFunc()
  //CHKRA-NEXT:   %4 = HBCLoadFromEnvironmentInst %3, [HEAP32@asmFunc] : undefined|object
  //CHKRA-NEXT:   %5 = CallIntrinsicInst [__uasm.store32_3] : number, %4, %2 : number, %0
  //CHKRA-NEXT:   %6 = HBCLoadFromEnvironmentInst %3, [HEAPU32@asmFunc] : undefined|object
  //CHKRA-NEXT:   %7 = CallIntrinsicInst [__uasm.store32_3] : number, %6, %2 : number, %0
  //CHKRA-NEXT:   %8 = HBCLoadConstInst undefined : undefined
  //CHKRA-NEXT:   %9 = ReturnInst %8 : undefined
  //CHKRA-NEXT: function_end

  function store32cse(addr, data) {
    addr = addr | 0;
    HEAP32[addr >> 2] = data;
    HEAPU32[addr >> 2] = data;
    return addr >> 2;
  }
  //CHKRA-LABEL: function store32cse(addr, data) : number
  //CHKRA-NEXT: frame = []
  //CHKRA-NEXT: %BB0:
  //CHKRA-NEXT:   %0 = HBCLoadParamInst 2 : number
  //CHKRA-NEXT:   %1 = HBCLoadParamInst 1 : number
  //CHKRA-NEXT:   %2 = AsInt32Inst %1
  //CHKRA-NEXT:   %3 = HBCResolveEnvironment %asmFunc()
  //CHKRA-NEXT:   %4 = HBCLoadFromEnvironmentInst %3, [HEAP32@asmFunc] : undefined|object
  //CHKRA-NEXT:   %5 = CallIntrinsicInst [__uasm.store32_3] : number, %4, %2 : number, %0
  //CHKRA-NEXT:   %6 = HBCLoadFromEnvironmentInst %3, [HEAPU32@asmFunc] : undefined|object
  //CHKRA-NEXT:   %7 = CallIntrinsicInst [__uasm.store32_3] : number, %6, %2 : number, %0
  //CHKRA-NEXT:   %8 = HBCLoadConstInst 2 : number
  //CHKRA-NEXT:   %9 = BinaryOperatorInst '>>', %2 : number, %8 : number
  //CHKRA-NEXT:   %10 = ReturnInst %9 : number
  //CHKRA-NEXT: function_end

  // The case where >> 2 is missing or have a different amount.
  function incorrect_load32(addr, data) {
    return HEAP32[addr >> 3] + HEAP32[addr] + HEAP32[addr + 3];
  }
  //CHKRA-LABEL: function incorrect_load32(addr, data) : string|number
  //CHKRA-NEXT: frame = []
  //CHKRA-NEXT: %BB0:
  //CHKRA-NEXT:   %0 = HBCLoadParamInst 1 : number
  //CHKRA-NEXT:   %1 = HBCResolveEnvironment %asmFunc()
  //CHKRA-NEXT:   %2 = HBCLoadFromEnvironmentInst %1, [HEAP32@asmFunc] : undefined|object
  //CHKRA-NEXT:   %3 = HBCLoadConstInst 3 : number
  //CHKRA-NEXT:   %4 = BinaryOperatorInst '>>', %0, %3 : number
  //CHKRA-NEXT:   %5 = LoadPropertyInst %2, %4 : number
  //CHKRA-NEXT:   %6 = HBCLoadFromEnvironmentInst %1, [HEAP32@asmFunc] : undefined|object
  //CHKRA-NEXT:   %7 = LoadPropertyInst %6, %0
  //CHKRA-NEXT:   %8 = BinaryOperatorInst '+', %5, %7
  //CHKRA-NEXT:   %9 = HBCLoadFromEnvironmentInst %1, [HEAP32@asmFunc] : undefined|object
  //CHKRA-NEXT:   %10 = BinaryOperatorInst '+', %0, %3 : number
  //CHKRA-NEXT:   %11 = LoadPropertyInst %9, %10 : string|number
  //CHKRA-NEXT:   %12 = BinaryOperatorInst '+', %8 : string|number, %11
  //CHKRA-NEXT:   %13 = ReturnInst %12 : string|number
  //CHKRA-NEXT: function_end

  return {
    "add32" : add32,
    "add32u" : add32u,
    "add32l" : add32l,
    "add32lf" : add32lf,
    "add32n" : add32n,
    "local" : local,
    "sub32" : sub32,
    "divi32" : divi32,
    "divu32" : divu32,
    "mul32" : mul32,
    "math_abs" : math_abs,
    "loadi8" : loadi8,
    "loadu8" : loadu8,
    "loadi16" : loadi16,
    "loadu16" : loadu16,
    "loadi32" : loadi32,
    "loadu32" : loadu32,
    "store8" : store8,
    "store16" : store16,
    "store32" : store32,
    "store32cse" : store32cse,
    "incorrect" : incorrect_load32
  };
}
