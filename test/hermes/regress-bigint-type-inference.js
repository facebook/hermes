/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -non-strict --dump-bytecode -O %s -fno-inline | %FileCheck %s --check-prefix=BC
// RUN: %hermes -non-strict -O %s -fno-inline | %FileCheck %s --check-prefix=EXE

// CHECK-LABEL: Function<testObject>
(function testObject() {
    var o = { valueOf: function() { return 1n; } };
    var result;

    (() => result = o + 1n)();
    print(result);
    // BC: Add
    // BC: StoreToEnvironment
    // EXE: 2

    (() => result = o - 1n)();
    print(result);
    // BC: Sub
    // BC: StoreToEnvironment
    // EXE: 0

    (() => result = o * 1n)();
    print(result);
    // BC: Mul
    // BC: StoreToEnvironment
    // EXE: 1

    (() => result = o / 1n)();
    print(result);
    // BC: Div
    // BC: StoreToEnvironment
    // EXE: 1

    (() => result = o % 1n)();
    print(result);
    // BC: Mod
    // BC: StoreToEnvironment
    // EXE: 0

    (() => result = o << 1n)();
    print(result);
    // BC: LShift
    // BC: StoreToEnvironment
    // EXE: 2

    (() => result = o >> 1n)();
    print(result);
    // BC: RShift
    // BC: StoreToEnvironment
    // EXE: 0

    (() => result = o | 1n)();
    print(result);
    // BC: BitOr
    // BC: StoreToEnvironment
    // EXE: 1

    (() => result = o ^ 1n)();
    print(result);
    // BC: BitXor
    // BC: StoreToEnvironment
    // EXE: 0

    (() => result = o & 1n)();
    print(result);
    // BC: BitAnd
    // BC: StoreToEnvironment
    // EXE: 1

    (() => result = - o)();
    print(result);
    // BC: Negate
    // BC: StoreToEnvironment
    // EXE: -1

    (() => result = ~ o)();
    print(result);
    // BC: BitNot
    // BC: StoreToEnvironment
    // EXE: -2

    var o1 = { valueOf: function() { return 1n; } };
    (() => result = o1++)();
    print(result);
    // BC: Inc
    // BC: StoreToEnvironment
    // EXE: 1

    var o2 = { valueOf: function() { return 1n; } };
    (() => result = o2--)();
    print(result);
    // BC: Dec
    // BC: StoreToEnvironment
    // EXE: 1

    var o3 = { valueOf: function() { return 1n; } };
    (() => result = ++o3)();
    print(result);
    // BC: Inc
    // BC: StoreToEnvironment
    // EXE: 2

    var o4 = { valueOf: function() { return 1n; } };
    (() => result = --o4)();
    print(result);
    // BC: Dec
    // BC: StoreToEnvironment
    // EXE: 0
})();

