/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes --target=HBC -dump-lir -O %s | %FileCheckOrRegen %s --match-full-lines --check-prefix=IRGEN
// RUN: %hermes --target=HBC -dump-bytecode -O %s | %FileCheckOrRegen %s --match-full-lines --check-prefix=BCGEN

function obj1() {
  return {'a': 'hello', 'b': 1, 'c': null, 'd': undefined, 'e': true, 'f': function() {}, 'g': 2};
}

// Objects are always fully commited into the buffer.
function obj2() {
  return {
    a : undefined,
    b : undefined,
    c : undefined,
    d : undefined,
    e : undefined,
    r : undefined,
    f : 1,
    g : 1,
    h : 1,
    i : 1,
    j : 1,
    k : 1,
    l : 1,
    m : 1,
    n : 1,
    o : 1,
    p : 1,
    q : 1,
  };
}

// Unserializable numeric key is admitted into the buffer.
function obj3() {
  return {
    1 : undefined,
    f : 1,
    g : 1,
    h : 1,
    i : 1,
    j : 1,
    k : 1,
    l : 1,
    m : 1,
    n : 1,
    o : 1,
    p : 1,
    q : 1,
  }
}

function obj4() {
  return {
    '1' : undefined,
    f : 1,
    g : 1,
    h : 1,
    i : 1,
    j : 1,
    k : 1,
    l : 1,
    m : 1,
    n : 1,
    o : 1,
    p : 1,
    q : 1,
  };
}

// Numeric keys containing constant values interleaved with nonnumeric keys.
function obj5() {
  return {
    '1': 50,
    'a': 1,
    '3': 50,
    'b': 2,
    '2': 50,
    'c': 3,
  };
}

// Numeric keys containing unserializable values interleaved with nonnumeric keys.
function obj6() {
  return {
    '1': undefined,
    'a': 1,
    '3': undefined,
    'b': 2,
    '2': undefined,
    'c': 3,
  };
}

// Numeric keys containing constant values interleaved with nonnumeric keys of unserializable values.
function obj7() {
  return {
    '1': 1,
    'a': undefined,
    '3': 2,
    'b': undefined,
    '2': 3,
    'c': undefined,
  };
}

// Auto-generated content below. Please do not modify manually.

// IRGEN:function global(): undefined
// IRGEN-NEXT:%BB0:
// IRGEN-NEXT:       DeclareGlobalVarInst "obj1": string
// IRGEN-NEXT:       DeclareGlobalVarInst "obj2": string
// IRGEN-NEXT:       DeclareGlobalVarInst "obj3": string
// IRGEN-NEXT:       DeclareGlobalVarInst "obj4": string
// IRGEN-NEXT:       DeclareGlobalVarInst "obj5": string
// IRGEN-NEXT:       DeclareGlobalVarInst "obj6": string
// IRGEN-NEXT:       DeclareGlobalVarInst "obj7": string
// IRGEN-NEXT:  %7 = LIRGetGlobalObjectInst (:object)
// IRGEN-NEXT:  %8 = LIRLoadConstInst (:undefined) undefined: undefined
// IRGEN-NEXT:  %9 = CreateFunctionInst (:object) %8: undefined, empty: any, %obj1(): functionCode
// IRGEN-NEXT:        StorePropertyLooseInst %9: object, %7: object, "obj1": string
// IRGEN-NEXT:  %11 = CreateFunctionInst (:object) %8: undefined, empty: any, %obj2(): functionCode
// IRGEN-NEXT:        StorePropertyLooseInst %11: object, %7: object, "obj2": string
// IRGEN-NEXT:  %13 = CreateFunctionInst (:object) %8: undefined, empty: any, %obj3(): functionCode
// IRGEN-NEXT:        StorePropertyLooseInst %13: object, %7: object, "obj3": string
// IRGEN-NEXT:  %15 = CreateFunctionInst (:object) %8: undefined, empty: any, %obj4(): functionCode
// IRGEN-NEXT:        StorePropertyLooseInst %15: object, %7: object, "obj4": string
// IRGEN-NEXT:  %17 = CreateFunctionInst (:object) %8: undefined, empty: any, %obj5(): functionCode
// IRGEN-NEXT:        StorePropertyLooseInst %17: object, %7: object, "obj5": string
// IRGEN-NEXT:  %19 = CreateFunctionInst (:object) %8: undefined, empty: any, %obj6(): functionCode
// IRGEN-NEXT:        StorePropertyLooseInst %19: object, %7: object, "obj6": string
// IRGEN-NEXT:  %21 = CreateFunctionInst (:object) %8: undefined, empty: any, %obj7(): functionCode
// IRGEN-NEXT:        StorePropertyLooseInst %21: object, %7: object, "obj7": string
// IRGEN-NEXT:        ReturnInst %8: undefined
// IRGEN-NEXT:function_end

