/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xes6-proxy -non-strict -O -target=HBC %s | %FileCheck --match-full-lines %s

var setterCalled = false;
var target = {};

Object.defineProperty(target, 'x', {
  get: function() {
    return 42;
  },
  set: function(v) {
    setterCalled = true;
    print('setter called:', v);
  },
  configurable: true,
});

var proxy = new Proxy(target, {
  defineProperty: function(obj, prop, desc) {
    print('has get:', Object.prototype.hasOwnProperty.call(desc, 'get'));
    print('has set:', Object.prototype.hasOwnProperty.call(desc, 'set'));
    print('configurable:', desc.configurable);
    return Reflect.defineProperty(obj, prop, desc);
  },
});

Object.defineProperty(proxy, 'x', {
  get: function() {
    return 99;
  },
  configurable: true,
});

target.x = 5;

print('setter intact:', setterCalled);

// CHECK: has get: true
// CHECK-NEXT: has set: false
// CHECK-NEXT: configurable: true
// CHECK-NEXT: setter called: 5
// CHECK-NEXT: setter intact: true
