/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -emit-binary -target=HBC -out=%t %s && %hbcdump %t -c "disassemble;quit" -objdump-disassemble | %FileCheck --match-full-lines %s

print("hello");

//CHECK-LABEL:{{.*}}: file format HBC-{{.*}}
//CHECK-LABEL:Disassembly of section .text:
//CHECK-LABEL:00000000000000b8 <_0>:
//CHECK-NEXT:000000b8:{{.*}}GetGlobalObject {{.*}}%r0
//CHECK-NEXT:000000ba:{{.*}}TryGetById {{.*}}%r2, %r0, $0x1, $0x02
//CHECK-NEXT:000000c0:{{.*}}LoadConstUndefined {{.*}}%r1
//CHECK-NEXT:000000c2:{{.*}}LoadConstString {{.*}}%r0, $0x01
//CHECK-NEXT:000000c6:{{.*}}Call2 {{.*}}%r0, %r2, %r1, %r0
//CHECK-NEXT:000000cb:{{.*}}Ret {{.*}}%r0
