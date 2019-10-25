/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

/*
RUN: %hermes -O -dump-bytecode %s \
RUN:     | %FileCheck --match-full-lines -check-prefix HBC %s
RUN: %hermes -O -dump-jitcode %s \
RUN:     | %FileCheck --match-full-lines -check-prefix JIT %s
REQUIRES: jit, jit_dis
*/

"use strict";

function add(x, y) {
    return x + y;
}

print(this['add'](1, 1));

print(this['add'](1.24, 7.11));

print(this['add']('1.24', 7.11));

print(this['add']('abc', 1));

print(this['add'](13, 'abc'));

print(this['add']('abc', 'xyz'));

print(this['add'](undefined, 1));

print(this['add'](undefined, 'xyz'));


//HBC-LABEL: Function<add>(3 params, 2 registers, 0 symbols):
//HBC-NEXT: Offset in debug table: {{.*}}
//HBC-NEXT: LoadParam         r1, 1
//HBC-NEXT: LoadParam         r0, 2
//HBC-NEXT: Add               r0, r1, r0
//HBC-NEXT: Ret               r0


//JIT-LABEL:Compiled Code of FunctionID: 1
//JIT-NEXT: pushq	%rbp
//JIT-NEXT: movq	%rsp, %rbp
//JIT-NEXT: pushq	%r15
//JIT-NEXT: pushq	%rbx
//JIT-NEXT: movq	%rdi, %rbx
//JIT-NEXT: pushq	3968(%rbx)
//JIT-NEXT: pushq	%rcx
//JIT-NEXT: movq	3952(%rbx), %r15
//JIT-NEXT: movq	%r15, 3968(%rbx)
//JIT-NEXT: leaq	{{.*}}, %rax
//JIT-NEXT: movq	%rax, 3952(%rbx)
//JIT-NEXT:  BB0:
//JIT-NEXT:  movq {{.*}}
//JIT-NEXT:  cmpl {{.*}}
//JIT-NEXT:  jb {{.*}}
//JIT-NEXT:  movq {{.*}}
//JIT-NEXT:  movq {{.*}}
//JIT-NEXT:  movq {{.*}}
//JIT-NEXT:  cmpl {{.*}}
//JIT-NEXT:  jb {{.*}}
//JIT-NEXT:  movq {{.*}}
//JIT-NEXT:  movq {{.*}}
//JIT-NEXT:  cmpl {{.*}}
//JIT-NEXT:  jae {{.*}}
//JIT-NEXT:  cmpl {{.*}}
//JIT-NEXT:  jae {{.*}}
//JIT-NEXT:  movsd {{.*}}
//JIT-NEXT:  addsd {{.*}}
//JIT-NEXT:  movsd {{.*}}
//JIT-NEXT:  movq {{.*}}
//JIT-NEXT:  movl {{.*}}
//JIT-NEXT:  BB1:
//JIT-NEXT: movq	%r15, 3952(%rbx)
//JIT-NEXT: popq	%rcx
//JIT-NEXT: popq	3968(%rbx)
//JIT-NEXT: popq	%rbx
//JIT-NEXT: popq	%r15
//JIT-NEXT: popq	%rbp
//JIT-NEXT: retq

//JIT-LABEL: ;SLOW PATHS
//JIT-NEXT:  leaq {{.*}}
//JIT-NEXT:  leaq {{.*}}
//JIT-NEXT:  movq {{.*}}
//JIT-NEXT:  callq {{.*}}
//JIT-NEXT:  testl {{.*}}
//JIT-NEXT:  je {{.*}}
//JIT-NEXT:  movq {{.*}}
//JIT-NEXT:  jmp {{.*}}


//JIT: 2
//JIT-NEXT: 8.35
//JIT-NEXT: 1.247.11
//JIT-NEXT: abc1
//JIT-NEXT: 13abc
//JIT-NEXT: abcxyz
//JIT-NEXT: NaN
//JIT-NEXT: undefinedxyz
