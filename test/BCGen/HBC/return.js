/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -dump-bytecode -pretty-disassemble=false -O %s | %FileCheck --match-full-lines %s

//CHECK-LABEL:Function<foo>(1 params, 1 registers, 0 symbols):
//CHECK-NEXT:{{.*}}] LoadConstUInt8 0<Reg8>, 42<UInt8>
//CHECK-NEXT:{{.*}}] Ret 0<Reg8>

function foo() {
    return 42;
}
