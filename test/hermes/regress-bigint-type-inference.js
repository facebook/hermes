/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -non-strict --dump-bytecode -O %s -fno-inline | %FileCheck %s --check-prefix=BC
// RUN: %hermes -non-strict -O %s -fno-inline | %FileCheck %s --check-prefix=EXE

function test() {
    var o = { valueOf() { return 1n; } }
    var bigint = 1n;
    var result;

    (() => result = o + bigint)();
    print(result);
    // BC: Add
    // BC: StoreToEnvironment
    // EXE: 2

    (() => result = o - bigint)();
    print(result);
    // BC: Sub
    // BC: StoreToEnvironment
    // EXE: 0

    (() => result = o * bigint)();
    print(result);
    // BC: Mul
    // BC: StoreToEnvironment
    // EXE: 1

    (() => result = o / bigint)();
    print(result);
    // BC: Div
    // BC: StoreToEnvironment
    // EXE: 1

    (() => result = o % bigint)();
    print(result);
    // BC: Mod
    // BC: StoreToEnvironment
    // EXE: 0

    (() => result = o << bigint)();
    print(result);
    // BC: LShift
    // BC: StoreToEnvironment
    // EXE: 2

    (() => result = o >> bigint)();
    print(result);
    // BC: RShift
    // BC: StoreToEnvironment
    // EXE: 0

    (() => result = o | bigint)();
    print(result);
    // BC: BitOr
    // BC: StoreToEnvironment
    // EXE: 1

    (() => result = o ^ bigint)();
    print(result);
    // BC: BitXor
    // BC: StoreToEnvironment
    // EXE: 0

    (() => result = o & bigint)();
    print(result);
    // BC: BitAnd
    // BC: StoreToEnvironment
    // EXE: 1

    (() => result = - bigint)();
    print(result);
    // BC: Negate
    // BC: StoreToEnvironment
    // EXE: -1

    (() => result = ~ bigint)();
    print(result);
    // BC: BitNot
    // BC: StoreToEnvironment
    // EXE: -2

    (() => result = o++)();
    print(result);
    // BC: Inc
    // BC: StoreToEnvironment
    // EXE: 1

    (() => result = o--)();
    print(result);
    // BC: Dec
    // BC: StoreToEnvironment
    // EXE: 2

    (() => result = ++o)();
    print(result);
    // BC: Inc
    // BC: StoreToEnvironment
    // EXE: 2

    (() => result = --o)();
    print(result);
    // BC: Dec
    // BC: StoreToEnvironment
    // EXE: 1
}

test();
