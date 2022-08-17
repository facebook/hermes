/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -non-strict -O -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -non-strict -O -target=HBC -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

if (typeof print === "undefined")
    var print = console.log;

function mustThrow(f) {
    try {
        f();
    } catch (e) {
        print("caught", e.name, e.message);
        return;
    }
    print("DID NOT THROW");
}

print("freeze");
//CHECK-LABEL: freeze

var a = [1,2,3];
Object.freeze(a);

a[1] = 10;
print(a[1]);
//CHECK-NEXT:2

a[10] = 10;
print(a.length, a[10]);
//CHECK-NEXT: 3 undefined

a.length = 0;
print(a.length, a[0]);
//CHECK-NEXT: 3 1

delete a[2];
print(a.length, a[2]);
//CHECK-NEXT: 3 3

mustThrow(function(){
    "use strict";
    a[1] = 10;
});
//CHECK-NEXT: caught TypeError {{.*}}

print("seal");
//CHECK-LABEL: seal

var a = [10,20,30];
Object.seal(a);

a[1] = 21;
print(a[1]);
//CHECK-NEXT: 21

a[10] = 100;
print(a.length, a[10]);
//CHECK-NEXT: 3 undefined

a.length = 10;
print(a.length);
//CHECK-NEXT: 10

a.length = 5;
print(a.length);
//CHECK-NEXT: 5

a.length = 0;
print(a.length, a[0]);
//CHECK-NEXT: 3 10

mustThrow(function() {
    "use strict";
    a.length = 0;
});
//CHECK-NEXT: caught TypeError {{.*}}
print(a.length, a[0]);
//CHECK-NEXT: 3 10

delete a[1];
print(a[1]);
//CHECK-NEXT: 21

mustThrow(function() {
    "use strict";
    delete a[1];
});
//CHECK-NEXT: caught TypeError {{.*}}
print(a[1]);
//CHECK-NEXT: 21

var a = []
a[10] = 10
a[11] = 11
a[12] = 12
a[13] = 13
a.length = 20;
print(String(a));
//CHECK-NEXT: ,,,,,,,,,,10,11,12,13,,,,,,

a.length = 13;
print(String(a));
//CHECK-NEXT: ,,,,,,,,,,10,11,12

delete a[12];
print(a.length + " " + String(a));
//CHECK-NEXT: 13 ,,,,,,,,,,10,11,

Object.seal(a);
a.length = 11;
print(a.length + " " + String(a));
//CHECK-NEXT: 12 ,,,,,,,,,,10,11