// CHECK-LABEL: Function<testRegex>
(function testRegex() {
    var o = /a/; o.valueOf = function() { return 1n; };
    var result;

    (() => result = o + 1n)();
    print(result);
    // BC: Add
    // BC: StoreToEnvironment
    // EXE: 2

    (() => result = o - 1n)();
    print(result);
    // BC: Sub
    // BC: StoreToEnvironment
    // EXE: 0

    (() => result = o * 1n)();
    print(result);
    // BC: Mul
    // BC: StoreToEnvironment
    // EXE: 1

    (() => result = o / 1n)();
    print(result);
    // BC: Div
    // BC: StoreToEnvironment
    // EXE: 1

    (() => result = o % 1n)();
    print(result);
    // BC: Mod
    // BC: StoreToEnvironment
    // EXE: 0

    (() => result = o << 1n)();
    print(result);
    // BC: LShift
    // BC: StoreToEnvironment
    // EXE: 2

    (() => result = o >> 1n)();
    print(result);
    // BC: RShift
    // BC: StoreToEnvironment
    // EXE: 0

    (() => result = o | 1n)();
    print(result);
    // BC: BitOr
    // BC: StoreToEnvironment
    // EXE: 1

    (() => result = o ^ 1n)();
    print(result);
    // BC: BitXor
    // BC: StoreToEnvironment
    // EXE: 0

    (() => result = o & 1n)();
    print(result);
    // BC: BitAnd
    // BC: StoreToEnvironment
    // EXE: 1

    (() => result = - o)();
    print(result);
    // BC: Negate
    // BC: StoreToEnvironment
    // EXE: -1

    (() => result = ~ o)();
    print(result);
    // BC: BitNot
    // BC: StoreToEnvironment
    // EXE: -2

    var o1 = /a/; o1.valueOf = function() { return 1n; };
    (() => result = o1++)();
    print(result);
    // BC: Inc
    // BC: StoreToEnvironment
    // EXE: 1

    var o2 = /a/; o2.valueOf = function() { return 1n; };
    (() => result = o2--)();
    print(result);
    // BC: Dec
    // BC: StoreToEnvironment
    // EXE: 1

    var o3 = /a/; o3.valueOf = function() { return 1n; };
    (() => result = ++o3)();
    print(result);
    // BC: Inc
    // BC: StoreToEnvironment
    // EXE: 2

    var o4 = /a/; o4.valueOf = function() { return 1n; };
    (() => result = --o4)();
    print(result);
    // BC: Dec
    // BC: StoreToEnvironment
    // EXE: 0
 })();

 // CHECK-LABEL: Function<testClosure>
 (function testClosure() {
    var o = _ => o; o.valueOf = function() { return 1n; };
    var result;

    (() => result = o + 1n)();
    print(result);
    // BC: Add
    // BC: StoreToEnvironment
    // EXE: 2

    (() => result = o - 1n)();
    print(result);
    // BC: Sub
    // BC: StoreToEnvironment
    // EXE: 0

    (() => result = o * 1n)();
    print(result);
    // BC: Mul
    // BC: StoreToEnvironment
    // EXE: 1

    (() => result = o / 1n)();
    print(result);
    // BC: Div
    // BC: StoreToEnvironment
    // EXE: 1

    (() => result = o % 1n)();
    print(result);
    // BC: Mod
    // BC: StoreToEnvironment
    // EXE: 0

    (() => result = o << 1n)();
    print(result);
    // BC: LShift
    // BC: StoreToEnvironment
    // EXE: 2

    (() => result = o >> 1n)();
    print(result);
    // BC: RShift
    // BC: StoreToEnvironment
    // EXE: 0

    (() => result = o | 1n)();
    print(result);
    // BC: BitOr
    // BC: StoreToEnvironment
    // EXE: 1

    (() => result = o ^ 1n)();
    print(result);
    // BC: BitXor
    // BC: StoreToEnvironment
    // EXE: 0

    (() => result = o & 1n)();
    print(result);
    // BC: BitAnd
    // BC: StoreToEnvironment
    // EXE: 1

    (() => result = - o)();
    print(result);
    // BC: Negate
    // BC: StoreToEnvironment
    // EXE: -1

    (() => result = ~ o)();
    print(result);
    // BC: BitNot
    // BC: StoreToEnvironment
    // EXE: -2

    var o1 = _ => o1; o1.valueOf = function() { return 1n; };
    (() => result = o1++)();
    print(result);
    // BC: Inc
    // BC: StoreToEnvironment
    // EXE: 1

    var o2 = _ => o2; o2.valueOf = function() { return 1n; };
    (() => result = o2--)();
    print(result);
    // BC: Dec
    // BC: StoreToEnvironment
    // EXE: 1

    var o3 = _ => o3; o3.valueOf = function() { return 1n; };
    (() => result = ++o3)();
    print(result);
    // BC: Inc
    // BC: StoreToEnvironment
    // EXE: 2

    var o4 = _ => o4; o4.valueOf = function() { return 1n; };
    (() => result = --o4)();
    print(result);
    // BC: Dec
    // BC: StoreToEnvironment
    // EXE: 0
})();
