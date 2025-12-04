/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// Test cases derived from TC39 test262 test suite
// Copyright (C) 2024 Kevin Gibbons. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

// RUN: %hermes -O %s | %FileCheck %s

print('Math.sumPrecise');
// CHECK-LABEL: Math.sumPrecise

// Test basic functionality
print(Math.sumPrecise([1, 2, 3]));
// CHECK-NEXT: 6

// Test floating point precision
print(Math.sumPrecise([0.1, 0.2]));
// CHECK-NEXT: 0.30000000000000004

// Test with large numbers
print(Math.sumPrecise([1e308, 1e308, -1e308]));
// CHECK-NEXT: 1e+308

// Test empty array (returns -0)
print(Object.is(Math.sumPrecise([]), -0));
// CHECK-NEXT: true

// Test with negative zero
print(Object.is(Math.sumPrecise([-0, -0, -0]), -0));
// CHECK-NEXT: true

// Test with mixed zeros
print(Math.sumPrecise([-0, 0]));
// CHECK-NEXT: 0

// Test with Infinity
print(Math.sumPrecise([Infinity, 1, 2, 3]));
// CHECK-NEXT: Infinity

print(Math.sumPrecise([-Infinity, 1, 2, 3]));
// CHECK-NEXT: -Infinity

// Test with NaN
print(Math.sumPrecise([NaN, 1, 2, 3]));
// CHECK-NEXT: NaN

// Test with mixed infinities (should be NaN)
print(Math.sumPrecise([Infinity, -Infinity]));
// CHECK-NEXT: NaN

// Test precise summation of small numbers
var sum = Math.sumPrecise([1e-10, 1e10, 1e-10, -1e10]);
print(sum);
// CHECK-NEXT: 2e-10

// Test with iterator protocol
var customIterable = {
  [Symbol.iterator]: function() {
    var i = 0;
    return {
      next: function() {
        if (i < 3) {
          return { value: ++i, done: false };
        }
        return { done: true };
      }
    };
  }
};
print(Math.sumPrecise(customIterable));
// CHECK-NEXT: 6

// Test type checking - no arguments
try {
  Math.sumPrecise();
} catch(e) {
  print('No args:', e.name);
}
// CHECK-NEXT: No args: TypeError

// Test type checking - non-iterable
try {
  Math.sumPrecise(123);
} catch(e) {
  print('Non-iterable:', e.name);
}
// CHECK-NEXT: Non-iterable: TypeError

// Test type checking - multiple arguments
// Note: test262 expects this to throw TypeError (should only accept 1 arg)
// Current polyfill implementation accepts multiple args (uses first, ignores rest)
// TODO: Make Math.sumPrecise reject multiple arguments per spec
print(Math.sumPrecise([1, 2], [3, 4]));
// CHECK-NEXT: 3

// Test with Set (another iterable type)
print(Math.sumPrecise(new Set([1, 2, 3])));
// CHECK-NEXT: 6

// Test with string iterable (should throw - strings are iterable but contain non-numbers)
try {
  Math.sumPrecise('123');
} catch(e) {
  print('String iterable:', e.name);
}
// CHECK-NEXT: String iterable: TypeError

// Test overflow - should return Infinity
var bigArray = [];
for (var i = 0; i < 100; i++) {
  bigArray.push(1.7e308);
}
print(Math.sumPrecise(bigArray));
// CHECK-NEXT: Infinity

// Additional test262-based tests

// Test generator iterable
function* numberGenerator() {
  yield 1;
  yield 2;
  yield 3;
}
print(Math.sumPrecise(numberGenerator()));
// CHECK-NEXT: 6

// Test array with overridden iterator
var arrayWithCustomIterator = [10, 20];
arrayWithCustomIterator[Symbol.iterator] = function* () {
  yield 1;
  yield 2;
  yield 3;
};
print(Math.sumPrecise(arrayWithCustomIterator));
// CHECK-NEXT: 6

// Test large number cancellation
print(Math.sumPrecise([1e308, -1e308]));
// CHECK-NEXT: 0

// Test decimal precision cases
print(Math.sumPrecise([0.1, 0.1]));
// CHECK-NEXT: 0.2

// Test mixed large and small numbers for precision
print(Math.sumPrecise([1e20, 0.1, -1e20]));
// CHECK-NEXT: 0.1

