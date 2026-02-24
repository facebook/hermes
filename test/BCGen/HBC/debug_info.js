/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 -dump-bytecode -target=HBC -g %s | %FileCheck %s --match-full-lines
// RUN: cat %s > %T/debug_info_à.js && %hermes -O0 -dump-bytecode -target=HBC -g %T/debug_info_à.js | %FileCheck %s --match-full-lines --check-prefix=UNICODE

var v1g = "global";

function Fa() {
    var v1a = 3;
    var v2a = 5;
}

// CHECK:Function<Fa>({{.*}}):
// CHECK-NEXT:Offset in debug table: {{.*}}
// UNICODE:Function<Fa>({{.*}}):
// UNICODE-NEXT:Offset in debug table: {{.*}}

function Fb() {
    var v1b = "abc";
}

// CHECK:Function<Fb>({{.*}}):
// CHECK-NEXT:Offset in debug table: {{.*}}
// UNICODE:Function<Fb>({{.*}}):
// UNICODE-NEXT:Offset in debug table: {{.*}}

function Fc() {
    var v1c = undefined;
    function Fcc() {
        var v1cc = 42;
    }
}

// CHECK:Function<Fc>({{.*}}):
// CHECK-NEXT:Offset in debug table: {{.*}}
// UNICODE:Function<Fc>({{.*}}):
// UNICODE-NEXT:Offset in debug table: {{.*}}

// CHECK:Debug filename table:
// CHECK-NEXT:  0: {{.*}}debug_info.js
// UNICODE:Debug filename table:
// UNICODE-NEXT:  0: {{.*}}debug_info_à.js

// We expect 5 "function idx" lines since we have 4 functions plus the global.
// CHECK:Debug source table:
// CHECK-NEXT:  0x{{[0-9a-f]+}}  function idx 0, starts at {{.*}}
// CHECK:  0x{{[0-9a-f]+}}  function idx 1, starts at {{.*}}
// CHECK:  0x{{[0-9a-f]+}}  function idx 2, starts at {{.*}}
// CHECK:  0x{{[0-9a-f]+}}  function idx 3, starts at {{.*}}
// CHECK:  0x{{[0-9a-f]+}}  function idx 4, starts at {{.*}}
// CHECK:  0x{{[0-9a-f]+}}  end of debug source table
// UNICODE:Debug source table:
// UNICODE-NEXT:  0x{{[0-9a-f]+}}  function idx 0, starts at {{.*}}
// UNICODE:  0x{{[0-9a-f]+}}  function idx 1, starts at {{.*}}
// UNICODE:  0x{{[0-9a-f]+}}  function idx 2, starts at {{.*}}
// UNICODE:  0x{{[0-9a-f]+}}  function idx 3, starts at {{.*}}
// UNICODE:  0x{{[0-9a-f]+}}  function idx 4, starts at {{.*}}
// UNICODE:  0x{{[0-9a-f]+}}  end of debug source table
