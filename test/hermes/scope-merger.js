/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s -bs | %FileCheck %s --check-prefix EXEC
// RUN: %hermes -O %s -bs -g --dump-bytecode -pretty-disassemble=false | %FileCheck %s --check-prefix BC

"use strict";

var print = typeof print !== "undefined" ? print : console.log;

function sink(v) {
    return v;
}

function f() {
    const ret = [];
    let i = 0;
    while (i < 10) {
        let a = i;
        if (sink(i)) {
            ret.push(() => ++a);
        }
        ++i;
    }
    return ret;
}

const C = f();
for (let i = 0; i < 10; ++i) {
    let str = "";
    for (const c of C) {
        str += `${c()} `;
    }
    print(str);
}

// EXEC: 2 3 4 5 6 7 8 9 10
// EXEC-NEXT: 3 4 5 6 7 8 9 10 11
// EXEC-NEXT: 4 5 6 7 8 9 10 11 12
// EXEC-NEXT: 5 6 7 8 9 10 11 12 13
// EXEC-NEXT: 6 7 8 9 10 11 12 13 14
// EXEC-NEXT: 7 8 9 10 11 12 13 14 15
// EXEC-NEXT: 8 9 10 11 12 13 14 15 16
// EXEC-NEXT: 9 10 11 12 13 14 15 16 17
// EXEC-NEXT: 10 11 12 13 14 15 16 17 18
// EXEC-NEXT: 11 12 13 14 15 16 17 18 19

function g(val) {
    var c = [];
    for (let e of [1,2,3]) {
      let h = e;
      c.push(_ => (val += 4)); // line 5
    }
    for (g of c) { g(); }
    return val;
}

print(g(10));

// EXEC-NEXT: 22

// BC-LABEL: Function<f>{{.*}}
// BC: Offset in debug table: source 0x{{[0-9a-f]+}}, scope 0x{{[0-9a-f]+}}
// BC:      [@ {{[0-9]+}}] CreateEnvironment [[ENV:[0-9]+]]<Reg8>
// BC:      [@ {{[0-9]+}}] CreateInnerEnvironment {{[0-9]+}}<Reg8>, [[ENV]]<Reg8>, 1<UInt32>

// BC-LABEL: Function<g>(2 params, 16 registers, 1 symbols):
// BC: Offset in debug table: source 0x{{[0-9a-f]+}}, scope 0x{{[0-9a-f]+}}
// BC:      [@ {{[0-9]+}}] CreateEnvironment [[ENV:[0-9]+]]<Reg8>
// BC-NOT: CreateInnerEnvironment

// BC-LABEL: Exception Handlers:

// BC-LABEL: Debug scope descriptor table:
// BC:    0x{{[0-9a-f]+}}  lexical parent: 0x{{[0-9a-f]+}}, flags: IsD, variable count: 1
// BC-NEXT:    "a"
// BC: 0x{{[0-9a-f]+}}  lexical parent: 0x{{[0-9a-f]+}}, flags:    , variable count: 1
// BC-NEXT:   "val"
// BC-NEXT: 0x{{[0-9a-f]+}}  lexical parent: 0x{{[0-9a-f]+}}, flags:    , variable count: 0
