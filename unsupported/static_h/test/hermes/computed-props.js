/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck %s --match-full-lines
// RUN: %hermes -O %s | %FileCheck %s --match-full-lines

print('computed properties');
// CHECK-LABEL: computed properties

function func() {
  return 'funcprop';
}

var count = 0;
var keyObj = {
  toString: null,
  valueOf: function() {
    count++;
    return 1;
  },
};

var obj = {
  ['x']: 3,
  ['another' + 'property']: 42,
  [func()]: 81,
  [keyObj]: 52,
};

print(obj.x);
// CHECK-NEXT: 3
print(obj.anotherproperty);
// CHECK-NEXT: 42
print(obj.funcprop);
// CHECK-NEXT: 81
print(obj[1], count);
// CHECK-NEXT: 52 1

var getset = {
  get ['x']() {
    return 13;
  },
  set ['x'](val) {
    print('set', val);
  }
};

print(getset['x']);
// CHECK-NEXT: 13
getset['x'] = 100;
// CHECK-NEXT: set 100
print(getset['x']);
// CHECK-NEXT: 13
getset['x'] = 101;
// CHECK-NEXT: set 101

var mixed_props = {
  ['b']: 'b',
  b: 'B',
};

print(Object.keys(mixed_props));
// CHECK-NEXT: b

var getters = {
  get ['x']() {},
  x: 3,
};
print(getters.x);
// CHECK-NEXT: 3
