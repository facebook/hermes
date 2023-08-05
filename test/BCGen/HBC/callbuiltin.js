/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O -fstatic-builtins -target=HBC -dump-postra %s | %FileCheckOrRegen --match-full-lines --check-prefix=CHKRA %s
// RUN: %hermesc -O -fstatic-builtins -target=HBC -dump-bytecode %s | %FileCheckOrRegen --match-full-lines --check-prefix=CHKBC %s
// RUN: %hermes -O -fstatic-builtins -target=HBC %s | %FileCheck --match-full-lines %s

function foo(x) {
    return Object.keys(x)
}

function foo2(x) {
    return typeof Object.keys === 'function' ? Object.keys(x) : null;
}

function foo3(x) {
    return Object.keys ? Object.keys(x) : null;
}

// Make sure that this isn't incorrectly recognized as a builtin.
function shadows() {
    var Object = {keys: print};
    Object.keys("evil");
}

function checkNonStaticBuiltin() {
  HermesInternal.concat('hello');
}

print(foo({a: 10, b: 20, lastKey:30, 5:6}))
//CHECK: 5,a,b,lastKey

// Auto-generated content below. Please do not modify manually.

// CHKRA:function global#0()#1
// CHKRA-NEXT:globals = [foo, foo2, foo3, shadows, checkNonStaticBuiltin]
// CHKRA-NEXT:S{global#0()#1} = []
// CHKRA-NEXT:%BB0:
// CHKRA-NEXT:  %0 = HBCCreateEnvironmentInst %S{global#0()#1}
// CHKRA-NEXT:  %1 = HBCCreateFunctionInst %foo#0#1()#2, %0
// CHKRA-NEXT:  %2 = HBCGetGlobalObjectInst
// CHKRA-NEXT:  %3 = StorePropertyInst %1 : closure, %2 : object, "foo" : string
// CHKRA-NEXT:  %4 = HBCCreateFunctionInst %foo2#0#1()#3, %0
// CHKRA-NEXT:  %5 = StorePropertyInst %4 : closure, %2 : object, "foo2" : string
// CHKRA-NEXT:  %6 = HBCCreateFunctionInst %foo3#0#1()#4, %0
// CHKRA-NEXT:  %7 = StorePropertyInst %6 : closure, %2 : object, "foo3" : string
// CHKRA-NEXT:  %8 = HBCCreateFunctionInst %shadows#0#1()#5 : undefined, %0
// CHKRA-NEXT:  %9 = StorePropertyInst %8 : closure, %2 : object, "shadows" : string
// CHKRA-NEXT:  %10 = HBCCreateFunctionInst %checkNonStaticBuiltin#0#1()#6 : undefined, %0
// CHKRA-NEXT:  %11 = StorePropertyInst %10 : closure, %2 : object, "checkNonStaticBuiltin" : string
// CHKRA-NEXT:  %12 = TryLoadGlobalPropertyInst %2 : object, "print" : string
// CHKRA-NEXT:  %13 = LoadPropertyInst %2 : object, "foo" : string
// CHKRA-NEXT:  %14 = HBCLoadConstInst undefined : undefined
// CHKRA-NEXT:  %15 = HBCAllocObjectFromBufferInst 4 : number, "a" : string, 10 : number, "b" : string, 20 : number, "lastKey" : string, 30 : number, 5 : number, 6 : number
// CHKRA-NEXT:  %16 = ImplicitMovInst %14 : undefined
// CHKRA-NEXT:  %17 = ImplicitMovInst %15 : object
// CHKRA-NEXT:  %18 = HBCCallNInst %13, undefined : undefined, %14 : undefined, %15 : object
// CHKRA-NEXT:  %19 = ImplicitMovInst %14 : undefined
// CHKRA-NEXT:  %20 = ImplicitMovInst %18
// CHKRA-NEXT:  %21 = HBCCallNInst %12, undefined : undefined, %14 : undefined, %18
// CHKRA-NEXT:  %22 = ReturnInst %21
// CHKRA-NEXT:function_end

