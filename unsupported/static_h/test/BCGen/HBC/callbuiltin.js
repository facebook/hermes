/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O -fstatic-builtins -target=HBC -dump-postra %s | %FileCheck --match-full-lines --check-prefix=CHKRA %s
// RUN: %hermesc -O -fstatic-builtins -target=HBC -dump-bytecode %s | %FileCheck --match-full-lines --check-prefix=CHKBC %s
// RUN: %hermes -O -fstatic-builtins -target=HBC %s | %FileCheck --match-full-lines %s

function foo(x) {
    return Object.keys(x)
}

//CHKRA-LABEL:function foo(x)
//CHKRA-NEXT:frame = []
//CHKRA-NEXT:%BB0:
//CHKRA-NEXT:  %0 = HBCLoadParamInst 1 : number
//CHKRA-NEXT:  %1 = ImplicitMovInst undefined : undefined
//CHKRA-NEXT:  %2 = CallBuiltinInst [Object.keys] : number, undefined : undefined, %0
//CHKRA-NEXT:  %3 = ReturnInst %2
//CHKRA-NEXT:function_end

//CHKBC-LABEL:Function<foo>(2 params, 9 registers, 0 symbols):
//CHKBC-NEXT: Offset{{.*}}
//CHKBC-NEXT:    LoadParam         r1, 1
//CHKBC-NEXT:    CallBuiltin       r0, "Object.keys", 2
//CHKBC-NEXT:    Ret               r0

// Make sure that this isn't incorrectly recognized as a builtin.
function shadows() {
    var Object = {keys: print};
    Object.keys("evil");
}

//CHKRA-LABEL:function shadows() : undefined
//CHKRA-NEXT:frame = []
//CHKRA-NEXT:%BB0:
//CHKRA-NEXT:  %0 = HBCGetGlobalObjectInst
//CHKRA-NEXT:  %1 = TryLoadGlobalPropertyInst %0 : object, "print" : string
//CHKRA-NEXT:  %2 = AllocObjectInst 1 : number, empty
//CHKRA-NEXT:  %3 = StoreNewOwnPropertyInst %1, %2 : object, "keys" : string, true : boolean
//CHKRA-NEXT:  %4 = LoadPropertyInst %2 : object, "keys" : string
//CHKRA-NEXT:  %5 = HBCLoadConstInst "evil" : string
//CHKRA-NEXT:  %6 = ImplicitMovInst %2 : object
//CHKRA-NEXT:  %7 = ImplicitMovInst %5 : string
//CHKRA-NEXT:  %8 = HBCCallNInst %4, %2 : object, %5 : string
//CHKRA-NEXT:  %9 = HBCLoadConstInst undefined : undefined
//CHKRA-NEXT:  %10 = ReturnInst %9 : undefined
//CHKRA-NEXT:function_end

function checkNonStaticBuiltin() {
  HermesInternal.concat('hello');
}

//CHKRA-LABEL:function checkNonStaticBuiltin() : undefined
//CHKRA-NEXT:frame = []
//CHKRA-NEXT:%BB0:
//CHKRA-NEXT:  %0 = HBCGetGlobalObjectInst
//CHKRA-NEXT:  %1 = TryLoadGlobalPropertyInst %0 : object, "HermesInternal" : string
//CHKRA-NEXT:  %2 = LoadPropertyInst %1, "concat" : string
//CHKRA-NEXT:  %3 = HBCLoadConstInst "hello" : string
//CHKRA-NEXT:  %4 = ImplicitMovInst %1
//CHKRA-NEXT:  %5 = ImplicitMovInst %3 : string
//CHKRA-NEXT:  %6 = HBCCallNInst %2, %1, %3 : string
//CHKRA-NEXT:  %7 = HBCLoadConstInst undefined : undefined
//CHKRA-NEXT:  %8 = ReturnInst %7 : undefined
//CHKRA-NEXT:function_end

print(foo({a: 10, b: 20, lastKey:30, 5:6}))
//CHECK: 5,a,b,lastKey
