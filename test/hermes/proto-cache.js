/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s

// Exercise the proto cache.
print(function () {
  var proto = {x: 7};
  var o = {};
  Object.setPrototypeOf(o, proto);

  function access(o) {
    'noinline'
    return o.x;
  }

  // o.x in second call to "access" should get a proto cache hit.
  return access(o) + access(o);
}());
// CHECK: 14

// Show that the proto cache doesn't get improper cache hits
// if the object changes to have the queried property.
print(function () {
  var proto = {x: 7};
  var o = {};
  Object.setPrototypeOf(o, proto);

  function access(o) {
    'noinline'
    return o.x;
  }

  // o.x in second call to "access" should get a proto cache hit.
  var sum = access(o);
  o.x = 700;
  sum += access(o);
  return sum;
}());
// CHECK: 707