// IRGEN:function obj1(): object
// IRGEN-NEXT:%BB0:
// IRGEN-NEXT:  %0 = LIRAllocObjectFromBufferInst (:object) empty: any, "a": string, "hello": string, "b": string, 1: number, "c": string, null: null, "d": string, undefined: undefined, "e": string, true: boolean, "f": string, null: null, "g": string, 2: number
// IRGEN-NEXT:  %1 = LIRLoadConstInst (:undefined) undefined: undefined
// IRGEN-NEXT:  %2 = CreateFunctionInst (:object) %1: undefined, empty: any, %f(): functionCode
// IRGEN-NEXT:       PrStoreInst %2: object, %0: object, 5: number, "f": string, false: boolean
// IRGEN-NEXT:       ReturnInst %0: object
// IRGEN-NEXT:function_end

// IRGEN:function obj2(): object
// IRGEN-NEXT:%BB0:
// IRGEN-NEXT:  %0 = LIRAllocObjectFromBufferInst (:object) empty: any, "a": string, undefined: undefined, "b": string, undefined: undefined, "c": string, undefined: undefined, "d": string, undefined: undefined, "e": string, undefined: undefined, "r": string, undefined: undefined, "f": string, 1: number, "g": string, 1: number, "h": string, 1: number, "i": string, 1: number, "j": string, 1: number, "k": string, 1: number, "l": string, 1: number, "m": string, 1: number, "n": string, 1: number, "o": string, 1: number, "p": string, 1: number, "q": string, 1: number
// IRGEN-NEXT:       ReturnInst %0: object
// IRGEN-NEXT:function_end

// IRGEN:function obj3(): object
// IRGEN-NEXT:%BB0:
// IRGEN-NEXT:  %0 = LIRAllocObjectFromBufferInst (:object) empty: any, 1: number, undefined: undefined, "f": string, 1: number, "g": string, 1: number, "h": string, 1: number, "i": string, 1: number, "j": string, 1: number, "k": string, 1: number, "l": string, 1: number, "m": string, 1: number, "n": string, 1: number, "o": string, 1: number, "p": string, 1: number, "q": string, 1: number
// IRGEN-NEXT:       ReturnInst %0: object
// IRGEN-NEXT:function_end

// IRGEN:function obj4(): object
// IRGEN-NEXT:%BB0:
// IRGEN-NEXT:  %0 = LIRAllocObjectFromBufferInst (:object) empty: any, 1: number, undefined: undefined, "f": string, 1: number, "g": string, 1: number, "h": string, 1: number, "i": string, 1: number, "j": string, 1: number, "k": string, 1: number, "l": string, 1: number, "m": string, 1: number, "n": string, 1: number, "o": string, 1: number, "p": string, 1: number, "q": string, 1: number
// IRGEN-NEXT:       ReturnInst %0: object
// IRGEN-NEXT:function_end

// IRGEN:function obj5(): object
// IRGEN-NEXT:%BB0:
// IRGEN-NEXT:  %0 = LIRAllocObjectFromBufferInst (:object) empty: any, 1: number, 50: number, "a": string, 1: number, 3: number, 50: number, "b": string, 2: number, 2: number, 50: number, "c": string, 3: number
// IRGEN-NEXT:       ReturnInst %0: object
// IRGEN-NEXT:function_end

// IRGEN:function obj6(): object
// IRGEN-NEXT:%BB0:
// IRGEN-NEXT:  %0 = LIRAllocObjectFromBufferInst (:object) empty: any, 1: number, undefined: undefined, "a": string, 1: number, 3: number, undefined: undefined, "b": string, 2: number, 2: number, undefined: undefined, "c": string, 3: number
// IRGEN-NEXT:       ReturnInst %0: object
// IRGEN-NEXT:function_end

// IRGEN:function obj7(): object
// IRGEN-NEXT:%BB0:
// IRGEN-NEXT:  %0 = LIRAllocObjectFromBufferInst (:object) empty: any, 1: number, 1: number, "a": string, undefined: undefined, 3: number, 2: number, "b": string, undefined: undefined, 2: number, 3: number, "c": string, undefined: undefined
// IRGEN-NEXT:       ReturnInst %0: object
// IRGEN-NEXT:function_end

