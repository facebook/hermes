/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 -dump-bytecode -target=HBC -g %s | %FileCheck %s --match-full-lines --check-prefixes=CHECK,ASCII
// RUN: cp %s %T/debug_info_à.js && %hermes -O0 -dump-bytecode -target=HBC -g %T/debug_info_à.js | %FileCheck %s --match-full-lines --check-prefixes=CHECK,UNICODE

var v1g = "global";

function Fa() {
    var v1a = 3;
    var v2a = 5;
}
// CHECK: Function<Fa>{{.*}}
// CHECK-NEXT: Offset in debug table: {{.*}}

function Fb() {
    var v1b = "abc";
}
// CHECK: Function<Fb>{{.*}}
// CHECK-NEXT: Offset in debug table: {{.*}}

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

// CHECK:      Debug filename table:
// ASCII-NEXT:   0: {{.*}}/debug_info.js
// UNICODE-NEXT:   0: {{.*}}/debug_info_à.js

// CHECK:      Debug file table:
// CHECK-NEXT:   source table offset 0x0000: filename id 0

// We expect 5 "function idx" lines since we have 4 functions plus the global.
// CHECK:      Debug source table:
// CHECK-NEXT:   0x{{[0-9a-f]+}}  function idx 0, {{.*}}
// CHECK:   0x{{[0-9a-f]+}}  function idx 1, {{.*}}
// CHECK:   0x{{[0-9a-f]+}}  function idx 2, {{.*}}
// CHECK:   0x{{[0-9a-f]+}}  function idx 3, {{.*}}
// CHECK:   0x{{[0-9a-f]+}}  function idx 4, {{.*}}
// CHECK:   0x{{[0-9a-f]+}}  end of debug source table

// CHECK: Debug scope descriptor table:
// CHECK-NEXT:  0x0000  lexical parent:   none, flags:   , variable count: 0
// CHECK-NEXT:  0x0003  lexical parent: 0x0000, flags:   , variable count: 2
// CHECK-NEXT:    "v1a"
// CHECK-NEXT:    "v2a"
// CHECK-NEXT:  0x0008  lexical parent: 0x0000, flags:   , variable count: 1
// CHECK-NEXT:    "v1b"
// CHECK-NEXT:  0x000c  lexical parent: 0x0000, flags:   , variable count: 2
// CHECK-NEXT:    "v1c"
// CHECK-NEXT:    "Fcc"
// CHECK-NEXT:  0x0011  lexical parent: 0x000c, flags:   , variable count: 1
// CHECK-NEXT:    "v1cc"
// CHECK-NEXT:  0x0015  end of debug scope descriptor table

// CHECK: Textified callees table:
// CHECK-NEXT:   0x0000  entries: 0
// CHECK-NEXT:   0x0001  end of textified callees table

// CHECK: Debug string table:
// CHECK-NEXT:   0x0000 v1a
// CHECK-NEXT:   0x0004 v2a
// CHECK-NEXT:   0x0008 v1b
// CHECK-NEXT:   0x000c v1c
// CHECK-NEXT:   0x0010 Fcc
// CHECK-NEXT:   0x0014 v1cc
// CHECK-NEXT:   0x0019  end of debug string table