// CHKRA:function foo#0#1(x)#2
// CHKRA-NEXT:S{foo#0#1()#2} = []
// CHKRA-NEXT:%BB0:
// CHKRA-NEXT:  %0 = HBCLoadParamInst 1 : number
// CHKRA-NEXT:  %1 = ImplicitMovInst undefined : undefined
// CHKRA-NEXT:  %2 = CallBuiltinInst [Object.keys] : number, undefined : undefined, undefined : undefined, %0
// CHKRA-NEXT:  %3 = ReturnInst %2
// CHKRA-NEXT:function_end

// CHKRA:function foo2#0#1(x)#2
// CHKRA-NEXT:S{foo2#0#1()#2} = []
// CHKRA-NEXT:%BB0:
// CHKRA-NEXT:  %0 = HBCLoadParamInst 1 : number
// CHKRA-NEXT:  %1 = ImplicitMovInst undefined : undefined
// CHKRA-NEXT:  %2 = CallBuiltinInst [Object.keys] : number, undefined : undefined, undefined : undefined, %0
// CHKRA-NEXT:  %3 = MovInst %2
// CHKRA-NEXT:  %4 = BranchInst %BB1
// CHKRA-NEXT:%BB1:
// CHKRA-NEXT:  %5 = PhiInst %3, %BB0
// CHKRA-NEXT:  %6 = MovInst %5
// CHKRA-NEXT:  %7 = ReturnInst %6
// CHKRA-NEXT:function_end

// CHKRA:function foo3#0#1(x)#2
// CHKRA-NEXT:S{foo3#0#1()#2} = []
// CHKRA-NEXT:%BB0:
// CHKRA-NEXT:  %0 = HBCLoadParamInst 1 : number
// CHKRA-NEXT:  %1 = ImplicitMovInst undefined : undefined
// CHKRA-NEXT:  %2 = CallBuiltinInst [Object.keys] : number, undefined : undefined, undefined : undefined, %0
// CHKRA-NEXT:  %3 = MovInst %2
// CHKRA-NEXT:  %4 = BranchInst %BB1
// CHKRA-NEXT:%BB1:
// CHKRA-NEXT:  %5 = PhiInst %3, %BB0
// CHKRA-NEXT:  %6 = MovInst %5
// CHKRA-NEXT:  %7 = ReturnInst %6
// CHKRA-NEXT:function_end

// CHKRA:function shadows#0#1()#2 : undefined
// CHKRA-NEXT:S{shadows#0#1()#2} = []
// CHKRA-NEXT:%BB0:
// CHKRA-NEXT:  %0 = HBCGetGlobalObjectInst
// CHKRA-NEXT:  %1 = TryLoadGlobalPropertyInst %0 : object, "print" : string
// CHKRA-NEXT:  %2 = AllocObjectInst 1 : number, empty
// CHKRA-NEXT:  %3 = StoreNewOwnPropertyInst %1, %2 : object, "keys" : string, true : boolean
// CHKRA-NEXT:  %4 = LoadPropertyInst %2 : object, "keys" : string
// CHKRA-NEXT:  %5 = HBCLoadConstInst "evil" : string
// CHKRA-NEXT:  %6 = ImplicitMovInst %2 : object
// CHKRA-NEXT:  %7 = ImplicitMovInst %5 : string
// CHKRA-NEXT:  %8 = HBCCallNInst %4, undefined : undefined, %2 : object, %5 : string
// CHKRA-NEXT:  %9 = HBCLoadConstInst undefined : undefined
// CHKRA-NEXT:  %10 = ReturnInst %9 : undefined
// CHKRA-NEXT:function_end

