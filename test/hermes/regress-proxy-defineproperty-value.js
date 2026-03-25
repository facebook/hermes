/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xes6-proxy -non-strict -O -target=HBC %s | %FileCheck --match-full-lines %s

var target = {x: 42};

var proxy = new Proxy(target, {
  defineProperty: function(obj, prop, desc) {
    print('has value:', Object.prototype.hasOwnProperty.call(desc, 'value'));
    print(
        'has writable:',
        Object.prototype.hasOwnProperty.call(desc, 'writable'));
    return Reflect.defineProperty(obj, prop, desc);
  },
});

Object.defineProperty(proxy, 'x', {writable: false});

print('target.x:', target.x);
print(
    'descriptor.writable:',
    Object.getOwnPropertyDescriptor(target, 'x').writable);

// CHECK: has value: false
// CHECK-NEXT: has writable: true
// CHECK-NEXT: target.x: 42
// CHECK-NEXT: descriptor.writable: false
