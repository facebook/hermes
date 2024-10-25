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
//CHECK-LABEL:00000000000000b4 <_0>:
//CHECK-NEXT:000000b4:{{.*}}GetGlobalObject {{.*}}%r1
//CHECK-NEXT:000000b6:{{.*}}TryGetById {{.*}}%r2, %r1, $0x1, $0x02
//CHECK-NEXT:000000bc:{{.*}}LoadConstUndefined {{.*}}%r0
//CHECK-NEXT:000000be:{{.*}}LoadConstString {{.*}}%r1, $0x01
//CHECK-NEXT:000000c2:{{.*}}Call2 {{.*}}%r1, %r2, %r0, %r1
//CHECK-NEXT:000000c7:{{.*}}Ret {{.*}}%r1
