/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s

// This elicits particular caching behavior to ensure that a hidden class without a property map does not trigger a false positive in the prototype property cache.

function print_a(obj) {
    print(obj.a);
}

var proto = {a: "protoprop"};
var obj = {};

// Populate the proto cache.
obj.__proto__ = proto;
print_a(obj);
// CHECK: protoprop

// Populate obj. Ensure its hidden class is distinct from its
// proto's hideden class, because we want the proto cache to be
// wrong. So start with a different property.
obj.b = "whatever";
obj.a = "ownprop";

// Make a new object with a hidden class that steals its property
// map from obj.
var thief = {};
thief.b = "thiefprop";
thief.a = "thiefprop";
thief.c = "thiefprop";

// Now obj's hidden class has no property map, and the cache points
// at its proto. We should NOT use the cache in this case.
print_a(obj);
// CHECK-NEXT: ownprop
