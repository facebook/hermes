/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -dump-bytecode -pretty-disassemble=false -O %s | %FileCheck --match-full-lines %s

function fooNS() {
  return this;
}

//CHECK-LABEL: Function<fooNS>({{.*}}):
//CHECK-NEXT: [@ 0] LoadThisNS 0<Reg8>
//CHECK-NEXT: [@ 2] Ret 0<Reg8>

function foo() {
  "use strict";
  return this;
}

//CHECK-LABEL: Function<foo>({{.*}}):
//CHECK-NEXT: [@ 0] LoadParam 0<Reg8>, 0<UInt8>
//CHECK-NEXT: [@ 3] Ret 0<Reg8>