// CHKRA:function checkNonStaticBuiltin#0#1()#2 : undefined
// CHKRA-NEXT:S{checkNonStaticBuiltin#0#1()#2} = []
// CHKRA-NEXT:%BB0:
// CHKRA-NEXT:  %0 = HBCGetGlobalObjectInst
// CHKRA-NEXT:  %1 = TryLoadGlobalPropertyInst %0 : object, "HermesInternal" : string
// CHKRA-NEXT:  %2 = LoadPropertyInst %1, "concat" : string
// CHKRA-NEXT:  %3 = HBCLoadConstInst "hello" : string
// CHKRA-NEXT:  %4 = ImplicitMovInst %1
// CHKRA-NEXT:  %5 = ImplicitMovInst %3 : string
// CHKRA-NEXT:  %6 = HBCCallNInst %2, undefined : undefined, %1, %3 : string
// CHKRA-NEXT:  %7 = HBCLoadConstInst undefined : undefined
// CHKRA-NEXT:  %8 = ReturnInst %7 : undefined
// CHKRA-NEXT:function_end

// CHKBC:Bytecode File Information:
// CHKBC-NEXT:  Bytecode version number: {{.*}}
// CHKBC-NEXT:  Source hash: {{.*}}
// CHKBC-NEXT:  Function count: 6
// CHKBC-NEXT:  String count: 15
// CHKBC-NEXT:  BigInt count: 0
// CHKBC-NEXT:  String Kind Entry count: 2
// CHKBC-NEXT:  RegExp count: 0
// CHKBC-NEXT:  Segment ID: 0
// CHKBC-NEXT:  CommonJS module count: 0
// CHKBC-NEXT:  CommonJS module count (static): 0
// CHKBC-NEXT:  Function source count: 0
// CHKBC-NEXT:  Bytecode options:
// CHKBC-NEXT:    staticBuiltins: 1
// CHKBC-NEXT:    cjsModulesStaticallyResolved: 0

// CHKBC:Global String Table:
// CHKBC-NEXT:s0[ASCII, 0..3]: evil
// CHKBC-NEXT:s1[ASCII, 10..15]: global
// CHKBC-NEXT:s2[ASCII, 16..20]: hello
// CHKBC-NEXT:i3[ASCII, 3..9] #C95C4184: lastKey
// CHKBC-NEXT:i4[ASCII, 14..14] #00018270: a
// CHKBC-NEXT:i5[ASCII, 21..34] #85BBF6F9: HermesInternal
// CHKBC-NEXT:i6[ASCII, 35..35] #00018E43: b
// CHKBC-NEXT:i7[ASCII, 36..56] #B182F5CE: checkNonStaticBuiltin
// CHKBC-NEXT:i8[ASCII, 57..62] #CB8DFA65: concat
// CHKBC-NEXT:i9[ASCII, 63..65] #9290584E: foo
// CHKBC-NEXT:i10[ASCII, 63..66] #D0BD91E2: foo2
// CHKBC-NEXT:i11[ASCII, 67..70] #D0BD95F3: foo3
// CHKBC-NEXT:i12[ASCII, 71..74] #973712F5: keys
// CHKBC-NEXT:i13[ASCII, 74..80] #65FB9C2E: shadows
// CHKBC-NEXT:i14[ASCII, 81..85] #A689F65B: print

// CHKBC:Object Key Buffer:
// CHKBC-NEXT:[String 4]
// CHKBC-NEXT:[String 6]
// CHKBC-NEXT:[String 3]
// CHKBC-NEXT:[int 5]

// CHKBC:Object Value Buffer:
// CHKBC-NEXT:[int 10]
// CHKBC-NEXT:[int 20]
// CHKBC-NEXT:[int 30]
// CHKBC-NEXT:[int 6]