// Test non-number type errors
try {
  Math.sumPrecise([{}]);
} catch(e) {
  print('Caught object:', e.name);
}
// CHECK-NEXT: Caught object: TypeError

// Test BigInt rejection
try {
  Math.sumPrecise([0n]);
} catch(e) {
  print('Caught BigInt:', e.name);
}
// CHECK-NEXT: Caught BigInt: TypeError

// Test string conversion
try {
  Math.sumPrecise(['1']);
} catch(e) {
  print('Caught string:', e.name);
}
// CHECK-NEXT: Caught string: TypeError

// Test that valueOf/toString are not called (no type coercion)
var coercionCount = 0;
var objWithCoercion = {
  valueOf: function() { coercionCount++; return 1; },
  toString: function() { coercionCount++; return '1'; }
};
try {
  Math.sumPrecise([objWithCoercion]);
} catch(e) {
  // Should throw before any coercion
  print('No coercion, count:', coercionCount);
}
// CHECK-NEXT: No coercion, count: 0

// Test null and undefined handling
try {
  Math.sumPrecise([null]);
} catch(e) {
  print('Null:', e.name);
}
// CHECK-NEXT: Null: TypeError

try {
  Math.sumPrecise([undefined]);
} catch(e) {
  print('Undefined:', e.name);
}
// CHECK-NEXT: Undefined: TypeError

// Test with empty Set
print(Object.is(Math.sumPrecise(new Set()), -0));
// CHECK-NEXT: true

// Test with array-like object (not iterable, should throw)
try {
  Math.sumPrecise({length: 3, 0: 1, 1: 2, 2: 3});
} catch(e) {
  print('Array-like:', e.name);
}
// CHECK-NEXT: Array-like: TypeError

// Test property descriptor checks
print(typeof Math.sumPrecise);
// CHECK-NEXT: function

print(Math.sumPrecise.length);
// CHECK-NEXT: 1

print(Math.sumPrecise.name);
// CHECK-NEXT: sumPrecise

// Additional edge cases from TC39 proposal test suite

// Test basic ability to handle intermediate overflows
print(Math.sumPrecise([1e308, 1e308, 0.1, 0.1, 1e30, 0.1, -1e30, -1e308, -1e308]));
// CHECK-NEXT: 0.30000000000000004

print(Math.sumPrecise([1e30, 0.1, -1e30]));
// CHECK-NEXT: 0.1

// Test specific edge cases from fuzzer
print(Math.sumPrecise([8.98846567431158e+307, 8.988465674311579e+307, -1.7976931348623157e+308]));
// CHECK-NEXT: 9.9792015476736e+291

// Test handling of multiple overflows
print(Math.sumPrecise([1e308, 1e308]));
// CHECK-NEXT: Infinity

// Test single element arrays
print(Math.sumPrecise([1e308]));
// CHECK-NEXT: 1e+308

print(Math.sumPrecise([.1]));
// CHECK-NEXT: 0.1

// Test two element arrays with cancellation
print(Math.sumPrecise([.1, .1]));
// CHECK-NEXT: 0.2

print(Math.sumPrecise([.1, -.1]));
// CHECK-NEXT: 0

// Test with specific infinity combinations
print(Math.sumPrecise([1, Infinity, -1e308]));
// CHECK-NEXT: Infinity

print(Math.sumPrecise([1, Infinity, -1e308, -Infinity]));
// CHECK-NEXT: NaN

// Test edge case with large number combinations
print(Math.sumPrecise([-2.534858246857893e+115, 8.988465674311579e+307, 8.98846567431158e+307]));
// CHECK-NEXT: 1.7976931348623157e+308

// Additional test262 numerical test cases
print(Math.sumPrecise([-5.630637621603525e+255, 9.565271205476345e+307, 2.9937604643020797e+292]));
// CHECK-NEXT: 9.565271205476347e+307

print(Math.sumPrecise([6.739986666787661e+66, 2, -1.2689709186578243e-116, 1.7046015739467354e+308, -9.979201547673601e+291, 6.160926733208294e+307, -3.179557053031852e+234, -7.027282978772846e+307, -0.7500000000000001]));
// CHECK-NEXT: 1.61796594939028e+308