// IRGEN:function f(): undefined
// IRGEN-NEXT:%BB0:
// IRGEN-NEXT:  %0 = LIRLoadConstInst (:undefined) undefined: undefined
// IRGEN-NEXT:       ReturnInst %0: undefined
// IRGEN-NEXT:function_end

// BCGEN:Bytecode File Information:
// BCGEN-NEXT:  Bytecode version number: {{.*}}
// BCGEN-NEXT:  Source hash: {{.*}}
// BCGEN-NEXT:  Function count: 9
// BCGEN-NEXT:  String count: 27
// BCGEN-NEXT:  BigInt count: 0
// BCGEN-NEXT:  String Kind Entry count: 2
// BCGEN-NEXT:  RegExp count: 0
// BCGEN-NEXT:  Segment ID: 0
// BCGEN-NEXT:  CommonJS module count: 0
// BCGEN-NEXT:  CommonJS module count (static): 0
// BCGEN-NEXT:  Function source count: 0
// BCGEN-NEXT:  Bytecode options:
// BCGEN-NEXT:    staticBuiltins: 0
// BCGEN-NEXT:    cjsModulesStaticallyResolved: 0

// BCGEN:Global String Table:
// BCGEN-NEXT:s0[ASCII, 0..5]: global
// BCGEN-NEXT:s1[ASCII, 6..10]: hello
// BCGEN-NEXT:i2[ASCII, 0..0] #00019A16: g
// BCGEN-NEXT:i3[ASCII, 4..4] #00018270: a
// BCGEN-NEXT:i4[ASCII, 5..5] #0001B6AD: l
// BCGEN-NEXT:i5[ASCII, 6..6] #0001A6E9: h
// BCGEN-NEXT:i6[ASCII, 10..10] #0001BA9E: o
// BCGEN-NEXT:i7[ASCII, 10..13] #2B6CD370: obj1
// BCGEN-NEXT:i8[ASCII, 12..12] #0001AECB: j
// BCGEN-NEXT:i9[ASCII, 14..14] #00018E43: b
// BCGEN-NEXT:i10[ASCII, 15..15] #00018A52: c
// BCGEN-NEXT:i11[ASCII, 16..16] #00019625: d
// BCGEN-NEXT:i12[ASCII, 17..17] #00019234: e
// BCGEN-NEXT:i13[ASCII, 18..18] #00019E07: f
// BCGEN-NEXT:i14[ASCII, 19..19] #0001A2F8: i
// BCGEN-NEXT:i15[ASCII, 20..20] #0001AADA: k
// BCGEN-NEXT:i16[ASCII, 21..21] #0001B2BC: m
// BCGEN-NEXT:i17[ASCII, 22..22] #0001BE8F: n
// BCGEN-NEXT:i18[ASCII, 23..26] #2B6CEF81: obj2
// BCGEN-NEXT:i19[ASCII, 27..30] #2B6CEB92: obj3
// BCGEN-NEXT:i20[ASCII, 31..34] #2B6CE7A3: obj4
// BCGEN-NEXT:i21[ASCII, 35..38] #2B6CE3B4: obj5
// BCGEN-NEXT:i22[ASCII, 39..42] #2B6CFFC5: obj6
// BCGEN-NEXT:i23[ASCII, 43..46] #2B6CFBD6: obj7
// BCGEN-NEXT:i24[ASCII, 47..47] #0001C771: p
// BCGEN-NEXT:i25[ASCII, 48..48] #0001C360: q
// BCGEN-NEXT:i26[ASCII, 49..49] #0001CF53: r