// CHKBC:Function<global>(1 params, 12 registers, 0 symbols):
// CHKBC-NEXT:Offset in debug table: source 0x0000, scope 0x0000, textified callees 0x0000
// CHKBC-NEXT:    DeclareGlobalVar  "foo"
// CHKBC-NEXT:    DeclareGlobalVar  "foo2"
// CHKBC-NEXT:    DeclareGlobalVar  "foo3"
// CHKBC-NEXT:    DeclareGlobalVar  "shadows"
// CHKBC-NEXT:    DeclareGlobalVar  "checkNonStaticBui"...
// CHKBC-NEXT:    CreateEnvironment r1
// CHKBC-NEXT:    CreateClosure     r2, r1, Function<foo>
// CHKBC-NEXT:    GetGlobalObject   r0
// CHKBC-NEXT:    PutById           r0, r2, 1, "foo"
// CHKBC-NEXT:    CreateClosure     r2, r1, Function<foo2>
// CHKBC-NEXT:    PutById           r0, r2, 2, "foo2"
// CHKBC-NEXT:    CreateClosure     r2, r1, Function<foo3>
// CHKBC-NEXT:    PutById           r0, r2, 3, "foo3"
// CHKBC-NEXT:    CreateClosure     r2, r1, Function<shadows>
// CHKBC-NEXT:    PutById           r0, r2, 4, "shadows"
// CHKBC-NEXT:    CreateClosure     r1, r1, Function<checkNonStaticBuiltin>
// CHKBC-NEXT:    PutById           r0, r1, 5, "checkNonStaticBui"...
// CHKBC-NEXT:    TryGetById        r2, r0, 1, "print"
// CHKBC-NEXT:    GetByIdShort      r3, r0, 2, "foo"
// CHKBC-NEXT:    LoadConstUndefined r1
// CHKBC-NEXT:    NewObjectWithBuffer r0, 4, 4, 0, 0
// CHKBC-NEXT:    Call2             r0, r3, r1, r0
// CHKBC-NEXT:    Call2             r0, r2, r1, r0
// CHKBC-NEXT:    Ret               r0

// CHKBC:Function<foo>(2 params, 9 registers, 0 symbols):
// CHKBC-NEXT:Offset in debug table: source 0x0031, scope 0x0000, textified callees 0x0000
// CHKBC-NEXT:    LoadParam         r1, 1
// CHKBC-NEXT:    CallBuiltin       r0, "Object.keys", 2
// CHKBC-NEXT:    Ret               r0

// CHKBC:Function<foo2>(2 params, 10 registers, 0 symbols):
// CHKBC-NEXT:Offset in debug table: source 0x003e, scope 0x0000, textified callees 0x0000
// CHKBC-NEXT:    LoadParam         r2, 1
// CHKBC-NEXT:    CallBuiltin       r0, "Object.keys", 2
// CHKBC-NEXT:    Ret               r0

// CHKBC:Function<foo3>(2 params, 10 registers, 0 symbols):
// CHKBC-NEXT:Offset in debug table: source 0x004b, scope 0x0000, textified callees 0x0000
// CHKBC-NEXT:    LoadParam         r2, 1
// CHKBC-NEXT:    CallBuiltin       r0, "Object.keys", 2
// CHKBC-NEXT:    Ret               r0

// CHKBC:Function<shadows>(1 params, 11 registers, 0 symbols):
// CHKBC-NEXT:Offset in debug table: source 0x0058, scope 0x0000, textified callees 0x0000
// CHKBC-NEXT:    GetGlobalObject   r0
// CHKBC-NEXT:    TryGetById        r0, r0, 1, "print"
// CHKBC-NEXT:    NewObject         r2
// CHKBC-NEXT:    PutNewOwnByIdShort r2, r0, "keys"
// CHKBC-NEXT:    GetByIdShort      r1, r2, 2, "keys"
// CHKBC-NEXT:    LoadConstString   r0, "evil"
// CHKBC-NEXT:    Call2             r0, r1, r2, r0
// CHKBC-NEXT:    LoadConstUndefined r0
// CHKBC-NEXT:    Ret               r0

