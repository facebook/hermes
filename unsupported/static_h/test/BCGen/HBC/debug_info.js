/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
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
// CHECK-NEXT:   source table offset 0x0000: filename id 0
// UNICODE:      Debug file table:
// UNICODE-NEXT:   source table offset 0x0000: filename id 0

// We expect 5 "function idx" lines since we have 4 functions plus the global.
// CHECK:      Debug source table:
// CHECK-NEXT:   0x{{[0-9a-f]+}}  function idx 0, {{.*}}
// CHECK:   0x{{[0-9a-f]+}}  function idx 1, {{.*}}
// CHECK:   0x{{[0-9a-f]+}}  function idx 2, {{.*}}
// CHECK:   0x{{[0-9a-f]+}}  function idx 3, {{.*}}
// CHECK:   0x{{[0-9a-f]+}}  function idx 4, {{.*}}
// CHECK:   0x{{[0-9a-f]+}}  end of debug source table
// CHECK: Debug lexical table:
// CHECK-NEXT:   0x0000  lexical parent: none, variable count: 0
// CHECK-NEXT:   0x0002  lexical parent: 0, variable count: 2
// CHECK-NEXT:     "v1a"
// CHECK-NEXT:     "v2a"
// CHECK-NEXT:   0x000c  lexical parent: 0, variable count: 1
// CHECK-NEXT:     "v1b"
// CHECK-NEXT:   0x0012  lexical parent: 0, variable count: 2
// CHECK-NEXT:     "v1c"
// CHECK-NEXT:     "Fcc"
// CHECK-NEXT:   0x001c  lexical parent: 3, variable count: 1
// CHECK-NEXT:     "v1cc"
// CHECK-NEXT:   0x0023  end of debug lexical table

// UNICODE:      Debug source table:
// UNICODE-NEXT:   0x{{[0-9a-f]+}}  function idx 0, {{.*}}
// UNICODE:   0x{{[0-9a-f]+}}  function idx 1, {{.*}}
// UNICODE:   0x{{[0-9a-f]+}}  function idx 2, {{.*}}
// UNICODE:   0x{{[0-9a-f]+}}  function idx 3, {{.*}}
// UNICODE:   0x{{[0-9a-f]+}}  function idx 4, {{.*}}
// UNICODE:   0x{{[0-9a-f]+}}  end of debug source table
// UNICODE: Debug lexical table:
// UNICODE-NEXT:   0x0000  lexical parent: none, variable count: 0
// UNICODE-NEXT:   0x0002  lexical parent: 0, variable count: 2
// UNICODE-NEXT:     "v1a"
// UNICODE-NEXT:     "v2a"
// UNICODE-NEXT:   0x000c  lexical parent: 0, variable count: 1
// UNICODE-NEXT:     "v1b"
// UNICODE-NEXT:   0x0012  lexical parent: 0, variable count: 2
// UNICODE-NEXT:     "v1c"
// UNICODE-NEXT:     "Fcc"
// UNICODE-NEXT:   0x001c  lexical parent: 3, variable count: 1
// UNICODE-NEXT:     "v1cc"
// UNICODE-NEXT:   0x0023  end of debug lexical table
