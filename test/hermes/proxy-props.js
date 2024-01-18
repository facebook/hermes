/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xes6-proxy -non-strict -O -target=HBC %s | %FileCheck --match-full-lines %s

// This is testing that the defineProperty trap of Proxy is triggered with a
// descriptor with proper attributes when a data property of a proxied object
// is assigned a new value.
// We had a bug that we passed a descriptor with incorrect attributes such that
// if the Proxy attempts to call Reflect.defineProperty() using the passed
// descriptor, it unexpectedly froze the property.
// For more detail, refer to https://github.com/facebook/hermes/issues/1065
// (T159152103)
var a = new Proxy({}, {
    defineProperty: (obj, prop, desc) => {
        // This print output is used for CHECK after each value assignment below.
        print(JSON.stringify(desc));
        return Reflect.defineProperty(obj, prop, desc);
    },
});

// The first time we assign a value to a property, the descriptor
// passed to the trap should have writable, enumerable, configurable
// with all true.
a.foo = 1;
//CHECK: {"value":1,"writable":true,"enumerable":true,"configurable":true}

print(a.foo)
//CHECK-NEXT: 1
const desc1 = Object.getOwnPropertyDescriptor(a, "foo");
print(desc1.configurable);
//CHECK-NEXT: true
print(desc1.enumerable);
//CHECK-NEXT: true
print(desc1.writable);
//CHECK-NEXT: true

// The second time we assign a value to a property, the descriptor
// passed to the trap should only have value.
a.foo = 2;
//CHECK-NEXT: {"value":2}

print(a.foo)
//CHECK-NEXT: 2
const desc2 = Object.getOwnPropertyDescriptor(a, "foo");
print(desc2.configurable);
//CHECK-NEXT: true
print(desc2.enumerable);
//CHECK-NEXT: true
print(desc2.writable);
//CHECK-NEXT: true

// Same as second case. Validating that foo is not unexpectedly fronzen.
a.foo = 3;
//CHECK-NEXT: {"value":3}
print(a.foo)
//CHECK-NEXT: 3
const desc3 = Object.getOwnPropertyDescriptor(a, "foo");
print(desc3.configurable);
//CHECK-NEXT: true
print(desc3.enumerable);
//CHECK-NEXT: true
print(desc3.writable);
//CHECK-NEXT: true
