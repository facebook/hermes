/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 -dump-bytecode -target=HBC -g %s | %FileCheck %s --match-full-lines
// RUN: cp %s %T/debug_info_à.js && %hermes -O0 -dump-bytecode -target=HBC -g %T/debug_info_à.js | %FileCheck %s --match-full-lines --check-prefix=UNICODE

var v1g = "global";

function Fa() {
    var v1a = 3;
    var v2a = 5;
}
// CHECK: Function<Fa>{{.*}}
// CHECK-NEXT: Offset in debug table: {{.*}}
// UNICODE: Function<Fa>{{.*}}
// UNICODE-NEXT: Offset in debug table: {{.*}}


function Fb() {
    var v1b = "abc";
}
// CHECK: Function<Fb>{{.*}}
// CHECK-NEXT: Offset in debug table: {{.*}}
// UNICODE: Function<Fb>{{.*}}
// UNICODE-NEXT: Offset in debug table: {{.*}}

function Fc() {
    var v1c = undefined;
    function Fcc() {
        var v1cc = 42;
    }
}
// CHECK: Function<Fc>{{.*}}
// CHECK-NEXT: Offset in debug table: {{.*}}
// CHECK: Function<Fcc>(1 params, 4 registers, 1 symbols):
// CHECK-NEXT: Offset in debug table: {{.*}}
// UNICODE: Function<Fc>{{.*}}
// UNICODE-NEXT: Offset in debug table: {{.*}}
// UNICODE: Function<Fcc>(1 params, 4 registers, 1 symbols):
// UNICODE-NEXT: Offset in debug table: {{.*}}

// CHECK:      Debug filename table:
// CHECK-NEXT:   0: {{.*}}/debug_info.js
// UNICODE:      Debug filename table:
// UNICODE-NEXT:   0: {{.*}}/debug_info_à.js

// CHECK:      Debug file table:
// CHECK-NEXT:   Debug offset 0: string id 0
// UNICODE:      Debug file table:
// UNICODE-NEXT:   Debug offset 0: string id 0

// We expect 5 DebugOffset lines since we have 4 functions plus the global.
// CHECK:      Debug data table:
// CHECK-NEXT:   DebugOffset {{.*}}
// CHECK-NEXT:   DebugOffset {{.*}}
// CHECK-NEXT:   DebugOffset {{.*}}
// CHECK-NEXT:   DebugOffset {{.*}}
// CHECK-NEXT:   DebugOffset {{.*}}
// CHECK-NEXT:   Debug table ends at debugOffset {{.*}}
// CHECK-NEXT: Debug variables table:
// CHECK-NEXT:   Offset: 0x0, vars count: 0, lexical parent: none
// CHECK-NEXT:   Offset: 0x2, vars count: 2, lexical parent: 0
// CHECK-NEXT:     0x0004: "v1a"
// CHECK-NEXT:     0x0008: "v2a"
// CHECK-NEXT:   Offset: 0xc, vars count: 1, lexical parent: 0
// CHECK-NEXT:     0x000e: "v1b"
// CHECK-NEXT:   Offset: 0x12, vars count: 2, lexical parent: 0
// CHECK-NEXT:     0x0014: "v1c"
// CHECK-NEXT:     0x0018: "Fcc"
// CHECK-NEXT:   Offset: 0x1c, vars count: 1, lexical parent: 3
// CHECK-NEXT:     0x001e: "v1cc"

// UNICODE:      Debug data table:
// UNICODE-NEXT:   DebugOffset {{.*}}
// UNICODE-NEXT:   DebugOffset {{.*}}
// UNICODE-NEXT:   DebugOffset {{.*}}
// UNICODE-NEXT:   DebugOffset {{.*}}
// UNICODE-NEXT:   DebugOffset {{.*}}
// UNICODE-NEXT:   Debug table ends at debugOffset {{.*}}
// UNICODE-NEXT: Debug variables table:
// UNICODE-NEXT:   Offset: 0x0, vars count: 0, lexical parent: none
// UNICODE-NEXT:   Offset: 0x2, vars count: 2, lexical parent: 0
// UNICODE-NEXT:     0x0004: "v1a"
// UNICODE-NEXT:     0x0008: "v2a"
// UNICODE-NEXT:   Offset: 0xc, vars count: 1, lexical parent: 0
// UNICODE-NEXT:     0x000e: "v1b"
// UNICODE-NEXT:   Offset: 0x12, vars count: 2, lexical parent: 0
// UNICODE-NEXT:     0x0014: "v1c"
// UNICODE-NEXT:     0x0018: "Fcc"
// UNICODE-NEXT:   Offset: 0x1c, vars count: 1, lexical parent: 3
// UNICODE-NEXT:     0x001e: "v1cc"
