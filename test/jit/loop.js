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

var logger = typeof print === "undefined"
    ? console.log
    : print;

function power (base, pow) {
    var res = 1;
    while (--pow >= 0) {
        res *= base;
    }
    return res;
}

logger(power(2, 10));


//HBC-LABEL: Function<power>(3 params, 7 registers, 0 symbols):
//HBC-NEXT: Offset in debug table: {{.*}}
//HBC-NEXT: LoadParam         r5, 1
//HBC-NEXT: LoadConstUInt8    r4, 1
//HBC-NEXT: LoadConstZero     r3
//HBC-NEXT: LoadParam         r0, 2
//HBC-NEXT: ToNumber          r0, r0
//HBC-NEXT: SubN              r2, r0, r4
//HBC-NEXT: Mov               r1, r4
//HBC-NEXT: Mov               r0, r1
//HBC-NEXT: JNotGreaterEqualN L1, r2, r3
//HBC-NEXT: L2:
//HBC-NEXT: Mul               r1, r1, r5
//HBC-NEXT: SubN              r2, r2, r4
//HBC-NEXT: Mov               r0, r1
//HBC-NEXT: JGreaterEqualN    L2, r2, r3
//HBC-NEXT: L1:
//HBC-NEXT: Ret               r0


//JIT-LABEL:Compiled Code of FunctionID: 1
//JIT-NEXT:pushq	%rbp
//JIT-NEXT:movq	%rsp, %rbp
//JIT-NEXT:pushq	%r15
//JIT-NEXT:pushq	%rbx
//JIT-NEXT:movq	%rdi, %rbx
//JIT-NEXT:pushq	3968(%rbx)
//JIT-NEXT:pushq	%rcx
//JIT-NEXT:movq	3952(%rbx), %r15
//JIT-NEXT:movq	%r15, 3968(%rbx)
//JIT-NEXT:leaq	{{.*}}, %rax
//JIT-NEXT:movq	%rax, 3952(%rbx)
//JIT-NEXT: BB0:
//JIT-NEXT: movq {{.*}}
//JIT-NEXT: cmpl{{.*}}
//JIT-NEXT: jb{{.*}}
//JIT-NEXT: movq {{.*}}
//JIT-NEXT: movq {{.*}}
//JIT-NEXT: movq {{.*}}
//JIT-NEXT: movq {{.*}}
//JIT-NEXT: xorq {{.*}}
//JIT-NEXT: movq {{.*}}
//JIT-NEXT: movq {{.*}}
//JIT-NEXT: cmpl{{.*}}
//JIT-NEXT: jb{{.*}}
//JIT-NEXT: movq {{.*}}
//JIT-NEXT: movq {{.*}}
//JIT-NEXT: cmpl{{.*}}
//JIT-NEXT: jae{{.*}}
//JIT-NEXT: movsd{{.*}}
//JIT-NEXT: subsd{{.*}}
//JIT-NEXT: movsd{{.*}}
//JIT-NEXT: movq {{.*}}
//JIT-NEXT: movq {{.*}}
//JIT-NEXT: movq {{.*}}
//JIT-NEXT: movq {{.*}}
//JIT-NEXT: movsd{{.*}}
//JIT-NEXT: ucomisd{{.*}}
//JIT-NEXT: jb{{.*}}
//JIT-NEXT: BB1:
//JIT-NEXT: cmpl{{.*}}
//JIT-NEXT: jae{{.*}}
//JIT-NEXT: cmpl{{.*}}
//JIT-NEXT: jae{{.*}}
//JIT-NEXT: movsd{{.*}}
//JIT-NEXT: mulsd{{.*}}
//JIT-NEXT: movsd{{.*}}
//JIT-NEXT: movsd{{.*}}
//JIT-NEXT: subsd{{.*}}
//JIT-NEXT: movsd{{.*}}
//JIT-NEXT: movq {{.*}}
//JIT-NEXT: movq {{.*}}
//JIT-NEXT: movsd{{.*}}
//JIT-NEXT: ucomisd{{.*}}
//JIT-NEXT: jae{{.*}}
//JIT-NEXT: BB2:
//JIT-NEXT: movq {{.*}}
//JIT-NEXT: movl {{.*}}
//JIT-NEXT: BB3:
//JIT-NEXT:movq	%r15, 3952(%rbx)
//JIT-NEXT:popq	%rcx
//JIT-NEXT:popq	3968(%rbx)
//JIT-NEXT:popq	%rbx
//JIT-NEXT:popq	%r15
//JIT-NEXT:popq	%rbp
//JIT-NEXT:retq

//JIT-LABEL:;SLOW PATHS
//JIT-NEXT: leaq {{.*}}
//JIT-NEXT: movq {{.*}}
//JIT-NEXT: callq {{.*}}
//JIT-NEXT: testl{{.*}}
//JIT-NEXT: je {{.*}}
//JIT-NEXT: movq {{.*}}
//JIT-NEXT: jmp {{.*}}

//JIT: leaq {{.*}}
//JIT-NEXT: leaq {{.*}}
//JIT-NEXT: movq {{.*}}
//JIT-NEXT: callq {{.*}}
//JIT-NEXT: testl{{.*}}
//JIT-NEXT: je {{.*}}
//JIT-NEXT: movq {{.*}}
//JIT-NEXT: jmp {{.*}}

//JIT: 1024