// CHKBC:Function<checkNonStaticBuiltin>(1 params, 11 registers, 0 symbols):
// CHKBC-NEXT:Offset in debug table: source 0x0080, scope 0x0000, textified callees 0x0000
// CHKBC-NEXT:    GetGlobalObject   r0
// CHKBC-NEXT:    TryGetById        r2, r0, 1, "HermesInternal"
// CHKBC-NEXT:    GetByIdShort      r1, r2, 2, "concat"
// CHKBC-NEXT:    LoadConstString   r0, "hello"
// CHKBC-NEXT:    Call2             r0, r1, r2, r0
// CHKBC-NEXT:    LoadConstUndefined r0
// CHKBC-NEXT:    Ret               r0

// CHKBC:Debug filename table:
// CHKBC-NEXT:  0: {{.*}}callbuiltin.js

// CHKBC:Debug file table:
// CHKBC-NEXT:  source table offset 0x0000: filename id 0

// CHKBC:Debug source table:
// CHKBC-NEXT:  0x0000  function idx 0, starts at line 12 col 1
// CHKBC-NEXT:    bc 34: line 12 col 1 scope offset 0x0000 env r2
// CHKBC-NEXT:    bc 45: line 12 col 1 scope offset 0x0000 env r2
// CHKBC-NEXT:    bc 56: line 12 col 1 scope offset 0x0000 env r2
// CHKBC-NEXT:    bc 67: line 12 col 1 scope offset 0x0000 env r2
// CHKBC-NEXT:    bc 78: line 12 col 1 scope offset 0x0000 env r1
// CHKBC-NEXT:    bc 84: line 34 col 1 scope offset 0x0000 env r2
// CHKBC-NEXT:    bc 90: line 34 col 7 scope offset 0x0000 env r3
// CHKBC-NEXT:    bc 107: line 34 col 10 scope offset 0x0000 env r0
// CHKBC-NEXT:    bc 112: line 34 col 6 scope offset 0x0000 env r0
// CHKBC-NEXT:  0x0031  function idx 1, starts at line 12 col 1
// CHKBC-NEXT:    bc 3: line 13 col 23 scope offset 0x0000 env none
// CHKBC-NEXT:  0x003e  function idx 2, starts at line 16 col 1
// CHKBC-NEXT:    bc 3: line 17 col 59 scope offset 0x0000 env none
// CHKBC-NEXT:  0x004b  function idx 3, starts at line 20 col 1
// CHKBC-NEXT:    bc 3: line 21 col 37 scope offset 0x0000 env none
// CHKBC-NEXT:  0x0058  function idx 4, starts at line 25 col 1
// CHKBC-NEXT:    bc 2: line 26 col 25 scope offset 0x0000 env none
// CHKBC-NEXT:    bc 10: line 26 col 18 scope offset 0x0000 env none
// CHKBC-NEXT:    bc 14: line 27 col 16 scope offset 0x0000 env none
// CHKBC-NEXT:    bc 23: line 27 col 16 scope offset 0x0000 env none
// CHKBC-NEXT:  0x0080  function idx 5, starts at line 30 col 1
// CHKBC-NEXT:    bc 2: line 31 col 3 scope offset 0x0000 env none
// CHKBC-NEXT:    bc 8: line 31 col 24 scope offset 0x0000 env none
// CHKBC-NEXT:    bc 17: line 31 col 24 scope offset 0x0000 env none
// CHKBC-NEXT:  0x009f  end of debug source table

// CHKBC:Debug scope descriptor table:
// CHKBC-NEXT:  0x0000  lexical parent:   none, flags:    , variable count: 0
// CHKBC-NEXT:  0x0003  end of debug scope descriptor table

// CHKBC:Textified callees table:
// CHKBC-NEXT:  0x0000  entries: 0
// CHKBC-NEXT:  0x0001  end of textified callees table

// CHKBC:Debug string table:
// CHKBC-NEXT:  0x0000  end of debug string table
