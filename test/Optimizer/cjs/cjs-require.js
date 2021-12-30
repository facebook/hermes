/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -commonjs -fstatic-require -fstatic-builtins -dump-bytecode %s %S/m1.js %S/m2.js -O  | %FileCheck --match-full-lines --check-prefix=CHKOPT %s
// RUN: %hermesc -commonjs -fstatic-require -fstatic-builtins -dump-bytecode %s %S/m1.js %S/m2.js -Og | %FileCheck --match-full-lines --check-prefix=CHKDBG %s

var m1 = require("./m" + "1.js");
m1.foo();

function bar() {
    var m2 = require("./m" + "2.js");
    m2.baz();
}

exports.bar = bar;

//CHKOPT-LABEL:Function<cjs_module>(4 params, 10 registers, 0 symbols):
//CHKOPT-NEXT:Offset in debug table: {{.*}}
//CHKOPT-NEXT:    LoadConstUInt8    r2, 1
//CHKOPT-NEXT:    CallBuiltin       r1, "HermesBuiltin.requireFast", 2
//CHKOPT-NEXT:    GetByIdShort      r0, r1, 1, "foo"
//CHKOPT-NEXT:    Call1             r0, r0, r1
//CHKOPT-NEXT:    CreateEnvironment r0
//CHKOPT-NEXT:    CreateClosure     r1, r0, 2
//CHKOPT-NEXT:    LoadParam         r0, 1
//CHKOPT-NEXT:    PutById           r0, r1, 1, "bar"
//CHKOPT-NEXT:    LoadConstUndefined r0
//CHKOPT-NEXT:    Ret               r0

//CHKOPT-LABEL:Function<bar>(1 params, 10 registers, 0 symbols):
//CHKOPT-NEXT:Offset in debug table: {{.*}}
//CHKOPT-NEXT:    LoadConstUInt8    r2, 2
//CHKOPT-NEXT:    CallBuiltin       r1, "HermesBuiltin.requireFast", 2
//CHKOPT-NEXT:    GetByIdShort      r0, r1, 1, "baz"
//CHKOPT-NEXT:    Call1             r0, r0, r1
//CHKOPT-NEXT:    LoadConstUndefined r0
//CHKOPT-NEXT:    Ret               r0

//CHKDBG-LABEL: Function<cjs_module>(4 params, 21 registers, 5 symbols):
//CHKDBG-NEXT: Offset in debug table: {{.*}}
//CHKDBG-NEXT:     CreateEnvironment r0
//CHKDBG-NEXT:     LoadParam         r1, 1
//CHKDBG-NEXT:     LoadParam         r2, 2
//CHKDBG-NEXT:     LoadParam         r3, 3
//CHKDBG-NEXT:     LoadConstUndefined r4
//CHKDBG-NEXT:     LoadConstUInt8    r5, 1
//CHKDBG-NEXT:     StoreNPToEnvironment r0, 0, r4
//CHKDBG-NEXT:     StoreToEnvironment r0, 2, r1
//CHKDBG-NEXT:     StoreToEnvironment r0, 3, r2
//CHKDBG-NEXT:     StoreToEnvironment r0, 4, r3
//CHKDBG-NEXT:     CreateClosure     r6, r0, 2
//CHKDBG-NEXT:     StoreToEnvironment r0, 1, r6
//CHKDBG-NEXT:     LoadFromEnvironment r7, r0, 3
//CHKDBG-NEXT:     Mov               r13, r5
//CHKDBG-NEXT:     CallBuiltin       r7, "HermesBuiltin.requireFast", 2
//CHKDBG-NEXT:     StoreToEnvironment r0, 0, r7
//CHKDBG-NEXT:     LoadFromEnvironment r8, r0, 0
//CHKDBG-NEXT:     GetByIdShort      r9, r8, 1, "foo"
//CHKDBG-NEXT:     Mov               r14, r8
//CHKDBG-NEXT:     Call              r10, r9, 1
//CHKDBG-NEXT:     LoadFromEnvironment r10, r0, 2
//CHKDBG-NEXT:     LoadFromEnvironment r11, r0, 1
//CHKDBG-NEXT:     PutById           r10, r11, 1, "bar"
//CHKDBG-NEXT:     Ret               r4

//CHKDBG-LABEL: Function<bar>(1 params, 16 registers, 1 symbols):
//CHKDBG-NEXT: Offset in debug table: {{.*}}
//CHKDBG-NEXT:     CreateEnvironment r0
//CHKDBG-NEXT:     LoadConstUndefined r1
//CHKDBG-NEXT:     LoadConstUInt8    r2, 2
//CHKDBG-NEXT:     StoreNPToEnvironment r0, 0, r1
//CHKDBG-NEXT:     GetEnvironment    r3, 0
//CHKDBG-NEXT:     LoadFromEnvironment r4, r3, 3
//CHKDBG-NEXT:     Mov               r8, r2
//CHKDBG-NEXT:     CallBuiltin       r4, "HermesBuiltin.requireFast", 2
//CHKDBG-NEXT:     StoreToEnvironment r0, 0, r4
//CHKDBG-NEXT:     LoadFromEnvironment r5, r0, 0
//CHKDBG-NEXT:     GetByIdShort      r6, r5, 1, "baz"
//CHKDBG-NEXT:     Mov               r9, r5
//CHKDBG-NEXT:     Call              r7, r6, 1
//CHKDBG-NEXT:     Ret               r1
