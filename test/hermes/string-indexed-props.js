/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -non-strict -O -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -non-strict -O -target=HBC -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s
// Check string indexed property access.

var s = "abc";
print(s.length, s[0], s[1], s[2], s[3]);
//CHECK: 3 a b c undefined

for(var i in s)
    print(i, s[i]);
//CHECK-NEXT: 0 a
//CHECK-NEXT: 1 b
//CHECK-NEXT: 2 c

// Assignment should be ignored in non-strict mode.
s[1] = "x";
print(s[1]);
//CHECK-NEXT: b

print(delete s[1]);
//CHECK-NEXT: false
print(delete s[3]);
//CHECK-NEXT: true

// Now check strict mode.
(function() {
    "use strict";
    try { s[1] = "x"; } catch (e) {
        print("caught 1", e.name, e.message);
//CHECK-NEXT: caught 1 TypeError {{.*}}
    }
    try { delete s[1]; } catch (e) {
        print("caught 2", e.name, e.message);
//CHECK-NEXT: caught 2 TypeError {{.*}}
    }
})();

// Property writes to a String object should fail in the valid range and
// succeed beyond it.
var so = new String("xyz");

// Assignment should be ignored in non-strict mode.
so[1] = "1";
print(so[1]);
//CHECK-NEXT: y

// Assignment should succeed.
so[5] = "foo";
print(so[5]);
//CHECK-NEXT: foo

// Check that indexed props are ok after killing the fast path above.
for(var i in so)
    print(i, so[i]);
//CHECK-NEXT: 0 x
//CHECK-NEXT: 1 y
//CHECK-NEXT: 2 z
//CHECK-NEXT: 5 foo

// Assignment should be ignored in non-strict mode.
so[2] = "3";
print(so[2]);
//CHECK-NEXT: z

// Assignment should succeed.
so[5] = "foo2";
print(so[5]);
//CHECK-NEXT: foo2