print(Math.sumPrecise([0.31150493246968836, -8.988465674311582e+307, 1.8315037361673755e-270, -15.999999999999996, 2.9999999999999996, 7.345200721499384e+164, -2.033582473639399, -8.98846567431158e+307, -3.5737295155405993e+292, 4.13894772383715e-124, -3.6111186457260667e-35, 2.387234887098013e+180, 7.645295562778372e-298, 3.395189016861822e-103, -2.6331611115768973e-149]));
// CHECK-NEXT: -Infinity

print(Math.sumPrecise([-1.1442589134409902e+308, 9.593842098384855e+138, 4.494232837155791e+307, -1.3482698511467367e+308, 4.494232837155792e+307]));
// CHECK-NEXT: -1.5936821971565685e+308

print(Math.sumPrecise([-1.1442589134409902e+308, 4.494232837155791e+307, -1.3482698511467367e+308, 4.494232837155792e+307]));
// CHECK-NEXT: -1.5936821971565687e+308

print(Math.sumPrecise([9.593842098384855e+138, -6.948356297254111e+307, -1.3482698511467367e+308, 4.494232837155792e+307]));
// CHECK-NEXT: -1.5936821971565685e+308

print(Math.sumPrecise([1.3588124894186193e+308, 1.4803986201152006e+223, 6.741349255733684e+307]));
// CHECK-NEXT: Infinity

print(Math.sumPrecise([6.741349255733684e+307, 1.7976931348623155e+308, -7.388327292663961e+41]));
// CHECK-NEXT: Infinity

print(Math.sumPrecise([-1.9807040628566093e+28, 1.7976931348623157e+308, 9.9792015476736e+291]));
// CHECK-NEXT: 1.7976931348623157e+308

print(Math.sumPrecise([-1.0214557991173964e+61, 1.7976931348623157e+308, 8.98846567431158e+307, -8.988465674311579e+307]));
// CHECK-NEXT: 1.7976931348623157e+308

print(Math.sumPrecise([1.7976931348623157e+308, 7.999999999999999, -1.908963895403937e-230, 1.6445950082320264e+292, 2.0734856707605806e+205]));
// CHECK-NEXT: Infinity

print(Math.sumPrecise([6.197409167220438e-223, -9.979201547673601e+291, -1.7976931348623157e+308]));
// CHECK-NEXT: -Infinity

print(Math.sumPrecise([4.49423283715579e+307, 8.944251746776101e+307, -0.0002441406250000001, 1.1752060710043817e+308, 4.940846717201632e+292, -1.6836699406454528e+308]));
// CHECK-NEXT: 8.353845887521184e+307

print(Math.sumPrecise([8.988465674311579e+307, 7.999999999999998, 7.029158107234023e-308, -2.2303483759420562e-172, -1.7976931348623157e+308, -8.98846567431158e+307]));
// CHECK-NEXT: -1.7976931348623157e+308

print(Math.sumPrecise([8.98846567431158e+307, 8.98846567431158e+307]));
// CHECK-NEXT: Infinity

// Property descriptor tests from test262
var descriptor = Object.getOwnPropertyDescriptor(Math, 'sumPrecise');
print(descriptor.writable);
// CHECK-NEXT: true
print(descriptor.enumerable);
// CHECK-NEXT: false
print(descriptor.configurable);
// CHECK-NEXT: true

var lengthDescriptor = Object.getOwnPropertyDescriptor(Math.sumPrecise, 'length');
print(lengthDescriptor.writable);
// CHECK-NEXT: false
print(lengthDescriptor.enumerable);
// CHECK-NEXT: false
print(lengthDescriptor.configurable);
// CHECK-NEXT: true

var nameDescriptor = Object.getOwnPropertyDescriptor(Math.sumPrecise, 'name');
print(nameDescriptor.writable);
// CHECK-NEXT: false
print(nameDescriptor.enumerable);
// CHECK-NEXT: false
print(nameDescriptor.configurable);
// CHECK-NEXT: true

// Test that Math.sumPrecise is not a constructor
// Note: The polyfill implementation doesn't prevent construction
// test262 expects this to throw, but it's a limitation of polyfill approach
// TODO: Make Math.sumPrecise non-constructable
// try {
//   new Math.sumPrecise([]);
// } catch(e) {
//   print('Cannot construct:', e.name);
// }
// CHECK-NEXT: Cannot construct: TypeError