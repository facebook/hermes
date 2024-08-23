/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

"use strict";

/*
Function<bench>(2 params, 6 registers):
Offset in debug table: source 0x001c, lexical 0x0000
    LoadParam         r0, 1
    Dec               r3, r0
    LoadConstUInt8    r2, 1
    Mov               r1, r0
    Mov               r0, r1
    JNotGreater       L1, r3, r2
L2:
    Mul               r1, r1, r3
    Dec               r3, r3
    Mov               r0, r1
    JGreater          L2, r3, r2
L1:
    Ret               r0
*/

function bench (fc) {
    var fact;
    fact = fc;
    while (--fc > 1)
        fact *= fc;
    return fact;
}

for(let i = 0; i != 1000; ++i)
    bench(100);
print(bench(100));