// BCGEN:Literal Value Buffer:
// BCGEN-NEXT:[String 1]
// BCGEN-NEXT:[int 1]
// BCGEN-NEXT:null
// BCGEN-NEXT:undefined
// BCGEN-NEXT:true
// BCGEN-NEXT:null
// BCGEN-NEXT:[int 2]
// BCGEN-NEXT:undefined
// BCGEN-NEXT:undefined
// BCGEN-NEXT:undefined
// BCGEN-NEXT:undefined
// BCGEN-NEXT:undefined
// BCGEN-NEXT:undefined
// BCGEN-NEXT:[int 1]
// BCGEN-NEXT:[int 1]
// BCGEN-NEXT:[int 1]
// BCGEN-NEXT:[int 1]
// BCGEN-NEXT:[int 1]
// BCGEN-NEXT:[int 1]
// BCGEN-NEXT:[int 1]
// BCGEN-NEXT:[int 1]
// BCGEN-NEXT:[int 1]
// BCGEN-NEXT:[int 1]
// BCGEN-NEXT:[int 1]
// BCGEN-NEXT:[int 1]
// BCGEN-NEXT:undefined
// BCGEN-NEXT:[int 1]
// BCGEN-NEXT:undefined
// BCGEN-NEXT:[int 2]
// BCGEN-NEXT:undefined
// BCGEN-NEXT:[int 3]
// BCGEN-NEXT:undefined
// BCGEN-NEXT:[int 1]
// BCGEN-NEXT:[int 1]
// BCGEN-NEXT:[int 1]
// BCGEN-NEXT:[int 1]
// BCGEN-NEXT:[int 1]
// BCGEN-NEXT:[int 1]
// BCGEN-NEXT:[int 1]
// BCGEN-NEXT:[int 1]
// BCGEN-NEXT:[int 1]
// BCGEN-NEXT:[int 1]
// BCGEN-NEXT:[int 1]
// BCGEN-NEXT:[int 1]
// BCGEN-NEXT:[int 50]
// BCGEN-NEXT:[int 1]
// BCGEN-NEXT:[int 50]
// BCGEN-NEXT:[int 2]
// BCGEN-NEXT:[int 50]
// BCGEN-NEXT:[int 3]
// BCGEN-NEXT:Object Key Buffer:
// BCGEN-NEXT:[String 3]
// BCGEN-NEXT:[String 9]
// BCGEN-NEXT:[String 10]
// BCGEN-NEXT:[String 11]
// BCGEN-NEXT:[String 12]
// BCGEN-NEXT:[String 13]
// BCGEN-NEXT:[String 2]
// BCGEN-NEXT:[String 3]
// BCGEN-NEXT:[String 9]
// BCGEN-NEXT:[String 10]
// BCGEN-NEXT:[String 11]
// BCGEN-NEXT:[String 12]
// BCGEN-NEXT:[String 26]
// BCGEN-NEXT:[String 13]
// BCGEN-NEXT:[String 2]
// BCGEN-NEXT:[String 5]
// BCGEN-NEXT:[String 14]
// BCGEN-NEXT:[String 8]
// BCGEN-NEXT:[String 15]
// BCGEN-NEXT:[String 4]
// BCGEN-NEXT:[String 16]
// BCGEN-NEXT:[String 17]
// BCGEN-NEXT:[String 6]
// BCGEN-NEXT:[String 24]
// BCGEN-NEXT:[String 25]
// BCGEN-NEXT:[int 1]
// BCGEN-NEXT:[String 13]
// BCGEN-NEXT:[String 2]
// BCGEN-NEXT:[String 5]
// BCGEN-NEXT:[String 14]
// BCGEN-NEXT:[String 8]
// BCGEN-NEXT:[String 15]
// BCGEN-NEXT:[String 4]
// BCGEN-NEXT:[String 16]
// BCGEN-NEXT:[String 17]
// BCGEN-NEXT:[String 6]
// BCGEN-NEXT:[String 24]
// BCGEN-NEXT:[String 25]
// BCGEN-NEXT:[int 1]
// BCGEN-NEXT:[String 3]
// BCGEN-NEXT:[int 3]
// BCGEN-NEXT:[String 9]
// BCGEN-NEXT:[int 2]
// BCGEN-NEXT:[String 10]
// BCGEN-NEXT:Object Shape Table:
// BCGEN-NEXT:0[0, 7]
// BCGEN-NEXT:1[15, 18]
// BCGEN-NEXT:2[53, 13]
// BCGEN-NEXT:3[83, 6]
// BCGEN-NEXT:Function<global>(1 params, 3 registers, 0 numbers, 1 non-pointers):
// BCGEN-NEXT:Offset in debug table: source 0x0000, lexical 0x0000
// BCGEN-NEXT:    DeclareGlobalVar  "obj1"
// BCGEN-NEXT:    DeclareGlobalVar  "obj2"
// BCGEN-NEXT:    DeclareGlobalVar  "obj3"
// BCGEN-NEXT:    DeclareGlobalVar  "obj4"
// BCGEN-NEXT:    DeclareGlobalVar  "obj5"
// BCGEN-NEXT:    DeclareGlobalVar  "obj6"
// BCGEN-NEXT:    DeclareGlobalVar  "obj7"
// BCGEN-NEXT:    GetGlobalObject   r2
// BCGEN-NEXT:    LoadConstUndefined r0
// BCGEN-NEXT:    CreateClosure     r1, r0, Function<obj1>
// BCGEN-NEXT:    PutByIdLoose      r2, r1, 1, "obj1"
// BCGEN-NEXT:    CreateClosure     r1, r0, Function<obj2>
// BCGEN-NEXT:    PutByIdLoose      r2, r1, 2, "obj2"
// BCGEN-NEXT:    CreateClosure     r1, r0, Function<obj3>
// BCGEN-NEXT:    PutByIdLoose      r2, r1, 3, "obj3"
// BCGEN-NEXT:    CreateClosure     r1, r0, Function<obj4>
// BCGEN-NEXT:    PutByIdLoose      r2, r1, 4, "obj4"
// BCGEN-NEXT:    CreateClosure     r1, r0, Function<obj5>
// BCGEN-NEXT:    PutByIdLoose      r2, r1, 5, "obj5"
// BCGEN-NEXT:    CreateClosure     r1, r0, Function<obj6>
// BCGEN-NEXT:    PutByIdLoose      r2, r1, 6, "obj6"
// BCGEN-NEXT:    CreateClosure     r1, r0, Function<obj7>
// BCGEN-NEXT:    PutByIdLoose      r2, r1, 7, "obj7"
// BCGEN-NEXT:    Ret               r0

