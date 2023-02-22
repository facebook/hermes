/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// REQUIRES: slow_debug

// RUN: %hermesc -target=HBC -fno-inline -debug-only regalloc -dump-bytecode -O0 %s 2>&1 | %FileCheck --match-full-lines %s --check-prefix=NOPC
// RUN: %hermesc -target=HBC -fno-inline -debug-only regalloc -dump-bytecode -O0 -g %s 2>&1 | %FileCheck --match-full-lines %s --check-prefix=PC
// RUN: %hermesc -target=HBC -fno-inline -debug-only regalloc -dump-bytecode -O0 -g0 %s 2>&1 | %FileCheck --match-full-lines %s --check-prefix=NOPC
// RUN: %hermesc -target=HBC -fno-inline -debug-only regalloc -dump-bytecode -O0 -g1 %s 2>&1 | %FileCheck --match-full-lines %s --check-prefix=NOPC
// RUN: %hermesc -target=HBC -fno-inline -debug-only regalloc -dump-bytecode -O0 -g2 %s 2>&1 | %FileCheck --match-full-lines %s --check-prefix=NOPC
// RUN: %hermesc -target=HBC -fno-inline -debug-only regalloc -dump-bytecode -O0 -g3 %s 2>&1 | %FileCheck --match-full-lines %s --check-prefix=PC

// RUN: %hermesc -target=HBC -fno-inline -debug-only regalloc -dump-bytecode -O %s 2>&1 | %FileCheck --match-full-lines %s --check-prefix=NOPC
// RUN: %hermesc -target=HBC -fno-inline -debug-only regalloc -dump-bytecode -O -g %s 2>&1 | %FileCheck --match-full-lines %s --check-prefix=PC
// RUN: %hermesc -target=HBC -fno-inline -debug-only regalloc -dump-bytecode -O -g0 %s 2>&1 | %FileCheck --match-full-lines %s --check-prefix=NOPC
// RUN: %hermesc -target=HBC -fno-inline -debug-only regalloc -dump-bytecode -O -g1 %s 2>&1 | %FileCheck --match-full-lines %s --check-prefix=NOPC
// RUN: %hermesc -target=HBC -fno-inline -debug-only regalloc -dump-bytecode -O -g2 %s 2>&1 | %FileCheck --match-full-lines %s --check-prefix=NOPC
// RUN: %hermesc -target=HBC -fno-inline -debug-only regalloc -dump-bytecode -O -g3 %s 2>&1 | %FileCheck --match-full-lines %s --check-prefix=PC

function sink(n) {
    return n;
}

// In No-PreColoring mode (i.e., -g2 or less) there should be no messages about registers being reserved.
// NOPC-NOT: Reserving register for ScopeCreationInst in

print((function _1() {
// PC: Reserving register for ScopeCreationInst in _1
// PC-NOT: Reserving register for ScopeCreationInst in _1
    var a = sink(1);
    return (function _2() {
// PC: Reserving register for ScopeCreationInst in _2
// PC-NOT: Reserving register for ScopeCreationInst in _2
        var b = sink(1);
        return (function _3() {
// PC: Reserving register for ScopeCreationInst in _3
// PC-NOT: Reserving register for ScopeCreationInst in _3
            var c = sink(1);
            return (function _4() {
// PC: Reserving register for ScopeCreationInst in _4
// PC-NOT: Reserving register for ScopeCreationInst in _4
                var d = sink(1);
                return (function _5() {
// PC: Reserving register for ScopeCreationInst in _5
// PC-NOT: Reserving register for ScopeCreationInst in _5
                    var e = sink(1);
                    return (function _6() {
// PC: Reserving register for ScopeCreationInst in _6
// PC-NOT: Reserving register for ScopeCreationInst in _6
                        var f = sink(1);
                        return (function _7() {
// PC: Reserving register for ScopeCreationInst in _7
// PC-NOT: Reserving register for ScopeCreationInst in _7
                            var g = sink(1);
                            return (function _8() {
// PC: Reserving register for ScopeCreationInst in _8
// PC-NOT: Reserving register for ScopeCreationInst in _8
                                var h = sink(1);
                                return (function _9() {
// PC: Reserving register for ScopeCreationInst in _9
// PC-NOT: Reserving register for ScopeCreationInst in _9
                                    var i = sink(1);
                                    return (function _10() {
// PC: Reserving register for ScopeCreationInst in _10
// PC-NOT: Reserving register for ScopeCreationInst in _10
                                        var j = sink(1);
                                        return (function _11() {
                                            return a + b + c + d + e + f + g + h + i + j + k;
                                        })()
                                    })()
                                })()
                            })()
                        })()
                    })()
                })()
            })()
        })()
    })()
})())
