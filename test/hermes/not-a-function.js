/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// Run tests ensuring the IR doesn't change based on the debug info settings.
// N.B.: only the first of these use %FileCheckOrRegen, but all tests use the
// same sets of expectations.
// RUN: %hermesc %s --dump-textified-callee --dump-ir     | %FileCheckOrRegen %s.ir.txt --check-prefix CHKIR
// RUN: %hermesc %s --dump-textified-callee --dump-ir -g  | %FileCheck %s.ir.txt --check-prefix CHKIR
// RUN: %hermesc %s --dump-textified-callee --dump-ir -g0 | %FileCheck %s.ir.txt --check-prefix CHKIR
// RUN: %hermesc %s --dump-textified-callee --dump-ir -g1 | %FileCheck %s.ir.txt --check-prefix CHKIR
// RUN: %hermesc %s --dump-textified-callee --dump-ir -g2 | %FileCheck %s.ir.txt --check-prefix CHKIR
// RUN: %hermesc %s --dump-textified-callee --dump-ir -g3 | %FileCheck %s.ir.txt --check-prefix CHKIR

// Run tests for ensuring that the VM operates properly both with and without the new info
// RUN: %hermes %s     | %FileCheck %s --match-full-lines --check-prefix CHK-DEFAULT
// RUN: %hermes %s -g  | %FileCheck %s --match-full-lines --check-prefix CHK-WITHNAME
// RUN: %hermes %s -g0 | %FileCheck %s --match-full-lines --check-prefix CHK-DEFAULT
// RUN: %hermes %s -g1 | %FileCheck %s --match-full-lines --check-prefix CHK-DEFAULT
// RUN: %hermes %s -g2 | %FileCheck %s --match-full-lines --check-prefix CHK-WITHNAME
// RUN: %hermes %s -g3 | %FileCheck %s --match-full-lines --check-prefix CHK-WITHNAME

// Run tests with pre-compiled bytecode to ensure the new info survives bytecode serialization.
// RUN: %hermesc %s      --emit-binary --out %t.hbc && %hermes %t.hbc | %FileCheck %s --check-prefix CHK-DEFAULT
// RUN: %hermesc %s -g   --emit-binary --out %t.hbc && %hermes %t.hbc | %FileCheck %s --check-prefix CHK-WITHNAME
// RUN: %hermesc %s -g0  --emit-binary --out %t.hbc && %hermes %t.hbc | %FileCheck %s --check-prefix CHK-DEFAULT
// RUN: %hermesc %s -g1  --emit-binary --out %t.hbc && %hermes %t.hbc | %FileCheck %s --check-prefix CHK-DEFAULT
// RUN: %hermesc %s -g2  --emit-binary --out %t.hbc && %hermes %t.hbc | %FileCheck %s --check-prefix CHK-WITHNAME
// RUN: %hermesc %s -g3  --emit-binary --out %t.hbc && %hermes %t.hbc | %FileCheck %s --check-prefix CHK-WITHNAME

// Run bytecode output verification tests.
// RUN: %hermesc %s --dump-bytecode     | %FileCheckOrRegen %s.bcdefault.txt --check-prefix CHK-BCDEFAULT
// RUN: %hermesc %s --dump-bytecode -g  | %FileCheckOrRegen %s.bcg.txt --check-prefix CHK-BCG
// RUN: %hermesc %s --dump-bytecode -g0 | %FileCheckOrRegen %s.bcg0.txt --check-prefix CHK-BCG0
// RUN: %hermesc %s --dump-bytecode -g1 | %FileCheckOrRegen %s.bcg1.txt --check-prefix CHK-BCG1
// RUN: %hermesc %s --dump-bytecode -g2 | %FileCheckOrRegen %s.bcg2.txt --check-prefix CHK-BCG2
// RUN: %hermesc %s --dump-bytecode -g3 | %FileCheckOrRegen %s.bcg3.txt --check-prefix CHK-BCG3

