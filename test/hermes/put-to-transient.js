/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -Wno-undefined-variable %s | %FileCheck --match-full-lines %s
// RUN: %hermes -target=HBC -Wno-undefined-variable -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s
"use strict";

if (typeof print === "undefined")
    var print = console.log;

print("put-to-transient");
//CHECK-LABEL: put-to-transient

var x = 1;

try {
    x.foo = 10;
} catch (e) {
    print("caught:", e.name, e.message);
}
//CHECK-NEXT: caught: TypeError {{.*}}

Number.prototype.baseProp = "value";
try {
    x.baseProp = 10;
} catch (e) {
    print("caught:", e.name, e.message);
}
//CHECK-NEXT: caught: TypeError {{.*}}

Object.defineProperty(Number.prototype, "accessor", {set: function(v) {
    print("setting", v);
}});
x.accessor = 10;
//CHECK-NEXT: setting 10

// Computed properties.
var propName = "p" + Math.round(Math.random()*100);

try {
    x[propName] = 10;
} catch (e) {
    print("caught:", e.name, e.message);
}
//CHECK-NEXT: caught: TypeError {{.*}}

propName += "a";

Number.prototype[propName] = "value";
try {
    x[propName] = 10;
} catch (e) {
    print("caught:", e.name, e.message);
}
//CHECK-NEXT: caught: TypeError {{.*}}

propName += "b";
Object.defineProperty(Number.prototype, propName, {set: function(v) {
    print("setting", v);
}});
x[propName] = 10;
//CHECK-NEXT: setting 10

// String computed properties.
propName = Math.round(Math.random()*5);

try {
    x[1] = "1";
} catch (e) {
    print("caught:", e.name, e.message);
}
//CHECK-NEXT: caught: TypeError {{.*}}
try {
    x[propName] = "1";
} catch (e) {
    print("caught:", e.name, e.message);
}
//CHECK-NEXT: caught: TypeError {{.*}}

try {
    x[20] = "1";
} catch (e) {
    print("caught:", e.name, e.message);
}
//CHECK-NEXT: caught: TypeError {{.*}}