// BCGEN:Function<obj1>(1 params, 3 registers, 0 numbers, 1 non-pointers):
// BCGEN-NEXT:    NewObjectWithBuffer r1, 0, 0
// BCGEN-NEXT:    LoadConstUndefined r0
// BCGEN-NEXT:    CreateClosure     r2, r0, Function<f>
// BCGEN-NEXT:    PutOwnBySlotIdx   r1, r2, 5
// BCGEN-NEXT:    Ret               r1

// BCGEN:Function<obj2>(1 params, 1 registers, 0 numbers, 0 non-pointers):
// BCGEN-NEXT:    NewObjectWithBuffer r0, 1, 17
// BCGEN-NEXT:    Ret               r0

// BCGEN:Function<obj3>(1 params, 1 registers, 0 numbers, 0 non-pointers):
// BCGEN-NEXT:    NewObjectWithBuffer r0, 2, 85
// BCGEN-NEXT:    Ret               r0

// BCGEN:Function<obj4>(1 params, 1 registers, 0 numbers, 0 non-pointers):
// BCGEN-NEXT:    NewObjectWithBuffer r0, 2, 85
// BCGEN-NEXT:    Ret               r0

// BCGEN:Function<obj5>(1 params, 1 registers, 0 numbers, 0 non-pointers):
// BCGEN-NEXT:    NewObjectWithBuffer r0, 3, 135
// BCGEN-NEXT:    Ret               r0

// BCGEN:Function<obj6>(1 params, 1 registers, 0 numbers, 0 non-pointers):
// BCGEN-NEXT:    NewObjectWithBuffer r0, 3, 67
// BCGEN-NEXT:    Ret               r0

// BCGEN:Function<obj7>(1 params, 1 registers, 0 numbers, 0 non-pointers):
// BCGEN-NEXT:    NewObjectWithBuffer r0, 3, 68
// BCGEN-NEXT:    Ret               r0

// BCGEN:Function<f>(1 params, 1 registers, 0 numbers, 1 non-pointers):
// BCGEN-NEXT:    LoadConstUndefined r0
// BCGEN-NEXT:    Ret               r0

// BCGEN:Debug filename table:
// BCGEN-NEXT:  0: {{.*}}hbc_object_literals.js

// BCGEN:Debug file table:
// BCGEN-NEXT:  source table offset 0x0000: filename id 0

// BCGEN:Debug source table:
// BCGEN-NEXT:  0x0000  function idx 0, starts at line 11 col 1
// BCGEN-NEXT:    bc 0: line 11 col 1
// BCGEN-NEXT:    bc 5: line 11 col 1
// BCGEN-NEXT:    bc 10: line 11 col 1
// BCGEN-NEXT:    bc 15: line 11 col 1
// BCGEN-NEXT:    bc 20: line 11 col 1
// BCGEN-NEXT:    bc 25: line 11 col 1
// BCGEN-NEXT:    bc 30: line 11 col 1
// BCGEN-NEXT:    bc 44: line 11 col 1
// BCGEN-NEXT:    bc 55: line 11 col 1
// BCGEN-NEXT:    bc 66: line 11 col 1
// BCGEN-NEXT:    bc 77: line 11 col 1
// BCGEN-NEXT:    bc 88: line 11 col 1
// BCGEN-NEXT:    bc 99: line 11 col 1
// BCGEN-NEXT:    bc 110: line 11 col 1
// BCGEN-NEXT:  0x002e  end of debug source table