"use strict";

print('errors');
// CHECK-LABEL: errors

var a = {
    b: (() => [() => false])
};

function test(f) {
    try
    {
        f();
    } catch(e) {
        print(`${e.name}:`, e.message);
    }
}

test(() => a());
// CHK-DEFAULT: TypeError: Object is not a function
// CHK-WITHNAME: TypeError: a is not a function (it is Object)

test(() => a.z());
// CHK-DEFAULT: TypeError: undefined is not a function
// CHK-WITHNAME: TypeError: a.z is not a function (it is undefined)

test(() => null());
// CHK-DEFAULT: TypeError: null is not a function
// CHK-WITHNAME: TypeError: null is not a function (it is null)

test(() => true());
// CHK-DEFAULT: TypeError: true is not a function
// CHK-WITHNAME: TypeError: true is not a function (it is true)

test(() => false());
// CHK-DEFAULT: TypeError: false is not a function
// CHK-WITHNAME: TypeError: false is not a function (it is false)

test(() => "str"());
// CHK-DEFAULT: TypeError: 'str' is not a function
// CHK-WITHNAME: TypeError: "str" is not a function (it is 'str')

test(() => 1());
// CHK-DEFAULT: TypeError: 1 is not a function
// CHK-WITHNAME: TypeError: 1 is not a function (it is 1)

test(() => (3.14)());
// CHK-DEFAULT: TypeError: 3.14 is not a function
// CHK-WITHNAME: TypeError: 3.14 is not a function (it is 3.14)

test(() => a.b()[0]()());
// CHK-DEFAULT: TypeError: false is not a function
// CHK-WITHNAME: TypeError: a.b()[0]() is not a function (it is false)

test(() => a.a0000000111111111122222222223333333333444444444455555555556666());
// CHK-DEFAULT: TypeError: undefined is not a function
// CHK-WITHNAME: TypeError: a.a0000000111111111122222222223333333333444444444455555555556666 is not a function (it is undefined)

test(() => a.a00000001111111111222222222233ThisShouldNotShowUpAtInTheTextifiedCallee33333333444444444455555555556666());
// CHK-DEFAULT: TypeError: undefined is not a function
// CHK-WITHNAME: TypeError: a.a00000001111111111222222222233(...)33333333444444444455555555556666 is not a function (it is undefined)

test(() => `

càll
  T     ô     Ü

 n d e f i n è d

`())
// CHK-DEFAULT: TypeError: '

// CHK-DEFAULT: càll
// CHK-DEFAULT:   T     ô     Ü

// CHK-DEFAULT:  n d e f i n è d

// CHK-DEFAULT: ' is not a function
// CHK-WITHNAME: TypeError: `càllT     ô     Ün d e f i n è d` is not a function (it is '

// CHK-WITHNAME: càll
// CHK-WITHNAME:   T     ô     Ü

// CHK-WITHNAME:  n d e f i n è d

// CHK-WITHNAME: ')

test(() => a.ààààààààèèèèèèèèèèììììììììììòòòòòòòòòòùùùùùùùùùùââââââââââêêêê());
// CHK-DEFAULT: TypeError: undefined is not a function
// CHK-WITHNAME: TypeError: a.ààààààààèèèèèèèèèèììììììììììòòòòòòòòòòùùùùùùùùùùââââââââââêêêê is not a function (it is undefined)

test(() => a.ààààààààèèèèèèèèèèììììììììììòòüThisShouldNotShowUpAtInThèTextifiedCalleeüòòòòòòòòùùùùùùùùùùââââââââââêêêê());
// CHK-DEFAULT: TypeError: undefined is not a function
// CHK-WITHNAME: TypeError: a.ààààààààèèèèèèèèèèììììììììììòò(...)òòòòòòòòùùùùùùùùùùââââââââââêêêê is not a function (it is undefined)
