/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

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

// Test type checking
try {
  Math.sumPrecise(123);
} catch(e) {
  print('Caught:', e.name);
}
// CHECK-NEXT: Caught: TypeError

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

// Test property descriptor checks
print(typeof Math.sumPrecise);
// CHECK-NEXT: function

print(Math.sumPrecise.length);
// CHECK-NEXT: 1

print(Math.sumPrecise.name);
// CHECK-NEXT: sumPrecise